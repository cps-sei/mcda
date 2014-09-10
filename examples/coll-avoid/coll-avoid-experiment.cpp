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


/*********************************************************************/
//-- function declarations
/*********************************************************************/
void sigalrm_handler (int signum);
void process_args(int argc,char **argv);
void set_env_vars();
void run ();

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
bool is_sync_moc;
std::string exec_name,domain,logdir;
std::vector<pid_t> child_pids;

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
  run ();
  return 0;
}

/*********************************************************************/
//-- process arguments
/*********************************************************************/
void process_args(int argc,char **argv)
{
  if(argc != 6) {
    std::cerr << "Usage: " << argv[0] << " <executable> <SYNC=0/ASYNC=1> <num-nodes> <domain> <log-dir>\n";
    exit(1);
  }

  exec_name = argv[1];
  is_sync_moc = (atoi (argv[2]) == 0);
  num_processes = atoi (argv[3]);
  domain = argv[4];
  logdir = argv[5];
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
void run ()
{
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

    // Fork process to run the executable
    pid_t pid = fork ();

    if (pid < 0)
    {
      perror ("fork failed");
      exit (EXIT_FAILURE);
    }

    if (pid == 0)
    {
      std::stringstream logfile;
      logfile << logdir << "/node" << i << ".log";

      if (is_sync_moc)
      {
        // Run the SYNC_MOC executable
        execl (exec_name.c_str(), exec_name.c_str(),
               "--id", boost::lexical_cast<std::string> (i).c_str (),
               "--domain", domain.c_str (),
               "--var_x",boost::lexical_cast<std::string> (x).c_str (),
               "--var_y", boost::lexical_cast<std::string> (y).c_str (),
               "--var_xf", boost::lexical_cast<std::string> (xf).c_str (),
               "--var_yf", boost::lexical_cast<std::string> (yf).c_str (),
               "--max-barrier-time", boost::lexical_cast<std::string> (MAX_BARRIER_TIME).c_str (),
               "--app-logfile", logfile.str ().c_str (),
               NULL);
      }
      else
      {
        // Run the ASYNC_MOC executable
        execl (exec_name.c_str(), exec_name.c_str(),
               "--id", boost::lexical_cast<std::string> (i).c_str (),
               "--domain", domain.c_str (),
               "--var_x",boost::lexical_cast<std::string> (x).c_str (),
               "--var_y", boost::lexical_cast<std::string> (y).c_str (),
               "--var_xf", boost::lexical_cast<std::string> (xf).c_str (),
               "--var_yf", boost::lexical_cast<std::string> (yf).c_str (),
               "--app-logfile", logfile.str ().c_str (),
               NULL);
      }


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
    pid_t pid = child_pids[i];
    int status;
    pid_t wait_ret = waitpid (pid, &status, 0);

    if (wait_ret == -1)
    {
      perror ("waitpid error");
      exit (EXIT_FAILURE);
    }
  }

  // Reset alarm
  alarm (0);
}

/*********************************************************************/
//all done
/*********************************************************************/
