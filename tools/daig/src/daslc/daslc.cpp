/**
 * Copyright (c) 2013 Carnegie Mellon University. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following acknowledgments and disclaimers.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. The names “Carnegie Mellon University,” "SEI” and/or “Software
 *    Engineering Institute" shall not be used to endorse or promote products
 *    derived from this software without prior written permission. For written
 *    permission, please contact permission@sei.cmu.edu.
 * 
 * 4. Products derived from this software may not be called "SEI" nor may "SEI"
 *    appear in their names without prior written permission of
 *    permission@sei.cmu.edu.
 * 
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 * 
 *      This material is based upon work funded and supported by the Department
 *      of Defense under Contract No. FA8721-05-C-0003 with Carnegie Mellon
 *      University for the operation of the Software Engineering Institute, a
 *      federally funded research and development center. Any opinions,
 *      findings and conclusions or recommendations expressed in this material
 *      are those of the author(s) and do not necessarily reflect the views of
 *      the United States Department of Defense.
 * 
 *      NO WARRANTY. THIS CARNEGIE MELLON UNIVERSITY AND SOFTWARE ENGINEERING
 *      INSTITUTE MATERIAL IS FURNISHED ON AN “AS-IS” BASIS. CARNEGIE MELLON
 *      UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER EXPRESSED OR
 *      IMPLIED, AS TO ANY MATTER INCLUDING, BUT NOT LIMITED TO, WARRANTY OF
 *      FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY, OR RESULTS
 *      OBTAINED FROM USE OF THE MATERIAL. CARNEGIE MELLON UNIVERSITY DOES
 *      NOT MAKE ANY WARRANTY OF ANY KIND WITH RESPECT TO FREEDOM FROM PATENT,
 *      TRADEMARK, OR COPYRIGHT INFRINGEMENT.
 * 
 *      This material has been approved for public release and unlimited
 *      distribution.
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
std::string fileName, outFileName;
std::string madaraTarget("GNU_CPP");
bool debug = false, print=false, seqSem = false;
bool seqDbl = false, seqNoArray = false, initGlobals = false;
size_t seqNodeNum = 0, madaraNodeNum = 0;
int roundNum = -1;

//constant definitions supplied via command line
std::map<std::string,std::string> constDef;

/*********************************************************************/
//function declarations
/*********************************************************************/
void usage(char *cmd);
void parseOptions(int argc, char **argv);

/*********************************************************************/
//the main function
/*********************************************************************/
int main(int argc, char **argv)
{
  parseOptions(argc,argv);

  //create the program
  daig::DaigBuilder builder(fileName,constDef,debug);
  builder.run();

  //print the program
  if(print)
  {
    if(outFileName.empty())
    {
      builder.printProgram(std::cout);
    }
    else
    {
      std::ofstream os(outFileName.c_str());
      builder.printProgram(os);
      os.close();
    }
  }
  
  // right now, we're just using a realize flag to indicate madara generation
  if (madaraNodeNum)
  {
    //fill in the processes with madaraNodeNum nodes
    daig::Program &program = builder.program;
    const std::string &nodeName = program.nodes.begin()->first;
    program.processes.clear();
    for(size_t i = 0;i < madaraNodeNum;++i)
      program.processes.push_back(daig::Process(nodeName,i));

    // create a madara builder instance of the daig builder parse
    daig::madara::Sync_Builder madara_builder (builder,madaraTarget);
    madara_builder.build ();

    //print the generated code
    if(outFileName.empty())
      madara_builder.print (std::cout);
    else
      {
        std::ofstream os(outFileName.c_str());
        madara_builder.print (os);
        os.close();
      }
  }

  //sequentialize and print result
  if(seqNodeNum)
  {
    std::string moc = builder.program.moc.to_string_type();

    //only support synchronous programs
    if(moc != "MOC_SYNC") {
      std::cerr << "ERROR: cannot sequentialize " << moc << " programs!!\n";
      exit(1);
    }

    //fill in the processes with seqNodeNum nodes
    daig::Program &program = builder.program;
    const std::string &nodeName = program.nodes.begin()->first;
    program.processes.clear();
    for(size_t i = 0;i < seqNodeNum;++i)
      program.processes.push_back(daig::Process(nodeName,i));

    //the C program produced by sequentialization
    daig::CProgram cprog;
    
    //if doing naive sequentialization to the semantics
    if(seqSem) {
      daig::SyncSem syncSem(builder,roundNum);
      syncSem.run();
      cprog = syncSem.cprog;
    }
    //if doing optimized sequentialization with double buffering
    else if(seqDbl) {
      daig::SyncSeqDbl syncSeqDbl(builder,roundNum);
      syncSeqDbl.run();
      cprog = syncSeqDbl.cprog;
    }
    //if doing optimized sequentialization
    else {
      daig::SyncSeq syncSeq(builder,roundNum);
      syncSeq.run();
      cprog = syncSeq.cprog;
    }      
    
    //eliminate arrays
    if(seqNoArray) {
      daig::ArrayElim ae(cprog,initGlobals);
      ae.run();
      cprog = ae.outProg;
    }
    
    if(outFileName.empty())
      {
        cprog.print(std::cout,0);
      }
    else
      {
        std::ofstream os(outFileName.c_str());
        cprog.print(os,0);
        os.close();
      }
  }

  //all done
  return 0;
}

