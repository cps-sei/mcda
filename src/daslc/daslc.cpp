/**
 * Copyright (c) 2014 Carnegie Mellon University. All Rights Reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:

 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following acknowledgments
 * and disclaimers.

 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.

 * 3. The names "Carnegie Mellon University," "SEI" and/or "Software
 * Engineering Institute" shall not be used to endorse or promote
 * products derived from this software without prior written
 * permission. For written permission, please contact
 * permission@sei.cmu.edu.

 * 4. Products derived from this software may not be called "SEI" nor
 * may "SEI" appear in their names without prior written permission of
 * permission@sei.cmu.edu.

 * 5. Redistributions of any form whatsoever must retain the following
 * acknowledgment:

 * This material is based upon work funded and supported by the
 * Department of Defense under Contract No. FA8721-05-C-0003 with
 * Carnegie Mellon University for the operation of the Software
 * Engineering Institute, a federally funded research and development
 * center.

 * Any opinions, findings and conclusions or recommendations expressed
 * in this material are those of the author(s) and do not necessarily
 * reflect the views of the United States Department of Defense.

 * NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE
 * ENGINEERING INSTITUTE MATERIAL IS FURNISHED ON AN "AS-IS"
 * BASIS. CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, AS TO ANY MATTER INCLUDING, BUT NOT
 * LIMITED TO, WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY,
 * EXCLUSIVITY, OR RESULTS OBTAINED FROM USE OF THE MATERIAL. CARNEGIE
 * MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH
 * RESPECT TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT
 * INFRINGEMENT.

 * This material has been approved for public release and unlimited
 * distribution.

 * DM-0001023
**/


#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include "DaigBuilder.hpp"
#include "daig/madara/Sync_Builder.hpp"
#include "SyncSeq.hpp"
#include "SyncSem.hpp"
#include "SyncSeqDbl.hpp"
#include "ArrayElim.hpp"

/*********************************************************************/
//options
/*********************************************************************/
std::string file_name, out_file;
std::string madara_target ("GNU_CPP");
bool do_print = false, do_madara = false, do_seq = false;
bool do_vrep = false;
bool debug = false, seq_sem = false, seq_dbl = false;
bool seq_no_array = false, init_globals = false;
size_t nodes = 0;
int round_num = -1;

//constant definitions supplied via command line
std::map<std::string, std::string> const_def;

/*********************************************************************/
//function declarations
/*********************************************************************/
void usage (char *cmd);
void parse_options (int argc, char **argv);

/*********************************************************************/
//the main function
/*********************************************************************/
int main (int argc, char **argv)
{
  parse_options (argc, argv);

  //create the program
  daig::DaigBuilder builder (file_name, const_def, debug);
  builder.run ();

  //print the program
  if (do_print)
  {
    if (out_file.empty ())
    {
      builder.printProgram (std::cout);
    }
    else
    {
      std::ofstream os (out_file.c_str ());
      builder.printProgram (os);
      os.close ();
    }
  }
  
  // right now, we're just using a realize flag to indicate madara
  // generation
  if (do_madara)
  {
    //fill in the processes with nodes nodes
    daig::Program & program = builder.program;
    const std::string & nodeName = program.nodes.begin ()->first;
    program.processes.clear ();
    for (size_t i = 0;i < nodes;++i)
      program.processes.push_back (daig::Process (nodeName, i));

    // create a madara builder instance of the daig builder parse
    std::string moc = builder.program.moc.to_string_type ();
    daig::madara::Madara_Builder *madara_builder = NULL;
    
    if(moc == "MOC_SYNC")
      madara_builder = new daig::madara::Sync_Builder (builder, madara_target, do_vrep);
    else {
      std::cerr << "ERROR: cannot generate code for " << moc << " programs!!\n";
      exit (1);
    }

    //build the generated code
    madara_builder->build ();

    //print the generated code
    if (out_file.empty ())
      madara_builder->print (std::cout);
    else
    {
      std::ofstream os (out_file.c_str ());
      madara_builder->print (os);
      os.close ();
    }

    //cleanup
    delete madara_builder;
  }

  //sequentialize and print result
  if (do_seq)
  {
    std::string moc = builder.program.moc.to_string_type ();

    //only support synchronous programs
    if (moc != "MOC_SYNC") {
      std::cerr << "ERROR: cannot sequentialize " << moc << " programs!!\n";
      exit (1);
    }

    //fill in the processes with seq_node_num nodes
    daig::Program & program = builder.program;
    const std::string & nodeName = program.nodes.begin ()->first;
    program.processes.clear ();
    for (size_t i = 0;i < nodes;++i)
      program.processes.push_back (daig::Process (nodeName, i));

    //the C program produced by sequentialization
    daig::CProgram cprog;
    
    //if doing naive sequentialization to the semantics
    if (seq_sem) {
      daig::SyncSem syncSem (builder, round_num);
      syncSem.run ();
      cprog = syncSem.cprog;
    }
    //if doing optimized sequentialization with double buffering
    else if (seq_dbl) {
      daig::SyncSeqDbl syncSeqDbl (builder, round_num);
      syncSeqDbl.run ();
      cprog = syncSeqDbl.cprog;
    }
    //if doing optimized sequentialization
    else {
      daig::SyncSeq syncSeq (builder, round_num);
      syncSeq.run ();
      cprog = syncSeq.cprog;
    }      
    
    //eliminate arrays
    if (seq_no_array) {
      daig::ArrayElim ae (cprog, init_globals);
      ae.run ();
      cprog = ae.outProg;
    }
    
    if (out_file.empty ())
      {
        cprog.print (std::cout, 0);
      }
    else
      {
        std::ofstream os (out_file.c_str ());
        cprog.print (os, 0);
        os.close ();
      }
  }

  //all done
  return 0;
}

