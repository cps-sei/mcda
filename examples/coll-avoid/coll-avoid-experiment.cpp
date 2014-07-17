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

void sigalrm_handler (int signum);
void compile ();
void run (const int r, double &distance, int &num_rounds, double &num_collisions, int &num_timeouts);
void read_from_pipe (int read_fd, int &xi, int &yi, int &n);
void create_out_files ();
void close_out_files ();

const unsigned int CHILD_WAIT_TIME = 60;
const int X_SIZE = 10;
const int Y_SIZE = 10;
const int MAX_WAIT_TIME = 3;

int num_processes;
int num_runs;
std::vector<pid_t> child_pids;
std::vector<std::ofstream *> out_files;

int main (int argc, char ** argv)
{
  srand (time(NULL));

  // Register alarm signal handler
  signal (SIGALRM, (void (*)(int)) sigalrm_handler);

  num_processes = atoi (argv[1]);
  num_runs = atoi (argv[2]);

  child_pids.resize(num_processes, 0);

  compile ();
  create_out_files ();

  // of all nodes combined
  double distance = 0;
  int num_rounds = 0;
  // incr by <= 1 each run
  double num_collisions = 0;
  int num_timeouts = 0;

  for (int i = 0; i < num_runs; i++)
  {
    printf ("Run %d...", i);
    fflush (stdout);
    run (i, distance, num_rounds, num_collisions, num_timeouts);
    printf ("completed\n");
  }

  double collision_rate = num_collisions / num_runs;
  double avg_speed = distance / num_rounds;

  printf ("Collision rate: %f\n", collision_rate);
  printf ("Average speed: %f unit/round\n", avg_speed);
  printf ("Number of timeouts: %d\n", num_timeouts);

  close_out_files ();

  return 0;
}

void sigalrm_handler (int signum)
{
  BOOST_FOREACH (pid_t pid, child_pids)
  {
    kill (pid, SIGTERM);
  }
}

void compile ()
{
  // 1st fork: to compile dasl program
  pid_t pid = fork ();

  if (pid < 0)
  {
    perror ("fork failed");
    exit (EXIT_FAILURE);
  }

  if (pid == 0)
  {
    execl ("/home/rsukkerd/Projects/mcda/src/daslc/daslc", "daslc",
           "--nodes", boost::lexical_cast<std::string> (num_processes).c_str (),
           "--madara",
           "--out", "coll-avoid.cpp", "coll-avoid.dasl", NULL);

    // execl returns only if error occured
    perror ("execl daslc failed");
    _exit (EXIT_FAILURE);
  }

  wait (NULL);

  // 2nd fork: to compile c++ program
  pid = fork ();

  if (pid < 0)
  {
    perror ("fork failed");
    exit (EXIT_FAILURE);
  }

  if (pid == 0)
  {
    execl ("/usr/bin/g++", "g++",
           "-I/home/rsukkerd/Projects/mcda/madara/ace/ACE_wrappers",
           "-I/home/rsukkerd/Projects/mcda/madara/madara/include",
           "-o", "coll-avoid", "coll-avoid.cpp",
           "/home/rsukkerd/Projects/mcda/madara/madara/libMADARA.so",
           "/home/rsukkerd/Projects/mcda/madara/ace/ACE_wrappers/lib/libACE.so", NULL);

    // execl returns only if error occured
    perror ("execl g++ failed");
    _exit (EXIT_FAILURE);
  }

  wait (NULL);
}

