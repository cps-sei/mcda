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
bool is_approaching (int x, int y);
bool is_crossing (int x, int y);

/*********************************************************************/
//-- constants
/*********************************************************************/
const int MAX_WAIT_TIME = 60;
const int X_SIZE = 6;
const int Y_SIZE = 6;
const int NUM_INTS_X = 1;
const int NUM_INTS_Y = 1;
const int N_lanes[] = {3};
const int E_lanes[] = {2};
const int W_lanes[] = {3};
const int S_lanes[] = {2};

/*********************************************************************/
//-- global variables
/*********************************************************************/
int num_processes;
std::string exec_name,domain,logdir;
std::vector<pid_t> node_pids,monitor_pids;

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
  node_pids.resize(num_processes, 0);
  monitor_pids.resize(num_processes, 0);
  set_env_vars();
  run ();
  return 0;
}

/*********************************************************************/
//-- process arguments
/*********************************************************************/
void process_args(int argc,char **argv)
{
  if(argc != 5) {
    std::cerr << "Usage: " << argv[0] << " <executable> <num-nodes> <domain> <log-dir>\n";
    exit(1);
  }

  exec_name = argv[1];
  num_processes = atoi (argv[2]);
  domain = argv[3];
  logdir = argv[4];
}

/*********************************************************************/
//-- signal handler
/*********************************************************************/
void sigalrm_handler (int signum)
{
  BOOST_FOREACH (pid_t npid, node_pids) kill (npid, SIGKILL);
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
  // Fork #N node processes
  for (int i = 0; i < num_processes; i++)
  {
    int x, y;
    do {
      // Choose lane type at random
      int lane = rand() % 4;

      if (lane == 0) {
        // Choose N lane at random
        x = N_lanes[rand() % NUM_INTS_X];
        do {
          y = rand() % Y_SIZE;
        } while (is_approaching(x, y) || is_crossing(x, y));
      } else if (lane == 1) {
        // Choose E lane at random
        y = E_lanes[rand() % NUM_INTS_Y];
        do {
          x = rand() % X_SIZE;
        } while (is_approaching(x, y) || is_crossing(x, y));
      } else if (lane == 2) {
        // Choose W lane at random
        y = W_lanes[rand() % NUM_INTS_Y];
        do {
          x = rand() % X_SIZE;
        } while (is_approaching(x, y) || is_crossing(x, y));
      } else {
        // Choose S lane at random
        x = S_lanes[rand() % NUM_INTS_X];
        do {
          y = rand() % Y_SIZE;
        } while (is_approaching(x, y) || is_crossing(x, y));
      }
      // Evenly distribute nodes along y-axis
      // (ensure no 2 nodes start at same point)
    } while (!(y >= i * Y_SIZE / num_processes && y < (i + 1) * Y_SIZE / num_processes));

    // Log starting locations to info file
    std::stringstream infofile;
    infofile << logdir << "/info";
    std::ofstream info;
    info.open (infofile.str().c_str(), std::ofstream::app);
    info << "Node " << i << " starts at ";
    info << "(" << x << ", " << y << ")" << std::endl;

    // Fork process to run the executable
    pid_t pid = fork ();

    if (pid < 0)
    {
      perror ("fork failed");
      exit (EXIT_FAILURE);
    }

    if (pid == 0)
    {
      // Node process
      std::stringstream logfile;
      logfile << logdir << "/node" << i << ".log";

      // Run the ASYNC_MOC executable
      execl (exec_name.c_str(), exec_name.c_str(),
             "--id", boost::lexical_cast<std::string> (i).c_str (),
             "--domain", domain.c_str (),
             "--var_x",boost::lexical_cast<std::string> (x).c_str (),
             "--var_y", boost::lexical_cast<std::string> (y).c_str (),
             "--app-logfile", logfile.str ().c_str (),
             NULL);

      // execl returns only if error occured
      perror ("execl failed");
      _exit (EXIT_FAILURE);
    }
    else
    {
      // Parent process
      // Record node pid for later waitpid
      node_pids[i] = pid;
    }
  }

  // Fork #N monitor processes
  for (int i = 0; i < num_processes; i++)
  {
    // Fork process to monitor node
    pid_t pid = fork ();

    if (pid < 0)
    {
      perror ("fork failed");
      exit (EXIT_FAILURE);
    }

    if (pid == 0)
    {
      // Monitor process
      // Wait for node i to terminate
      pid_t npid = node_pids[i];
      int status;
      pid_t wait_ret = waitpid (npid, &status, 0);

      if (wait_ret == -1)
      {
        perror ("waitpid error");
        exit (EXIT_FAILURE);
      }

      // Kill all other nodes and terminate
      // (as a result, other monitors will terminate)
      BOOST_FOREACH (pid_t p, node_pids) {
        if (p != npid)
          kill (p, SIGKILL);
      }

      _exit (EXIT_SUCCESS);
    }
    else
    {
      // Parent process
      // Record monitor pid for later waitpid
      monitor_pids[i] = pid;
    }
  }

  // Set timeout for monitor processes
  // (but kill node processes if timeout)
  alarm (MAX_WAIT_TIME);

  // Wait for monitor processes
  for (int i = 0; i < num_processes; i++)
  {
    pid_t mpid = monitor_pids[i];
    int status;
    pid_t wait_ret = waitpid (mpid, &status, 0);

    if (wait_ret == -1)
    {
      perror ("waitpid error");
      exit (EXIT_FAILURE);
    }

    // Other nodes should have terminated or been killed
  }

  // Reset alarm
  alarm (0);
}

bool is_approaching (int x, int y) {
  for (int i = 0; i < NUM_INTS_X; i++) {
    for (int j = 0; j < NUM_INTS_Y; j++) {
      if ((x == N_lanes[i] && y == E_lanes[j] - 1) || // A
          (y == E_lanes[j] && x == S_lanes[i] - 1) || // B
          (y == W_lanes[j] && x == N_lanes[i] + 1) || // C
          (x == S_lanes[i] && y == W_lanes[j] + 1)) { // D
        // Approaching intersection
        return true;
      }
    }
  }
  return false;
}

bool is_crossing (int x, int y) {
  for (int i = 0; i < NUM_INTS_X; i++) {
    for (int j = 0; j < NUM_INTS_Y; j++) {
      if ((x == N_lanes[i] || x == S_lanes[i]) &&
          (y == E_lanes[j] || y == W_lanes[j])) {
        // Crossing intersection
        return true;
      }
    }
  }
  return false;
}

/*********************************************************************/
//all done
/*********************************************************************/
