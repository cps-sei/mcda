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

#include <iostream>
#include <boost/foreach.hpp>
#include "Visitor.h"

/*********************************************************************/
//top level dispatch method for expressions
/*********************************************************************/
void daig::Visitor::visit(const daig::Expr &expr)
{
  //std::cout << "*************** visiting expr *******\n";
  //std::cout << expr->toString() << '\n';

  if(IntExpr *ex = dynamic_cast<IntExpr*>(&*expr)) {
    hostExpr = expr; enterInt(*ex);
    hostExpr = expr; exitInt(*ex);
  } else if(DoubleExpr *ex = dynamic_cast<DoubleExpr*>(&*expr)) {
    hostExpr = expr; enterDouble(*ex);
    hostExpr = expr; exitDouble(*ex);
  } else if(LvalExpr *ex = dynamic_cast<LvalExpr*>(&*expr)) {
    hostExpr = expr; 
    if(enterLval(*ex)) BOOST_FOREACH(Expr &e,ex->indices) visit(e);
    hostExpr = expr; exitLval(*ex);
  } else if(CompExpr *ex = dynamic_cast<CompExpr*>(&*expr)) {
    hostExpr = expr; 
    if(enterComp(*ex)) BOOST_FOREACH(Expr &e,ex->args) visit(e);
    hostExpr = expr; exitComp(*ex);
  } else if(CallExpr *ex = dynamic_cast<CallExpr*>(&*expr)) {
    hostExpr = expr; if(enterCall(*ex)) { 
      visit(ex->func);
      BOOST_FOREACH(Expr &e,ex->args) visit(e);
    }
    hostExpr = expr; exitCall(*ex);
  } else if(EXOExpr *ex = dynamic_cast<EXOExpr*>(&*expr)) {
    hostExpr = expr; if(enterEXO(*ex)) visit(ex->arg);
    hostExpr = expr; exitEXO(*ex);
  } else if(EXHExpr *ex = dynamic_cast<EXHExpr*>(&*expr)) {
    hostExpr = expr; if(enterEXH(*ex)) visit(ex->arg);
    hostExpr = expr; exitEXH(*ex);
  } else if(EXLExpr *ex = dynamic_cast<EXLExpr*>(&*expr)) {
    hostExpr = expr; if(enterEXL(*ex)) visit(ex->arg);
    hostExpr = expr; exitEXL(*ex);
  } else assert(0 && "ERROR : visiting unknown expression!");
}

/*********************************************************************/
//top level dispatch method for statements
/*********************************************************************/
void daig::Visitor::visit(const daig::Stmt &stmt)
{
  //std::cout << "*************** visiting stmt *******\n";
  //stmt->print(std::cout,0);

  if(AtomicStmt *st = dynamic_cast<AtomicStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterAtomic(*st)) visit(st->data);
    hostStmt = stmt; exitAtomic(*st);
  } else if(PrivateStmt *st = dynamic_cast<PrivateStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterPrivate(*st)) visit(st->data);
    hostStmt = stmt; exitPrivate(*st);
  } else if(BlockStmt *st = dynamic_cast<BlockStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterBlock(*st)) BOOST_FOREACH(Stmt &s,st->data) visit(s);
    hostStmt = stmt; exitBlock(*st);
  } else if(AsgnStmt *st = dynamic_cast<AsgnStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterAsgn(*st)) { visit(st->lhs); visit(st->rhs); }
    hostStmt = stmt; exitAsgn(*st);
  } else if(ITStmt *st = dynamic_cast<ITStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterIT(*st)) { visit(st->cond); visit(st->tbranch); }
    hostStmt = stmt; exitIT(*st);
  } else if(ITEStmt *st = dynamic_cast<ITEStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterITE(*st)) { 
      visit(st->cond); visit(st->tbranch); visit(st->ebranch); 
    }
    hostStmt = stmt; exitITE(*st);
  } else if(ForStmt *st = dynamic_cast<ForStmt*>(&*stmt)) {
    hostStmt = stmt; 
    if(enterFor(*st)) {
      BOOST_FOREACH(Stmt &s,st->init) visit(s);
      BOOST_FOREACH(Expr &e,st->test) visit(e);
      BOOST_FOREACH(Stmt &s,st->update) visit(s);
      visit(st->body);
    }
    hostStmt = stmt; exitFor(*st);
  } else if(WhileStmt *st = dynamic_cast<WhileStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterWhile(*st)) { visit(st->cond); visit(st->body); }
    hostStmt = stmt; exitWhile(*st);
  } else if(BreakStmt *st = dynamic_cast<BreakStmt*>(&*stmt)) {
    hostStmt = stmt; enterBreak(*st);
    hostStmt = stmt; exitBreak(*st);
  } else if(ContStmt *st = dynamic_cast<ContStmt*>(&*stmt)) {
    hostStmt = stmt; enterCont(*st);
    hostStmt = stmt; exitCont(*st);
  } else if(RetStmt *st = dynamic_cast<RetStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterRet(*st)) visit(st->retVal);
    hostStmt = stmt; exitRet(*st);
  } else if(RetVoidStmt *st = dynamic_cast<RetVoidStmt*>(&*stmt)) {
    hostStmt = stmt; enterRetVoid(*st);
    hostStmt = stmt; exitRetVoid(*st);
  } else if(EXITStmt *st = dynamic_cast<EXITStmt*>(&*stmt)) {
    hostStmt = stmt; enterEXIT(*st);
    hostStmt = stmt; exitEXIT(*st);
  } else if(LOGStmt *st = dynamic_cast<LOGStmt*>(&*stmt)) {
    hostStmt = stmt; enterLOG(*st);
    hostStmt = stmt; exitLOG(*st);
  } else if(CallStmt *st = dynamic_cast<CallStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterCall(*st)) visit(st->data);
    hostStmt = stmt; exitCall(*st);
  } else if(FANStmt *st = dynamic_cast<FANStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterFAN(*st)) visit(st->data);
    hostStmt = stmt; exitFAN(*st);
  } else if(FADNPStmt *st = dynamic_cast<FADNPStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterFADNP(*st)) visit(st->data);
    hostStmt = stmt; exitFADNP(*st);
  } else if(FAOStmt *st = dynamic_cast<FAOStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterFAO(*st)) visit(st->data);
    hostStmt = stmt; exitFAO(*st);
  } else if(FAOLStmt *st = dynamic_cast<FAOLStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterFAOL(*st)) visit(st->data);
    hostStmt = stmt; exitFAOL(*st);
  } else if(FAOHStmt *st = dynamic_cast<FAOHStmt*>(&*stmt)) {
    hostStmt = stmt; if(enterFAOH(*st)) visit(st->data);
    hostStmt = stmt; exitFAOH(*st);
  } else assert(0 && "ERROR : visiting unknown statement!");
}

/*********************************************************************/
//end of file
/*********************************************************************/
