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

#include <boost/lexical_cast.hpp>
#include "daig/Type.h"
#include "daig/Variable.h"
#include "daig/Expression.h"
#include "daig/Statement.h"
#include "daig-parser.hpp"
#include "SyncSeq.hpp"

/*********************************************************************/
//methods for GlobalTransformer
/*********************************************************************/

//add id to int mapping
void daig::syncseq::GlobalTransformer::addIdMap(const std::string &s,size_t i)
{
  idMap[s] = i;
}

//remove id to int mappiing
void daig::syncseq::GlobalTransformer::delIdMap(const std::string &s)
{
  idMap.erase(s);
}

/*********************************************************************/
//dispatchers for expressions
/*********************************************************************/

void daig::syncseq::GlobalTransformer::exitComp(daig::CompExpr &expr)
{
  if(expr.op == TNODENUM)
    exprMap[hostExpr] = Expr(new IntExpr(nodeNum));
  else
    exprMap[hostExpr] = daig::Expr(new daig::CompExpr(expr.op,collect(expr.args)));
}

void daig::syncseq::GlobalTransformer::exitLval(daig::LvalExpr &expr)
{
  exprMap[hostExpr] = hostExpr;

  //substitute global variable name x with x_i
  std::string newName = expr.var;

  //handle assume and assert
  if(newName == "ASSUME") newName = "__CPROVER_assume";
  else if(newName == "ASSERT") newName = "assert";

  //handle global variables
  Node &node = prog.nodes.begin()->second;
  if(node.globVars.count(expr.var)) newName += "_i";

  //substitute .id with its mapping in idMap
  std::map<std::string,size_t>::const_iterator iit = idMap.find(expr.var);
  newName = iit == idMap.end() ? newName : boost::lexical_cast<std::string>(iit->second);

  iit = idMap.find(expr.node);
  newName = iit == idMap.end() ? newName : 
    newName + "_" + boost::lexical_cast<std::string>(iit->second);

  exprMap[hostExpr] = daig::Expr(new daig::LvalExpr(newName,collect(expr.indices)));
  
  //std::cout << "**************************************\n";
  //std::cout << hostExpr->toString() << '\n';
  //std::cout << exprMap[hostExpr]->toString() << '\n';
}

/*********************************************************************/
//dispatchers for statements
/*********************************************************************/

void daig::syncseq::GlobalTransformer::exitAtomic(daig::AtomicStmt &stmt)
{
  stmtMap[hostStmt] = stmtMap[stmt.data];
}

void daig::syncseq::GlobalTransformer::exitPrivate(daig::PrivateStmt &stmt)
{
  stmtMap[hostStmt] = stmtMap[stmt.data];
}

void daig::syncseq::GlobalTransformer::exitCall(daig::CallStmt &stmt) 
{ 
  //handle calls to ND(x) -- assign x non-deterministically
  CallExpr *expr = dynamic_cast<CallExpr*>(stmt.data.get());
  if(expr->func->toString() == "ND") {
    const Expr &arg = exprMap[*(expr->args.begin())];
    Expr ndfn = syncSeq.createNondetFunc(arg);
    Expr ndcall(new CallExpr(ndfn,ExprList()));
    stmtMap[hostStmt] = Stmt(new AsgnStmt(arg,ndcall));
    return;
  }

  stmtMap[hostStmt] = Stmt(new CallStmt(exprMap[stmt.data]));
}

void daig::syncseq::GlobalTransformer::exitFAN(daig::FANStmt &stmt) 
{ 
  Stmt shost = hostStmt;
  StmtList sl;

  for(size_t i = 0;i < nodeNum;++i) {
    addIdMap(stmt.id,i);
    visit(stmt.data);
    sl.push_back(stmtMap[stmt.data]);
    delIdMap(stmt.id);
  }

  stmtMap[shost] = Stmt(new daig::BlockStmt(sl));
}

void daig::syncseq::GlobalTransformer::exitFADNP(daig::FADNPStmt &stmt) 
{ 
  Stmt shost = hostStmt;
  StmtList sl;

  for(size_t i1 = 0;i1 < nodeNum;++i1) {
    for(size_t i2 = i1+1;i2 < nodeNum;++i2) {
      addIdMap(stmt.id1,i1);
      addIdMap(stmt.id2,i2);
      visit(stmt.data);
      sl.push_back(stmtMap[stmt.data]);
      delIdMap(stmt.id1);
      delIdMap(stmt.id2);
    }
  }

  stmtMap[shost] = Stmt(new daig::BlockStmt(sl));
}