/*********************************************************************/
//parse command line options and populate global variables
/*********************************************************************/
void parse_options (int argc, char **argv)
{
  for (int i = 1; i < argc; ++i)
  {
    std::string arg1 (argv[i]);

    if (arg1 == "-d" || arg1 == "--debug")
    {
      debug = true;
    }
    else if (arg1 == "-h" || arg1 == "--help")
    {
      usage (argv[0]);
    }
    else if (arg1 == "-o" || arg1 == "--out")
    {
      if (i + 1 < argc)
      {
        out_file = argv[i + 1];
      }
      else
      {
        std::cerr << "ERROR: output file (-o|--out) must have file name\n";
        usage (argv[0]);
      }
      ++i;
    }
    else if (arg1 == "-p" || arg1 == "--print")
    {
      do_print = true;
    }
    else if (arg1 == "-n" || arg1 == "--nodes")
    {
      if (i + 1 < argc)
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> nodes;
      }
      else
      {
        std::cerr << "ERROR: nodes (-n|--nodes) must have value (e.g. -n 5)\n";
        usage (argv[0]);
      }
      ++i;
    }
    else if (arg1 == "-m" || arg1 == "--madara")
    {
      do_madara = true;
    }
    else if (arg1 == "-t" || arg1 == "--target" || arg1 == "--platform")
    {
      if (i + 1 < argc)
      {
        madara_target = argv[i + 1];
      }
      else
      {
        std::cerr << "ERROR: Target platform (-t|--target|--platform) must have a ";
        std::cerr << " target (e.g. -t WIN_CPP)\n";
        usage (argv[0]);
      }
      ++i;
    }
    else if (arg1 == "-vr" || arg1 == "--vrep")
    {
      do_vrep = true;
    }
    else if (arg1 == "-s" || arg1 == "--seq")
    {
      do_seq = true;
    }
    else if (arg1 == "-r" || arg1 == "--rounds")
    {
      if (i + 1 < argc)
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> round_num;
      }
      else
      {
        std::cerr << "ERROR: Number of rounds (-r|--rounds) must have a ";
        std::cerr << " number of rounds (e.g. -r 5)\n";
        usage (argv[0]);
      }
      ++i;
    }
    else if (arg1 == "--seq-dbl")
    {
      seq_dbl = true;
    }
    else if (arg1 == "--seq-no-array")
    {
      seq_no_array = true;
    }
    else if (arg1 == "--seq-sem")
    {
      seq_sem = true;
    }
    else if (arg1 == "-g" || arg1 == "--init-globals")
    {
      init_globals = true;
    }
    else if (arg1.substr (0,3) == "--D")
    {
      //extract constant and value from constant=value
      std::string var_name = arg1.substr (3);
      
      if (i + 1 < argc && var_name.size () > 0)
      {
        std::string value;
        std::stringstream buffer (argv[i + 1]);
        buffer >> value;
        const_def[var_name] = value;
      }
      else
      {
        std::cerr << "ERROR: Constant value (via --D) must have a ";
        std::cerr << " name and a value (e.g. --Dmy_const 5)\n";
        usage (argv[0]);
      }
      ++i;

    }
    else
    {
      if (!file_name.empty ())
      {
        std::cerr << "ERROR: filename " << file_name << " already specified\n";
        usage (argv[0]);
      }

      file_name = std::string (argv[i]);
    }
  }

  if (file_name.empty ()) {
    usage (argv[0]);
  }
}

/*********************************************************************/
//print usage and exit
/*********************************************************************/
void usage (char *cmd)
{
  std::cerr << "Usage : " << cmd << " <options optionval> filename\n";
  std::cerr << "Options :\n";
  std::cerr << "  -d|--debug               print debugging information\n";
  std::cerr << "  -h|--help                print help and usage\n";
  std::cerr << "  -o|--out file            output file, default is stdout\n";
  std::cerr << "  -p|--print               parse and print DASL file\n";
  std::cerr << "  -n|--nodes nodes         number of nodes\n";
  std::cerr << "  -m|--madara              generate C++/MADARA code to run\n";
  std::cerr << "  -t|--target|--platform p specify a target platform\n";
  std::cerr << "        Available platforms: WIN_CPP, GNU_CPP (default)\n";
  std::cerr << "  -vr|--vrep               generate code that targets VREP\n";
  std::cerr << "  -s|--seq                 generate sequentialized code to verify\n";
  std::cerr << "  -r|--rounds rounds       number of verification rounds\n";
  std::cerr << "  --seq-dbl                use double buffering during verification\n";
  std::cerr << "  --seq-no-array           do not use arrays during verification\n";
  std::cerr << "  --seq-sem                use variable copying during verification\n";
  std::cerr << "  -g|--init-globals        initialize global variables\n";
  std::cerr << "  --D<const_name> value    set a const to a value\n";
  exit (0);
}

/*********************************************************************/
//end of daslc.cpp
/*********************************************************************/
