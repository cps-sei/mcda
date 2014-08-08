#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <fstream>

#define NODE_FINISHED 5
#define NODE_COLLIDED 6
#define NODE_FINISHED_AND_COLLIDED 7

/*********************************************************************/
//-- function declarations
/*********************************************************************/
void sigalrm_handler (int signum);
void process_args(int argc,char **argv);
void set_env_vars();
void run (double & collided_nodes, double & finished_nodes, double & rounds_to_dest, double & distance_to_dest);
void read_from_file (int read_fd, int &retst, int &xi, int &yi, int &n);
void create_log_files ();
void close_log_files ();

/*********************************************************************/
//-- constants
/*********************************************************************/
const unsigned int CHILD_WAIT_TIME = 60;
const int X_SIZE = 20;
const int Y_SIZE = 20;
const int MAX_BARRIER_TIME = 3;

/*********************************************************************/
//-- global variables
/*********************************************************************/
int num_processes;
std::string exec_name,domain,out_dir;
std::vector<pid_t> child_pids;
std::vector<std::ofstream *> log_files;

//-- environment variables
std::string mcda_root,madara_root,ace_root;

/*********************************************************************/
//-- this is where everything starts
/*********************************************************************/
int main (int argc, char ** argv)
{
  signal (SIGALRM, sigalrm_handler);
  srand(getpid());
  process_args(argc,argv);
  child_pids.resize(num_processes, 0);
  set_env_vars();
  create_log_files ();

  double collided_nodes = 0;
  double finished_nodes = 0;
  double rounds_to_dest = 0;
  double distance_to_dest = 0;

  run (collided_nodes, finished_nodes, rounds_to_dest, distance_to_dest);
  close_log_files ();

  double collision_rate = collided_nodes / num_processes;
  double success_rate = finished_nodes / num_processes;

  std::ofstream stats;
  std::string stats_filename = out_dir + "/stats";
  stats.open (stats_filename.c_str(), std::ofstream::app);
  stats << collision_rate << '\t' << success_rate << '\t';

  if (distance_to_dest > 0)
  {
    double norm_completion_time = rounds_to_dest / distance_to_dest;
    stats << norm_completion_time << '\n';
  }
  else
  {
    stats << "N/A" << '\n';
  }

  stats.close();

  return 0;
}

/*********************************************************************/
//-- process arguments
/*********************************************************************/
void process_args(int argc,char **argv)
{
  if(argc != 5) {
    std::cerr << "Usage: " << argv[0] << " <executable> <num-nodes> <domain> <out-dir>\n";
    exit(1);
  }

  exec_name = argv[1];
  num_processes = atoi (argv[2]);
  domain = argv[3];
  out_dir = argv[4];
}

/*********************************************************************/
//-- signal handler
/*********************************************************************/
void sigalrm_handler (int signum)
{
  BOOST_FOREACH (pid_t pid, child_pids) kill (pid, SIGKILL);
  sleep(1);
}

/*********************************************************************/
// set environment variables
/*********************************************************************/
void set_env_vars()
{
  char *envVar = NULL;
  envVar = getenv("MCDA_ROOT");
  if(!envVar) assert(0 && "ERROR: environment variable MCDA_ROOT not set!");
  mcda_root = std::string(envVar);
  envVar = getenv("MADARA_ROOT");
  if(!envVar) assert(0 && "ERROR: environment variable MADARA_ROOT not set!");
  madara_root = std::string(envVar);
  envVar = getenv("ACE_ROOT");
  if(!envVar) assert(0 && "ERROR: environment variable ACE_ROOT not set!");
  ace_root = std::string(envVar);
}