/*********************************************************************/
//methods for NodeTransformer
/*********************************************************************/

void daig::syncseq::NodeTransformer::exitLval(daig::LvalExpr &expr)
{
  exprMap[hostExpr] = hostExpr;

  //substitute global variable name x with x_i
  std::string newName = expr.var;

  //handle function call -- only change name if the function is not
  //external
  if(inCall && !prog.isExternalFunction(newName)) 
    newName += ("_" + boost::lexical_cast<std::string>(nodeId));

  //handle global variables -- distinguishing between lhs of
  //assignments and other cases
  Node &node = prog.nodes.begin()->second;
  if(node.globVars.count(expr.var)) {
    if(inLhs) newName += "_f";
    else newName += "_i";
  }

  //handle local variables
  if(node.locVars.count(expr.var))
    newName += "_" + boost::lexical_cast<std::string>(nodeId);

  //substitute .id with its mapping in idMap
  std::map<std::string,size_t>::const_iterator iit = idMap.find(expr.var);
  newName = iit == idMap.end() ? newName : boost::lexical_cast<std::string>(iit->second);

  iit = idMap.find(expr.node);
  newName = iit == idMap.end() ? newName : 
    newName + "_" + boost::lexical_cast<std::string>(iit->second);

  exprMap[hostExpr] = daig::Expr(new daig::LvalExpr(newName,collect(expr.indices)));
  
  //std::cout << "**************************************\n";
  //std::cout << hostExpr->toString() << '\n';
  //std::cout << exprMap[hostExpr]->toString() << '\n';
}

void daig::syncseq::NodeTransformer::exitCall(daig::CallExpr &expr) 
{
  Expr shost = hostExpr;
  inCall = true;
  visit(expr.func);
  inCall = false;
  BOOST_FOREACH(Expr &e,expr.args) visit(e);
  exprMap[shost] = daig::Expr(new daig::CallExpr(exprMap[expr.func],collect(expr.args)));
}

//compute disjunction over all other node ids
void daig::syncseq::NodeTransformer::exitEXO(daig::EXOExpr &expr)
{
  Expr shost = hostExpr;
  exprMap[shost] = Expr();
  for(size_t i = 0;i < nodeNum;++i) {
    if(i == nodeId) continue;
    addIdMap(expr.id,i);
    visit(expr.arg);
    delIdMap(expr.id);
    if(exprMap[shost].get()) 
      exprMap[shost] = 
        daig::Expr(new daig::CompExpr(TLOR,exprMap[shost],exprMap[expr.arg]));
    else
      exprMap[shost] = exprMap[expr.arg];
  }

  //turn empty disjunct into "0"
  if(!exprMap[shost].get())
    exprMap[shost] = Expr(new daig::IntExpr(0));
}

//compute disjunction over all higher node ids
void daig::syncseq::NodeTransformer::exitEXH(daig::EXHExpr &expr)
{
  Expr shost = hostExpr;
  exprMap[shost] = Expr();
  for(size_t i = nodeId+1;i < nodeNum;++i) {
    addIdMap(expr.id,i);
    visit(expr.arg);
    delIdMap(expr.id);
    if(exprMap[shost].get()) 
      exprMap[shost] = 
        daig::Expr(new daig::CompExpr(TLOR,exprMap[shost],exprMap[expr.arg]));
    else
      exprMap[shost] = exprMap[expr.arg];
  }

  //turn empty disjunct into "0"
  if(!exprMap[shost].get())
    exprMap[shost] = Expr(new daig::IntExpr(0));
}