void run (const int r, double &distance, int &num_rounds, double &num_collisions, int &num_timeouts)
{
  std::vector<int> xs (num_processes);
  std::vector<int> ys (num_processes);
  std::vector<int> read_fds (num_processes);

  // Fork #N processes
  for (int i = 0; i < num_processes; i++)
  {
    // Create pipe for parent to read result from child
    int pipefd[2];

    if (pipe (pipefd) == -1)
    {
      perror ("pipe failed");
      exit (EXIT_FAILURE);
    }

    // Generate random starting and ending points
    int x = rand () % X_SIZE;
    int y = rand () % Y_SIZE;
    int xf = rand () % X_SIZE;
    int yf = rand () % Y_SIZE;
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
      // Child process writes result to pipe
      // Close read end of pipe
      close (pipefd[0]);

      std::ofstream * out_file = out_files[i];
      *out_file << "Run " << r << '\n';
      *out_file << "x : " << x << ", ";
      *out_file << "y : " << y << ", ";
      *out_file << "xf : " << xf << ", ";
      *out_file << "yf : " << yf << '\n';
      out_file->flush ();

      execl ("./coll-avoid", "coll-avoid",
             "--id", boost::lexical_cast<std::string> (i).c_str (),
             "--var_x",boost::lexical_cast<std::string> (x).c_str (),
             "--var_y", boost::lexical_cast<std::string> (y).c_str (),
             "--var_xf", boost::lexical_cast<std::string> (xf).c_str (),
             "--var_yf", boost::lexical_cast<std::string> (yf).c_str (),
             "-mb", boost::lexical_cast<std::string> (MAX_WAIT_TIME).c_str (),
             "--write-fd", boost::lexical_cast<std::string> (pipefd[1]).c_str (), NULL);

      // execl returns only if error occured
      perror ("execl coll-avoid failed");
      _exit (EXIT_FAILURE);
    }
    else
    {
      // Parent process reads result from pipe
      // Record child pid for later waitpid
      child_pids[i] = pid;
      // Close write end of pipe
      close (pipefd[1]);
      // Record read pipefd for later read
      read_fds[i] = pipefd[0];
    }
  }

  bool collision = false;
  bool timeout = false;

  // Set timeout for child processes
  alarm (CHILD_WAIT_TIME);

  // Wait for all #N processes to finish
  for (int i = 0; i < num_processes; i++)
  {
    std::ofstream * out_file = out_files[i];

    pid_t pid = child_pids[i];
    int read_fd = read_fds[i];
    int status;

    pid_t ret = waitpid (pid, &status, 0);

    if (ret == -1)
    {
      perror ("waitpid error");
      exit (EXIT_FAILURE);
    }

    if (WEXITSTATUS (status) == EXIT_FINISHED)
    {
      // Node reached destination
      *out_file << "Finished\n";
    }
    else if (WEXITSTATUS (status) == EXIT_COLLISION)
    {
      // Collision
      collision = true;
      *out_file << "Collision!\n";
    }
    else if (WEXITSTATUS (status) == EXIT_TIMEOUT)
    {
      // Node timed out
      timeout = true;
      *out_file << "Timed out\n";
    }

    // Read output from child process
    int xi, yi, n;
    read_from_pipe (read_fd, xi, yi, n);

    int d = abs (xs[i] - xi) + abs (ys[i] - yi);
    distance += d;
    num_rounds += n;

    *out_file << "Distance completed: " << d << '\n';
    *out_file << "Rounds taken: " << n << '\n';
    out_file->flush ();
  }

  // Reset alarm
  alarm (0);

  if (collision)
  {
    num_collisions++;
  }

  if (timeout)
  {
    num_timeouts++;
  }
}

void read_from_pipe (int read_fd, int &xi, int &yi, int &n)
{
  int _xi, _yi, _n;
  FILE * stream = fdopen (read_fd, "r");
  fscanf (stream, "%d %d %d", &_xi, &_yi, &_n);
  fclose (stream);
  xi = _xi;
  yi = _yi;
  n = _n;
}

void create_out_files ()
{
  for (int i = 0; i < num_processes; i++)
  {
    std::ofstream * out_file = new std::ofstream ();
    std::stringstream ss;
    ss << "node" << i << ".out";
    out_file->open (ss.str().c_str());
    *out_file << "Node " << i << '\n';
    out_file->flush ();
    out_files.push_back (out_file);
  }
}

void close_out_files ()
{
  BOOST_FOREACH (std::ofstream * out_file, out_files)
  {
    out_file->close ();
    delete out_file;
  }
}