/*********************************************************************/
//do one run
/*********************************************************************/
void run (double & collided_nodes, double & finished_nodes, double & rounds_to_dest, double & distance_to_dest)
{
  std::vector<int> xs (num_processes);
  std::vector<int> ys (num_processes);

  // Fork #N processes
  for (int i = 0; i < num_processes; i++)
  {
    // Generate random starting and ending points
    int x = rand () % X_SIZE;
    int y = rand () % Y_SIZE;
    int xf = rand () % X_SIZE;
    int yf = rand () % Y_SIZE;

    // Ensure different destinations for different nodes
    while (yf * num_processes / Y_SIZE != i)
    {
      xf = rand () % X_SIZE;
      yf = rand () % Y_SIZE;
    }

    // Record x and y for later use
    xs[i] = x;
    ys[i] = y;

    // Fork process to run the executable
    pid_t pid = fork ();

    if (pid < 0)
    {
      perror ("fork failed");
      exit (EXIT_FAILURE);
    }

    if (pid == 0)
    {
      std::ofstream * log_file = log_files[i];
      *log_file << "x : " << x << ", "
                << "y : " << y << ", "
                << "xf : " << xf << ", "
                << "yf : " << yf << '\n';
      log_file->flush ();

      execl (exec_name.c_str(), exec_name.c_str(),
             "--id", boost::lexical_cast<std::string> (i).c_str (),
             "--domain", domain.c_str (),
             "--var_x",boost::lexical_cast<std::string> (x).c_str (),
             "--var_y", boost::lexical_cast<std::string> (y).c_str (),
             "--var_xf", boost::lexical_cast<std::string> (xf).c_str (),
             "--var_yf", boost::lexical_cast<std::string> (yf).c_str (),
             "-mb", boost::lexical_cast<std::string> (MAX_BARRIER_TIME).c_str (),
             NULL);

      // execl returns only if error occured
      perror ("execl failed");
      _exit (EXIT_FAILURE);
    }
    else
    {
      // Record child pid for later waitpid
      child_pids[i] = pid;
    }
  }

  // Set timeout for child processes
  alarm (CHILD_WAIT_TIME);

  // Wait for all #N processes to finish
  for (int i = 0; i < num_processes; i++)
  {
    std::ofstream * log_file = log_files[i];

    pid_t pid = child_pids[i];
    int status;
    pid_t wait_ret = waitpid (pid, &status, 0);

    if (wait_ret == -1)
    {
      perror ("waitpid error");
      exit (EXIT_FAILURE);
    }

    // Read output from child process and update stats
    int child_status = -1;
    int xi = 0;
    int yi = 0;
    int n = 0;
    read_from_file (pid, child_status, xi, yi, n);
    int d = abs (xs[i] - xi) + abs (ys[i] - yi);
    
    if (child_status == NODE_FINISHED)
    {
      // Node reached destination
      finished_nodes++;
      rounds_to_dest += n;
      distance_to_dest += d;
      *log_file << "Finished\n";
    }
    else if (child_status == NODE_COLLIDED)
    {
      // Node collided with at least 1 other node
      collided_nodes++;
      *log_file << "Collided!\n";
    }
    else if (child_status == NODE_FINISHED_AND_COLLIDED)
    {
      // Node already reached destination, but another node collided with it
      finished_nodes++;
      rounds_to_dest += n;
      distance_to_dest += d;
      collided_nodes++;
      *log_file << "Finished and Collided!\n";
    }
    else
    {
      // Node timed out
      *log_file << "Timed out\n";
    }

    // Note: only use distance and rounds to compute normalized completion time
    // in cases where nodes finished; otherwise, just log them
    *log_file << "Distance: " << d << '\n';
    *log_file << "Rounds: " << n << '\n';

    log_file->flush ();
  }

  // Reset alarm
  alarm (0);
}

/*********************************************************************/
//read stuff written by each child process (i.e., node) via file
/*********************************************************************/
void read_from_file (int read_fd, int &retst, int &xi, int &yi, int &n)
{
  char buf[64];
  snprintf(buf,64,"/tmp/coll-avoid-%d",read_fd);
  FILE * stream = fopen (buf, "r");
  if(stream) {
    fscanf (stream, "%d %d %d %d", &retst, &xi, &yi, &n);
    fclose (stream);
  }
}

/*********************************************************************/
//create log files for all nodes
/*********************************************************************/
void create_log_files ()
{
  for (int i = 0; i < num_processes; i++)
  {
    std::ofstream * log_file = new std::ofstream ();
    std::stringstream ss;
    ss << out_dir << "/node" << i << ".log";
    log_file->open (ss.str().c_str());
    *log_file << "Node " << i << '\n';
    log_file->flush ();
    log_files.push_back (log_file);
  }
}

/*********************************************************************/
//close output files
/*********************************************************************/
void close_log_files ()
{
  BOOST_FOREACH (std::ofstream * out_file, log_files)
  {
    out_file->close ();
    delete out_file;
  }
}

/*********************************************************************/
//all done
/*********************************************************************/
