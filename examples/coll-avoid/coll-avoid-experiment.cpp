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

#define EXIT_FINISHED 0
#define EXIT_COLLISION 1
#define EXIT_TIMEOUT 2

/*********************************************************************/
//-- function declarations
/*********************************************************************/
void sigalrm_handler (int signum);
void process_args(int argc,char **argv);
void set_env_vars();
void run (double &distance, int &num_rounds, double &num_collisions, int &num_timeouts);
void read_from_pipe (int read_fd, int &retst, int &xi, int &yi, int &n);
void create_out_files ();
void close_out_files ();

/*********************************************************************/
//-- constants
/*********************************************************************/
const unsigned int CHILD_WAIT_TIME = 60;
const int X_SIZE = 20;
const int Y_SIZE = 20;
const int MAX_WAIT_TIME = 3;

/*********************************************************************/
//-- global variables
/*********************************************************************/
int num_processes;
std::string domain;
std::string out_dir;
std::vector<pid_t> child_pids;
std::vector<std::ofstream *> out_files;

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
  create_out_files ();

  // of all nodes combined
  double distance = 0;
  int num_rounds = 0;
  // incr by <= 1 each run
  double num_collisions = 0;
  int num_timeouts = 0;

  run (distance, num_rounds, num_collisions, num_timeouts);

  printf ("Number of nodes collided: %f\n", num_collisions);
  double avg_speed = num_rounds ? distance / num_rounds : 0.0;
  printf ("Average speed: %f unit/round\n", avg_speed);
  printf ("Number of nodes timedout: %d\n", num_timeouts);

  close_out_files ();

  std::ofstream main_out;
  std::string main_out_filename = out_dir + "/stats";
  main_out.open (main_out_filename.c_str(), std::ofstream::app);
  main_out << num_collisions << '\t' << avg_speed << '\t' << num_timeouts << '\n';
  main_out.close();

  return 0;
}

/*********************************************************************/
//-- process arguments
/*********************************************************************/
void process_args(int argc,char **argv)
{
  num_processes = atoi (argv[1]);
  domain = argv[2];
  out_dir = argv[3];
}

/*********************************************************************/
//-- signal handler
/*********************************************************************/
void sigalrm_handler (int signum)
{
  //BOOST_FOREACH (pid_t pid, child_pids) kill (pid, SIGTERM);
  //sleep(1);
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
void run (double &distance, int &num_rounds, double &num_collisions, int &num_timeouts)
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
    while(abs(x-xf) < 5 || abs(y-yf) < 5) {
      xf = rand () % X_SIZE;
      yf = rand () % Y_SIZE;
    }


    xs[i] = x;
    ys[i] = y;

    // Fork process to run coll-avoid
    pid_t pid = fork ();

    if (pid < 0)
    {
      perror ("fork failed");
      exit (EXIT_FAILURE);
    }

    if (pid == 0)
    {
      std::ofstream * out_file = out_files[i];
      *out_file << "x : " << x << ", "
                << "y : " << y << ", "
                << "xf : " << xf << ", "
                << "yf : " << yf << '\n';
      out_file->flush ();

      execl ("./coll-avoid", "coll-avoid",
             "--id", boost::lexical_cast<std::string> (i).c_str (),
             "--domain", domain.c_str (),
             "--var_x",boost::lexical_cast<std::string> (x).c_str (),
             "--var_y", boost::lexical_cast<std::string> (y).c_str (),
             "--var_xf", boost::lexical_cast<std::string> (xf).c_str (),
             "--var_yf", boost::lexical_cast<std::string> (yf).c_str (),
             "-mb", boost::lexical_cast<std::string> (MAX_WAIT_TIME).c_str (),
             NULL);

      // execl returns only if error occured
      perror ("execl coll-avoid failed");
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
    std::ofstream * out_file = out_files[i];

    pid_t pid = child_pids[i];
    int status;
    pid_t ret = waitpid (pid, &status, 0);

    if (ret == -1)
    {
      perror ("waitpid error");
      exit (EXIT_FAILURE);
    }

    // Read output from child process and update stats but only if the
    // node did not timeout
    int retst = -1,xi=0, yi=0, n=0;
    read_from_pipe (pid, retst, xi, yi, n);
    
    if (retst == EXIT_FINISHED)
    {
      // Node reached destination
      *out_file << "Finished\n";
    }
    else if (retst == EXIT_COLLISION)
    {
      // Collision
      num_collisions++;
      *out_file << "Collision!\n";
    }
    else
    {
      // Node timed out
      num_timeouts++;
      *out_file << "Timed out\n";
    }

    if(retst == EXIT_FINISHED || retst == EXIT_COLLISION) {
      int d = abs (xs[i] - xi) + abs (ys[i] - yi);
      distance += d;
      num_rounds += n;      
      *out_file << "Distance completed: " << d << '\n';
      *out_file << "Rounds taken: " << n << '\n';
    }

    out_file->flush ();
  }

  // Reset alarm
  alarm (0);
}

void read_from_pipe (int read_fd, int &retst, int &xi, int &yi, int &n)
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
//read stuff written by each child process (i.e., node) via pipe
/*********************************************************************/
void create_out_files ()
{
  for (int i = 0; i < num_processes; i++)
  {
    std::ofstream * out_file = new std::ofstream ();
    std::stringstream ss;
    ss << out_dir << "/node" << i << ".out";
    out_file->open (ss.str().c_str());
    *out_file << "Node " << i << '\n';
    out_file->flush ();
    out_files.push_back (out_file);
  }
}

/*********************************************************************/
//close output files
/*********************************************************************/
void close_out_files ()
{
  BOOST_FOREACH (std::ofstream * out_file, out_files)
  {
    out_file->close ();
    delete out_file;
  }
}

/*********************************************************************/
//all done
/*********************************************************************/