//compute disjunction over all lower node ids
void daig::syncseq::NodeTransformer::exitEXL(daig::EXLExpr &expr)
{
  Expr shost = hostExpr;
  exprMap[shost] = Expr();
  for(size_t i = 0;i < nodeId;++i) {
    addIdMap(expr.id,i);
    visit(expr.arg);
    delIdMap(expr.id);
    if(exprMap[shost].get()) 
      exprMap[shost] = 
        daig::Expr(new daig::CompExpr(TLOR,exprMap[shost],exprMap[expr.arg]));
    else
      exprMap[shost] = exprMap[expr.arg];
  }

  //turn empty disjunct into "0"
  if(!exprMap[shost].get())
    exprMap[shost] = Expr(new daig::IntExpr(0));
}

void daig::syncseq::NodeTransformer::exitAsgn(daig::AsgnStmt &stmt)
{
  Stmt shost = hostStmt;
  inLhs = true; visit(stmt.lhs); inLhs = false;
  visit(stmt.rhs);
  stmtMap[shost] = daig::Stmt(new daig::AsgnStmt(exprMap[stmt.lhs],exprMap[stmt.rhs]));
}

void daig::syncseq::NodeTransformer::exitFAO(daig::FAOStmt &stmt)
{
  Stmt shost = hostStmt;
  StmtList sl;

  for(size_t i = 0;i < nodeNum;++i) {
    if(i == nodeId) continue;
    addIdMap(stmt.id,i);
    visit(stmt.data);
    sl.push_back(stmtMap[stmt.data]);
    delIdMap(stmt.id);
  }

  stmtMap[shost] = Stmt(new daig::BlockStmt(sl));
}

void daig::syncseq::NodeTransformer::exitFAOL(daig::FAOLStmt &stmt)
{
  Stmt shost = hostStmt;
  StmtList sl;

  for(size_t i = 0;i < nodeId;++i) {
    addIdMap(stmt.id,i);
    visit(stmt.data);
    sl.push_back(stmtMap[stmt.data]);
    delIdMap(stmt.id);
  }

  stmtMap[shost] = Stmt(new daig::BlockStmt(sl));
}

void daig::syncseq::NodeTransformer::exitFAOH(daig::FAOHStmt &stmt)
{
  Stmt shost = hostStmt;
  StmtList sl;

  for(size_t i = nodeId+1;i < nodeNum;++i) {
    addIdMap(stmt.id,i);
    visit(stmt.data);
    sl.push_back(stmtMap[stmt.data]);
    delIdMap(stmt.id);
  }

  stmtMap[shost] = Stmt(new daig::BlockStmt(sl));
}

/*********************************************************************/
//constructor
/*********************************************************************/
daig::SyncSeq::SyncSeq(daig::DaigBuilder &b,int r) 
  : builder(b),roundNum(r)
{
  nodeNum = builder.program.processes.size();
}

/*********************************************************************/
//create the global variables
/*********************************************************************/
void daig::SyncSeq::createGlobVars()
{
  Node &node = builder.program.nodes.begin()->second;

  //instantiate node-global variables by replacing dimension #N with
  //nodeNum -- make two copies, one for initial value for a round, and
  //the other for the final value for a round
  daig::VarList gvars;
  BOOST_FOREACH(Variables::value_type &v,node.globVars) {
    gvars.push_back(v.second.instDim(nodeNum));
  }
  BOOST_FOREACH(const Variable &v,gvars) {
    cprog.addGlobVar(v.instName("_i"));
    cprog.addGlobVar(v.instName("_f"));
  }

  //instantiate node-local variables by adding _i for each node id i
  BOOST_FOREACH(Variables::value_type &v,node.locVars) {
    for(size_t i = 0;i < nodeNum;++i)
      cprog.addGlobVar(v.second.instName(std::string("_") + 
                                         boost::lexical_cast<std::string>(i)));
  }
}

/*********************************************************************/
//create list of statements that copy new value of var into old value
//of var. append list of statements to res.
/*********************************************************************/
void daig::SyncSeq::createCopyStmts(bool fwd,const Variable &var,StmtList &res,ExprList indx)
{
  //non-array type
  if(var.type->dims.empty()) {
    Expr lhs(new LvalExpr(var.name + (fwd ? "_f" : "_i"),indx));
    Expr rhs(new LvalExpr(var.name + (fwd ? "_i" : "_f"),indx));
    Stmt stmt(new AsgnStmt(lhs,rhs));
    res.push_back(stmt);
    return;
  }

  //array type -- peel off the first dimension and iterate over it
  //recursively
  int dim = *(var.type->dims.begin());
  for(int i = 0;i < dim;++i) {
    ExprList newIndx = indx;
    newIndx.push_back(Expr(new IntExpr(i)));
    Variable newVar = var.decrDim();
    createCopyStmts(fwd,newVar,res,newIndx);
  }
}

