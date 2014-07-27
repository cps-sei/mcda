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

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include "daig/Type.h"
#include "daig/Variable.h"
#include "daig/Expression.h"
#include "daig/Statement.h"
#include "daig-parser.hpp"
#include "daslc/daig-parser.hpp"
#include "ArrayElim.hpp"

/*********************************************************************/
//constructor
/*********************************************************************/
daig::ArrayElim::ArrayElim(CProgram &ip,bool ig) 
  : inProg(ip),initGlobals(ig) {}

/*********************************************************************/
//recursively descent into an array variable to expand it into its
//elements. for example, an array variable int x[2] is expanded into
//two variables int x_0 and int x_1
/*********************************************************************/
void daig::ArrayElim::expandArrayVar(const Variable &var)
{
  if(!var.type->isArray()) {
    outProg.addGlobVar(var);
    return;
  }

  int dim = var.type->getFirstDim();
  for(int i = 0;i < dim;++i) {
    Variable v1 = var.decrDim();
    Variable v2 = v1.instName("_" + boost::lexical_cast<std::string>(i));
    expandArrayVar(v2);
  }
}
                                       
/*********************************************************************/
//recursively create the statements in the body of a getter
/*********************************************************************/
void daig::ArrayElim::createGetterBody(const std::string &varName,const Expr &cond,
                                       const Type &type,const VarList &params,
                                       StmtList &body)
{
  //no more indices to cover
  if(params.empty()) {
    Expr retVal(new LvalExpr(varName));
    Stmt retStmt(new RetStmt(retVal));
    body.push_back(Stmt(new ITStmt(cond,retStmt)));
    return;
  }

  //more parameters to cover
  int dim = type->getFirstDim();
  Type newType = type->decrDim();
  VarList newParams = params;
  newParams.pop_front();
  const Variable &param = *(params.begin());

  for(int i = 0;i < dim;++i) {
    std::string newVarName = varName + "_" + boost::lexical_cast<std::string>(i);
    Expr eq(new CompExpr(TCEQ,Expr(new LvalExpr(param.name)),Expr(new IntExpr(i))));
    Expr newCond = cond.get() ? Expr(new CompExpr(TLAND, cond, eq)) : eq;
    createGetterBody(newVarName,newCond,newType,newParams,body);
  }
}
                                       
/*********************************************************************/
//create the getter for an array
/*********************************************************************/
daig::Expr daig::ArrayElim::createGetter(const LvalExpr &expr)
{
  //return existing
  std::map<std::string,Expr>::iterator it = getters.find(expr.var);
  if(it != getters.end()) return it->second;

  //find element type of array
  Variables::iterator git = inProg.globVars.find(expr.var);
  assert(git != inProg.globVars.end() && 
         "ERROR: can only eliminate global arrays!");
  Type elemType = git->second.type->getElemType();

  //create new getter
  std::string fnName = "get_" + expr.var;
  Expr getFunc(new LvalExpr(fnName));
  
  //create parameters
  daig::VarList params;
  for(unsigned int i = 0;i < expr.indices.size();++i) {
    params.push_back(Variable("idx_" + boost::lexical_cast<std::string>(i),daig::ucharType()));
  }

  //create the body
  StmtList body;
  createGetterBody(expr.var,Expr(),git->second.type,params,body);

  //add a return 0 statement at the end
  body.push_back(Stmt(new RetStmt(Expr(new IntExpr(0)))));

  //create and add the function to the result C program
  outProg.addFunction(Function(elemType,fnName,params,daig::VarList(),body));

  getters[expr.var] = getFunc;
  return getFunc;
}
                                       
/*********************************************************************/
//recursively create the statements in the body of a setter
/*********************************************************************/
void daig::ArrayElim::createSetterBody(const std::string &varName,const Expr &cond,
                                       const Type &type,const VarList &params,
                                       StmtList &body)
{
  //no more indices to cover -- the last parameter is the value to be
  //assigned
  if(params.size() == 1) {
    Expr lhs(new LvalExpr(varName));
    Expr rhs(new LvalExpr(params.begin()->name));
    Stmt asgn(new AsgnStmt(lhs,rhs));
    body.push_back(Stmt(new ITStmt(cond,asgn)));
    return;
  }

  //more parameters to cover
  int dim = type->getFirstDim();
  Type newType = type->decrDim();
  VarList newParams = params;
  newParams.pop_front();
  const Variable &param = *(params.begin());

  for(int i = 0;i < dim;++i) {
    std::string newVarName = varName + "_" + boost::lexical_cast<std::string>(i);
    Expr eq(new CompExpr(TCEQ,Expr(new LvalExpr(param.name)),Expr(new IntExpr(i))));
    Expr newCond = cond.get() ? Expr(new CompExpr(TLAND, cond, eq)) : eq;
    createSetterBody(newVarName,newCond,newType,newParams,body);
  }
}

