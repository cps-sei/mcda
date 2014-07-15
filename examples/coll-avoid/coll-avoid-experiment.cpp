#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <fstream>

void compile ();
void run (int &num_collisions, int &total_num_rounds);
int read_from_pipe (int fd);
void create_out_files ();
void close_out_files ();

const int X_SIZE = 10;
const int Y_SIZE = 10;
const int MAX_WAIT_TIME = 3;

int num_processes;
int num_runs;
std::vector<std::ofstream *> out_files;

int main (int argc, char ** argv)
{
  num_processes = atoi (argv[1]);
  num_runs = atoi (argv[2]);

  compile ();
  create_out_files ();

  int num_collisions = 0;
  int total_num_rounds = 0;

  for (int i = 0; i < num_runs; i++)
  {
    run (num_collisions, total_num_rounds);
  }

  double collision_rate = (double) num_collisions / num_runs;
  double avg_num_rounds = (double) total_num_rounds / (num_runs - num_collisions);

  printf ("Collision rate: %f\n", collision_rate);
  printf ("Average rounds/run: %f\n", avg_num_rounds);

  close_out_files ();

  return 0;
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

void run (int &num_collisions, int &total_num_rounds)
{
  std::vector<pid_t> children_pids (num_processes);
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

    // Fork process to run coll-avoid
    pid_t pid = fork ();

    // Unique random seed for each process
    srand (time(NULL) ^ getpid () << 16);

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

      // Generate random starting and ending points
      int x = rand () % X_SIZE;
      int y = rand () % Y_SIZE;
      int xf = rand () % X_SIZE;
      int yf = rand () % Y_SIZE;

      std::ofstream * out_file = out_files[i];
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
      children_pids[i] = pid;
      // Close write end of pipe
      close (pipefd[1]);
      // Record read pipefd for later read
      read_fds[i] = pipefd[0];
    }
  }

  bool collision = false;

  // Wait for all #N processes to finish
  for (int i = 0; i < num_processes; i++)
  {
    std::ofstream * out_file = out_files[i];

    pid_t pid = children_pids[i];
    int read_fd = read_fds[i];
    int status;

    pid_t ret = waitpid (pid, &status, 0);

    if (ret == -1)
    {
      perror ("waitpid error");
      exit (EXIT_FAILURE);
    }

    if (!WIFEXITED (status))
    {
      perror ("child did not exit normally");
      exit (EXIT_FAILURE);
    }

    // Child process exited normally
    if (WEXITSTATUS (status) != 0)
    {
      // There was a collision
      collision = true;
      *out_file << "Collision!\n";
    }
    else
    {
      // There was no collision
      // Read number of rounds from child process
      int num_rounds = read_from_pipe (read_fd);
      total_num_rounds += num_rounds;
      *out_file << "#rounds : " << num_rounds << '\n';
    }

    out_file->flush ();
  }

  if (collision)
  {
    num_collisions++;
  }
}

int read_from_pipe (int read_fd)
{
  int n;
  read (read_fd, &n, sizeof (n));
  return n;
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