/*********************************************************************/
//create one function that copies global variables back from _f to _i
//at the end of a round, and another that copies global variables from
//_i to _f at the beginning of the round
/*********************************************************************/
void daig::SyncSeq::createRoundCopier()
{
  Node &node = builder.program.nodes.begin()->second;
  daig::VarList fnParams,fnTemps;

  //create the copier from _f to _i
  StmtList fnBody1;
  BOOST_FOREACH(Variables::value_type &v,node.globVars) {
    Variable var = v.second.instDim(nodeNum);
    createCopyStmts(0,var,fnBody1,ExprList());
  }

  Function func1(daig::voidType(),"round_bwd_copier",fnParams,fnTemps,fnBody1);
  cprog.addFunction(func1);

  //create the copier from _i to _f
  StmtList fnBody2;
  BOOST_FOREACH(Variables::value_type &v,node.globVars) {
    Variable var = v.second.instDim(nodeNum);
    createCopyStmts(1,var,fnBody2,ExprList());
  }

  Function func2(daig::voidType(),"round_fwd_copier",fnParams,fnTemps,fnBody2);
  cprog.addFunction(func2);
}

/*********************************************************************/
//create the main function
/*********************************************************************/
void daig::SyncSeq::createMainFunc()
{
  daig::VarList mainParams,mainTemps;
  StmtList mainBody,roundBody;

  //call SAFETY()
  Expr callExpr1(new LvalExpr("SAFETY"));
  Stmt callStmt1(new CallStmt(callExpr1,daig::ExprList()));
  roundBody.push_back(callStmt1);

  //call fowward round copier
  Expr callExpr4(new LvalExpr("round_fwd_copier"));
  Stmt callStmt4(new CallStmt(callExpr4,daig::ExprList()));
  roundBody.push_back(callStmt4);

  //call ROUND function of each node
  for(size_t i = 0;i < nodeNum;++i) {
    std::string callName = std::string("ROUND_") + boost::lexical_cast<std::string>(i);
    Expr callExpr(new LvalExpr(callName));
    Stmt callStmt(new CallStmt(callExpr,daig::ExprList()));
    roundBody.push_back(callStmt);
  }

  //call backward round copier
  Expr callExpr2(new LvalExpr("round_bwd_copier"));
  Stmt callStmt2(new CallStmt(callExpr2,daig::ExprList()));
  roundBody.push_back(callStmt2);

  //add call to INIT()
  Expr callExpr3(new LvalExpr("INIT"));
  Stmt callStmt3(new CallStmt(callExpr3,daig::ExprList()));
  mainBody.push_back(callStmt3);

  //if number of rounds not specified, add an infinite loop
  if(roundNum == -1) {
    Stmt forBody(new BlockStmt(roundBody));
    mainBody.push_back(Stmt(new ForStmt(StmtList(),ExprList(),StmtList(),forBody)));
  }
  //otherwise statically unroll the loop roundNum times and the call
  //SAFETY again one more time
  else {
    for(int i = 0;i < roundNum;++i)
      mainBody.insert(mainBody.end(),roundBody.begin(),roundBody.end());
    mainBody.push_back(callStmt1);
  }

  Function mainFunc(daig::intType(),"main",mainParams,mainTemps,mainBody);
  cprog.addFunction(mainFunc);
}