/*********************************************************************/
//create the setter for an array
/*********************************************************************/
daig::Expr daig::ArrayElim::createSetter(const LvalExpr &expr)
{
  //return existing
  std::map<std::string,Expr>::iterator it = setters.find(expr.var);
  if(it != setters.end()) return it->second;

  //find element type of array
  Variables::iterator git = inProg.globVars.find(expr.var);
  assert(git != inProg.globVars.end() && 
         "ERROR: can only eliminate global arrays!");
  Type elemType = git->second.type->getElemType();

  //create new setter
  std::string fnName = "set_" + expr.var;
  Expr setFunc(new LvalExpr(fnName));
  
  //create parameters
  daig::VarList params;
  for(unsigned int i = 0;i < expr.indices.size();++i) {
    params.push_back(Variable("idx_" + boost::lexical_cast<std::string>(i),daig::ucharType()));
  }
  params.push_back(Variable("val", elemType));

  //create the body
  StmtList body;
  createSetterBody(expr.var,Expr(),git->second.type,params,body);

  //create and add the function to the result C program
  outProg.addFunction(Function(voidType(),fnName,params,daig::VarList(),body));

  setters[expr.var] = setFunc;
  return setFunc;
}

/*********************************************************************/
//visitor dispatchers
/*********************************************************************/
void daig::ArrayElim::exitLval(LvalExpr &expr)
{
  //not an array element
  if(expr.indices.empty()) {
    exprMap[hostExpr] = hostExpr;
    return;
  }

  //array element
  exprMap[hostExpr] = Expr(new CallExpr(createGetter(expr),expr.indices));
}

void daig::ArrayElim::exitAsgn(AsgnStmt &stmt)
{
  Stmt shost = hostStmt;

  visit(stmt.rhs);

  LvalExpr *lve = dynamic_cast<LvalExpr*>(stmt.lhs.get());
  assert(lve && "ERROR: lhs of assignment must be an LvalExpr!!");  

  //if lhs is not an array element
  if(lve->indices.empty()) {
    stmtMap[shost] = Stmt(new AsgnStmt(stmt.lhs,exprMap[stmt.rhs]));
    return;
  }

  //create the setter function and call it
  ExprList args = lve->indices;
  args.push_back(exprMap[stmt.rhs]);
  stmtMap[shost] = Stmt(new CallStmt(Expr(new CallExpr(createSetter(*lve),args))));
}

/*********************************************************************/
//run the array eliminator -- construct the output program
/*********************************************************************/
void daig::ArrayElim::run()
{
  //copy over external function declarations
  outProg.externalFuncs = inProg.externalFuncs;

  //convert global variable arrays to individual variable elements
  BOOST_FOREACH(const Variables::value_type &v,inProg.globVars) {
    expandArrayVar(v.second);
  }

  //convert array accesses in each function
  BOOST_FOREACH(const Functions::value_type &v,inProg.funcs) {
    const Function &func = v.second;
    daig::VarList fnParams,fnTemps;
    BOOST_FOREACH(const Variables::value_type &v,func.params)
      fnParams.push_back(v.second);
    BOOST_FOREACH(const Variables::value_type &v,func.temps)
      fnTemps.push_back(v.second);

    StmtList fnBody;
    BOOST_FOREACH(const Stmt &s,func.body) {
      visit(s);
      fnBody.push_back(stmtMap[s]);
    }

    //if needed, create and push a global initializer
    if(initGlobals && func.name == "main") {
      daig::StmtList igBody;
      BOOST_FOREACH(const Variables::value_type &v,outProg.globVars) {
        igBody.push_back(Stmt(new AsgnStmt(Expr(new LvalExpr(v.second.name)),
                                           Expr(new IntExpr(0)))));
      }

      outProg.addFunction(Function(daig::voidType(),"init_globals",daig::VarList(),
                                   daig::VarList(),igBody));

      Expr callExpr(new LvalExpr("init_globals"));
      Stmt callStmt(new CallStmt(callExpr,daig::ExprList()));
      fnBody.push_front(callStmt);
    }

    outProg.addFunction(Function(func.retType,func.name,fnParams,fnTemps,fnBody));
  }
}

/*********************************************************************/
//end of file
/*********************************************************************/
