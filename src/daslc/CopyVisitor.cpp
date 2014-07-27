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
#include "daig/Type.h"
#include "daig/Variable.h"
#include "daig/Expression.h"
#include "daig/Statement.h"
#include "daig-parser.hpp"
#include "CopyVisitor.hpp"

/*********************************************************************/
//collect results for an expr list
/*********************************************************************/
daig::ExprList daig::CopyVisitor::collect(const ExprList &el)
{
  daig::ExprList res;
  BOOST_FOREACH(const daig::Expr &e,el) res.push_back(exprMap[e]);
  return res;
}

/*********************************************************************/
//collect results for an stmt list
/*********************************************************************/
daig::StmtList daig::CopyVisitor::collect(const StmtList &sl)
{
  daig::StmtList res;
  BOOST_FOREACH(const daig::Stmt &s,sl) res.push_back(stmtMap[s]);
  return res;
}

/*********************************************************************/
//dispatchers for expressions
/*********************************************************************/

void daig::CopyVisitor::exitInt(daig::IntExpr &expr)
{
  exprMap[hostExpr] = hostExpr;
}

void daig::CopyVisitor::exitLval(daig::LvalExpr &expr)
{
  exprMap[hostExpr] = hostExpr;
}

void daig::CopyVisitor::exitComp(daig::CompExpr &expr)
{
  exprMap[hostExpr] = daig::Expr(new daig::CompExpr(expr.op,collect(expr.args)));
}

void daig::CopyVisitor::exitCall(daig::CallExpr &expr)
{
  exprMap[hostExpr] = daig::Expr(new daig::CallExpr(exprMap[expr.func],
                                                    collect(expr.args)));
}

void daig::CopyVisitor::exitEXO(daig::EXOExpr &expr)
{
  exprMap[hostExpr] = daig::Expr(new daig::EXOExpr(expr.id,exprMap[expr.arg]));
}

void daig::CopyVisitor::exitEXH(daig::EXHExpr &expr)
{
  exprMap[hostExpr] = daig::Expr(new daig::EXHExpr(expr.id,exprMap[expr.arg]));
}

void daig::CopyVisitor::exitEXL(daig::EXLExpr &expr)
{
  exprMap[hostExpr] = daig::Expr(new daig::EXLExpr(expr.id,exprMap[expr.arg]));
}

/*********************************************************************/
//dispatchers for statements
/*********************************************************************/

void daig::CopyVisitor::exitAtomic(daig::AtomicStmt &stmt)
{
  stmtMap[hostStmt] = Stmt(new daig::AtomicStmt(stmtMap[stmt.data]));
}

void daig::CopyVisitor::exitPrivate(daig::PrivateStmt &stmt)
{
  stmtMap[hostStmt] = Stmt(new daig::PrivateStmt(stmtMap[stmt.data]));
}

void daig::CopyVisitor::exitBlock(daig::BlockStmt &stmt)
{
  stmtMap[hostStmt] = Stmt(new BlockStmt(collect(stmt.data)));
}

void daig::CopyVisitor::exitAsgn(daig::AsgnStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new AsgnStmt(exprMap[stmt.lhs],exprMap[stmt.rhs]));

  //std::cout << "**************************************\n";
  //hostStmt->print(std::cout,0);
  //stmtMap[hostStmt]->print(std::cout,0);
}

void daig::CopyVisitor::exitIT(daig::ITStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new ITStmt(exprMap[stmt.cond], stmtMap[stmt.tbranch]));
}

void daig::CopyVisitor::exitITE(daig::ITEStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new ITEStmt(exprMap[stmt.cond], stmtMap[stmt.tbranch], 
                                       stmtMap[stmt.ebranch]));
}

void daig::CopyVisitor::exitFor(daig::ForStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new ForStmt(collect(stmt.init),collect(stmt.test),
                                       collect(stmt.update),stmtMap[stmt.body]));
}

void daig::CopyVisitor::exitWhile(daig::WhileStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new WhileStmt(exprMap[stmt.cond],stmtMap[stmt.body]));
}

void daig::CopyVisitor::exitBreak(daig::BreakStmt &stmt) 
{ 
  stmtMap[hostStmt] = hostStmt; 
}

void daig::CopyVisitor::exitCont(daig::ContStmt &stmt) 
{ 
  stmtMap[hostStmt] = hostStmt; 
}

void daig::CopyVisitor::exitRet(daig::RetStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new RetStmt(exprMap[stmt.retVal]));
}

void daig::CopyVisitor::exitRetVoid(daig::RetVoidStmt &stmt) 
{ 
  stmtMap[hostStmt] = hostStmt; 
}

void daig::CopyVisitor::exitCall(daig::CallStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new CallStmt(exprMap[stmt.data]));
}

void daig::CopyVisitor::exitFAN(daig::FANStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new FANStmt(stmt.id,stmtMap[stmt.data]));
}

void daig::CopyVisitor::exitFADNP(daig::FADNPStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new FADNPStmt(stmt.id1,stmt.id2,stmtMap[stmt.data]));
}

void daig::CopyVisitor::exitFAO(daig::FAOStmt &stmt) 
{ 
  stmtMap[hostStmt] = Stmt(new FAOStmt(stmt.id,stmtMap[stmt.data]));
}

void daig::CopyVisitor::exitFAOL(daig::FAOLStmt &stmt) 
{
  stmtMap[hostStmt] = Stmt(new FAOLStmt(stmt.id,stmtMap[stmt.data]));
}

void daig::CopyVisitor::exitFAOH(daig::FAOHStmt &stmt) 
{
  stmtMap[hostStmt] = Stmt(new FAOHStmt(stmt.id,stmtMap[stmt.data]));
}

/*********************************************************************/
//end of file
/*********************************************************************/
