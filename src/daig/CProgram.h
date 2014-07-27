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

#ifndef _DAIG_CPROGRAM_H_
#define _DAIG_CPROGRAM_H_

/**
 * @file CProgram.h
 * @author James Edmondson <jedmondson@gmail.com>
 *
 * This file contains a class definition for the DAIG C Program
 * container.
 **/

#include <list>
#include <map>
#include <string>
#include "Model_Of_Computation.h"
#include "Variable.h"
#include "Function.h"
#include "Node.h"


namespace daig
{
  /**
    * @class CProgram
    * @brief Encapsulates a C program definition
    */
  class CProgram
  {
  public:
    /**
     * Prints variable information
     * @param  indent  spaces to indent printout
     **/
    void print (std::ostream &os,unsigned int indent);

    /**
     * external function declarations -- these are Function objects
     * with empty bodies
     **/
    Functions externalFuncs;

    ///global variables
    Variables globVars;
    
    /**
     * A map of function names to function definitions
     **/
    Functions funcs;

    ///add a global variable
    void addGlobVar(const Variable &v) { globVars[v.name] = v; }

    ///add a function
    void addFunction(const Function &f) { funcs[f.name] = f; }
  };
}

#endif // _DAIG_CPROGRAM_H_
