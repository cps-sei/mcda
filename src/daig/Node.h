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

#ifndef _DAIG_NODE_H_
#define _DAIG_NODE_H_

/**
 * @file Node.h
 * @author James Edmondson <jedmondson@gmail.com>
 *
 * This file contains the definition of a node/process.
 **/

#include <vector>
#include <map>
#include <string>
#include "Function.h"
#include "Variable.h"


namespace daig
{
  /**
    * @class Node
    * @brief Represents a process in a distributed program
    */
  class Node
  {
  public:    
    /**
     * The node name
     **/
    std::string name;

    ///the node arguments
    std::list<std::string> args;

    /**
     * list of global variables
     **/
    Variables globVars;

    ///list of local variables
    Variables locVars;
    
    /**
     * A map of function names to function definitions
     **/
    Functions funcs;

    /**
     * NOTE: NODE_INIT and periodic functions are added to funcs
     **/

    /**
     * Periodic function names and periods (actual functions are in funcs)
     * Preserve the order of the functions as they appear in dasl program
     */
    std::vector<std::string> periodic_func_names;
    std::map<std::string, int> periods;

    ///constructors
    Node() {}

    ///clear the node -- reset it to an empty node
    void clear()
    {
      name.clear(); args.clear(); 
      globVars.clear(); locVars.clear();
      funcs.clear();
    }

    ///add a global variable
    void addGlobalVar(const std::list<Variable> &vl)
    {
      BOOST_FOREACH(const Variable &v,vl) {
        assert(globVars.count(v.name) == 0 && "ERROR: global variable redeclared!!");
        globVars[v.name] = v;
        globVars[v.name].scope = Variable::GLOBAL;
      }
    }

    ///add a local variable
    void addLocalVar(const std::list<Variable> &vl)
    {
      BOOST_FOREACH(const Variable &v,vl) {
        assert(locVars.count(v.name) == 0 && "ERROR: local variable redeclared!!");
        locVars[v.name] = v;
        locVars[v.name].scope = Variable::LOCAL;
      }
    }

    ///add a function
    void addFunction(const Function &f) { funcs[f.name] = f; }

    ///add a periodic function and its period
    void addPeriodicFunction(const Function &f, int T)
    {
      addFunction(f);
      periodic_func_names.push_back(f.name);
      periods[f.name] = T;
    }

    /**
     * Prints function information
     * @param  indent  spaces to indent printout
     **/
    void print (std::ostream &os,unsigned int indent);
  };


  /**
   * A mapping of node types to node definitions
   **/
  typedef std::map <std::string, Node> Nodes;
}

#endif // _DAIG_NODE_H_
