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


//a class for sequentializing DAIG into a C program

#ifndef __ARRAY_ELIM_HPP__
#define __ARRAY_ELIM_HPP__

#include <iostream>
#include "DaigBuilder.hpp"
#include "daig/CProgram.h"
#include "CopyVisitor.hpp"

namespace daig {

  /*******************************************************************/
  //array eliminator
  /*******************************************************************/
  class ArrayElim : public CopyVisitor
  {
  public:
    ///the input program with arrays
    CProgram &inProg;

    ///the output program without arrays
    CProgram outProg;

    ///whether to add an initializer for globals at the beginning of
    ///main
    bool initGlobals;

    ///constructor
    ArrayElim(CProgram &ip,bool ig);

    //existing setter and getter functions
    std::map<std::string,Expr> getters,setters;

    void expandArrayVar(const Variable &var);

    void createGetterBody(const std::string &varName,const Expr &cond,
                          const Type &type,const VarList &params,
                          StmtList &body);
    Expr createGetter(const LvalExpr &expr);
    void createSetterBody(const std::string &varName,const Expr &cond,
                          const Type &type,const VarList &params,
                          StmtList &body);
    Expr createSetter(const LvalExpr &expr);

    //dispatchers for visitor
    void exitLval(LvalExpr &expr);
    bool enterAsgn(AsgnStmt &stmt) { return false; }
    void exitAsgn(AsgnStmt &stmt);

    ///do array elimination
    void run();
  };
} //namespace daig

#endif //__ARRAY_ELIM_HPP__