/*********************************************************************/
//parse command line options and populate global variables
/*********************************************************************/
void parseOptions(int argc, char **argv)
{
  for(int i = 1; i < argc; ++i)
  {
    if(!strcmp(argv[i],"--debug"))
    {
      debug = true;
    }
    else if(strstr(argv[i],"--seq=") == argv[i])
    {
      seqNodeNum = atoi(argv[i] + 6);
    }
    else if(!strcmp(argv[i],"--seq-sem")) seqSem = true;
    else if(!strcmp(argv[i],"--seq-dbl")) seqDbl = true;
    else if(!strcmp(argv[i],"--print"))
    {
      print = true;
    }
    else if(strstr(argv[i],"--D") == argv[i])
    {
      //extract constant and value from constant=value
      std::string arg(argv[i]);
      size_t eqpos = arg.find('=',3);
      assert(eqpos != std::string::npos 
             && "ERROR : illegal option, must be --Dconstant=value");
      int constLen = eqpos - 3;
      std::string constStr = arg.substr(3,eqpos - 3);
      std::string constVal(argv[i] + eqpos + 1);
      //std::cout << constStr << "=" << constVal << '\n';
      constDef[constStr] = constVal;
    }
    else if(strstr(argv[i],"--out=") == argv[i])
    {
      outFileName = std::string(argv[i] + 6);
    }
    else if(!strcmp(argv[i],"--seq-no-array"))
    {
      seqNoArray = true;
    }
    else if(!strcmp(argv[i],"--init-globals"))
      initGlobals = true;
    else if(strstr(argv[i],"--rounds=") == argv[i])
    {
      roundNum = atoi(argv[i] + 9);
    }
    else if(strstr(argv[i],"--madara=") == argv[i])
    {
      madaraNodeNum = atoi(argv[i] + 9);
    }
    else if(strstr(argv[i],"--target=") == argv[i])
    {
      madaraTarget = std::string(argv[i] + 9);
    }
    else if(strstr(argv[i],"--") == argv[i]) {
      std::cerr << "ERROR: illegal option " << argv[i] << '\n';
      usage(argv[0]);
    }
    else
    {
      if(!fileName.empty()) {
        std::cerr << "ERROR: filename " << fileName << " already specified\n";
        usage(argv[0]);
      }

      fileName = std::string(argv[i]);
    }
  }

  if(fileName.empty()) {
    std::cerr << "ERROR: no input-filename ...\n";
    usage(argv[0]);
  }
}

/*********************************************************************/
//print usage and exit
/*********************************************************************/
void usage(char *cmd)
{
  std::cerr << "Usage : " << cmd << " <options> filename\n";
  std::cerr << "Options :\n\t--debug\n\t--print\n\t"
            << "--Dconstant=value\n\t--seq=node-num\n\t"
            << "--seq-sem\n\t--seq-dbl\n\t--out=output-filename\n\t"
            << "--seq-no-array\n\t--init-globals\n\t--rounds=round-num\n\t"
            << "--madara=node-num\n\t"
            << "--target=target-name [default=GNU_CPP]\n";
  exit(1);
}

/*********************************************************************/
//end of daslc.cpp
/*********************************************************************/
