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
#ifndef _DAIG_PROGRAM_H_
#define _DAIG_PROGRAM_H_

/**
 * @file Program.h
 * @author James Edmondson <jedmondson@gmail.com>
 *
 * This file contains a class definition for the DAIG Program container.
 **/

#include <list>
#include <map>
#include <string>
#include "Model_Of_Computation.h"
#include "Variable.h"
#include "Function.h"
#include "Node.h"
#include "Visitor.h"

namespace daig
{
  //a process is a node name and a node id
  class Process : public std::pair<std::string,int>
  {
  public:
    Process(const std::string &n,const int id)
      : std::pair<std::string,int>(n,id) {}

    const std::string &getNode() const { return first; }
    int getId() const { return second; }
  };

  /**
    * @class Program
    * @brief Encapsulates a program definition
    */
  class Program
  {
  public:
    /**
     * Prints variable information
     * @param  indent  spaces to indent printout
     **/
    void print (std::ostream &os,unsigned int indent);
    
    /**
     *
     **/

    /**
     * Model of computation for the program
     **/
    Model_Of_Computation moc;

    ///target thunks
    typedef std::map<std::string,std::string> TargetType;
    TargetType targets;

    //constant definitions
    typedef std::map<std::string,std::string> ConstDef;
    ConstDef constDef;

    /**
     * external function declarations -- these are Function objects
     * with empty bodies
     **/
    Functions externalFuncs;

    /**
     * A map of function names to function definitions
     **/
    Functions funcs;

    /**
     * The node definitions
     **/
    Nodes nodes;

    //the list of processes
    std::list<Process> processes;

    ///add a target thunk, or append to an existing one
    void addTarget(const std::string &tgt,const std::string &thk)
    { targets[tgt] += thk; }

    ///add an external function
    void addExternalFunction(const Function &f) { externalFuncs[f.name] = f; }

    ///add a function
    void addFunction(const Function &f) { funcs[f.name] = f; }

    ///return true if the argument is the name of an external function
    bool isExternalFunction(const std::string &fn) const 
    {
      return externalFuncs.count(fn) > 0;
    }

    ///check various sanity conditions on the program
    void sanityCheck();
  };

  //new namespace to avoid name collisions
  namespace program {
    /*****************************************************************/
    //a visitor that checks for various sanity conditions
    /*****************************************************************/
    struct SanityChecker : public Visitor
    {
      //the DASL program being checked
      Program &prog;

      //map from variables to constants for substitution
      std::map<std::string,size_t> idMap;

      //constructors
      SanityChecker(Program &p) : prog(p) {}

      //update substitution mapping
      void addIdMap(const std::string &s,size_t i);
      void delIdMap(const std::string &s);

      //dispatchers
      void exitCall(CallStmt &stmt);
      bool enterFAN(FANStmt &stmt) { return false; }
      void exitFAN(FANStmt &stmt);
      bool enterFADNP(FADNPStmt &stmt) { return false; }
      void exitFADNP(FADNPStmt &stmt);
    };
  } //namespace program
}

#endif // _DAIG_PROGRAM_H_