/*********************************************************************/
//create the INIT() function
/*********************************************************************/
void daig::SyncSeq::createInit()
{
  daig::VarList fnParams,fnTemps;
  StmtList fnBody;

  //if no INIT() defined, create an empty one
  daig::Functions::iterator fit = builder.program.funcs.find("INIT");
  if(fit == builder.program.funcs.end()) {
    std::cout << "node does not have a INIT function, creating an empty one ...\n";
    Function func(daig::voidType(),"INIT",fnParams,fnTemps,fnBody);
    cprog.addFunction(func);
    return;
  }

  //create parameters
  BOOST_FOREACH(Variables::value_type &v,fit->second.params)
    fnParams.push_back(v.second);

  //create temporary variables
  BOOST_FOREACH(Variables::value_type &v,fit->second.temps)
    fnTemps.push_back(v.second);

  //transform the body of init
  BOOST_FOREACH(const Stmt &st,fit->second.body) {
    syncseq::GlobalTransformer gt(*this,builder.program,nodeNum);
    gt.visit(st);
    fnBody.push_back(gt.stmtMap[st]);
  }

  Function func(daig::voidType(),"INIT",fnParams,fnTemps,fnBody);
  cprog.addFunction(func);
}

/*********************************************************************/
//create the SAFETY() function
/*********************************************************************/
void daig::SyncSeq::createSafety()
{
  Node &node = builder.program.nodes.begin()->second;
  daig::VarList fnParams,fnTemps;
  StmtList fnBody;

  //if no SAFETY() defined, create an empty one
  daig::Functions::iterator fit = builder.program.funcs.find("SAFETY");
  if(fit == builder.program.funcs.end()) {
    std::cout << "node does not have a SAFETY function, creating an empty one ...\n";
    Function func(daig::voidType(),"SAFETY",fnParams,fnTemps,fnBody);
    cprog.addFunction(func);
    return;
  }

  //create parameters
  BOOST_FOREACH(Variables::value_type &v,fit->second.params)
    fnParams.push_back(v.second);

  //create temporary variables
  BOOST_FOREACH(Variables::value_type &v,fit->second.temps)
    fnTemps.push_back(v.second);

  //transform the body of safety
  BOOST_FOREACH(const Stmt &st,fit->second.body) {
    syncseq::GlobalTransformer gt(*this,builder.program,nodeNum);
    gt.visit(st);
    fnBody.push_back(gt.stmtMap[st]);
  }

  Function func(daig::voidType(),"SAFETY",fnParams,fnTemps,fnBody);
  cprog.addFunction(func);
}

/*********************************************************************/
//create the functions for nodes
/*********************************************************************/
void daig::SyncSeq::createNodeFuncs()
{
  Node &node = builder.program.nodes.begin()->second;
  for(size_t i = 0;i < nodeNum;++i) {
    BOOST_FOREACH(Functions::value_type &f,node.funcs) {
      daig::VarList fnParams,fnTemps;

      //create parameters
      BOOST_FOREACH(Variables::value_type &v,f.second.params)
        fnParams.push_back(v.second);

      //create temporary variables
      BOOST_FOREACH(Variables::value_type &v,f.second.temps)
        fnTemps.push_back(v.second);

      StmtList fnBody;

      BOOST_FOREACH(const Stmt &st,f.second.body) {
        syncseq::NodeTransformer nt(*this,builder.program,nodeNum,i);

        std::string nodeId = *node.args.begin();
        nt.addIdMap(nodeId,i);
        nt.visit(st);
        nt.delIdMap(nodeId);
        fnBody.push_back(nt.stmtMap[st]);
      }

      std::string fnName = f.second.name + "_" + boost::lexical_cast<std::string>(i);
      Function func(daig::voidType(),fnName,fnParams,fnTemps,fnBody);
      cprog.addFunction(func);
    }
  }
}

/*********************************************************************/
//create a nondet function for the type of the argument
//expression. add its declaration to the C program if a new function
//was created
/*********************************************************************/
daig::Expr daig::SyncSeq::createNondetFunc(const Expr &expr)
{
  const LvalExpr *lve = dynamic_cast<LvalExpr*>(expr.get());
  assert(lve && "ERROR: can only create nondet function for LvalExpr");
  return Expr(new LvalExpr("nondet_" + lve->var));
}

/*********************************************************************/
//run the sequentialization, generating a C program
/*********************************************************************/
void daig::SyncSeq::run()
{
  std::cout << "Sequentializing with " << nodeNum << " nodes ...\n";

  //copy over external function declarations
  cprog.externalFuncs = builder.program.externalFuncs;

  createGlobVars();
  createRoundCopier();
  createMainFunc();
  createInit();
  createSafety();
  createNodeFuncs();

  //instantiate functions
}

/*********************************************************************/
//end of file
/*********************************************************************/
