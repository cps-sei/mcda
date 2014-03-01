//a class for sequentializing DAIG into a C program

#ifndef __SYNC_SEQ_HPP__
#define __SYNC_SEQ_HPP__

#include <iostream>
#include "DaigBuilder.hpp"
#include "daig/CProgram.h"
#include "daig/Visitor.h"

namespace daig {

  //forward declaration
  class SyncSeq;

  /*******************************************************************/
  //a visitor that transforms at the global level
  /*******************************************************************/
  struct GlobalTransformer : public Visitor
  {
    ///reference to callee class for callbacks
    SyncSeq &syncSeq;

    //the DASL program being transformed
    daig::Program &prog;

    //the number of nodes
    size_t nodeNum;

    //map from variables to constants for substitution
    std::map<std::string,size_t> idMap;

    //map from expressions to the result of transforming them
    std::map<Expr,Expr> exprMap;

    //map from statements to the result of transforming them
    std::map<Stmt,Stmt> stmtMap;

    //constructors
    GlobalTransformer(SyncSeq &ss,daig::Program &p,size_t n) 
      : syncSeq(ss),prog(p),nodeNum(n) {}

    //update substitution mapping
    void addIdMap(const std::string &s,size_t i);
    void delIdMap(const std::string &s);

    //collect results
    ExprList collect(const ExprList &el);
    StmtList collect(const StmtList &sl);

    //dispatchers
    void exitInt(IntExpr &expr);
    void exitLval(LvalExpr &expr);
    void exitComp(CompExpr &expr);
    void exitCall(CallExpr &expr);
    void exitAtomic(AtomicStmt &stmt);
    void exitPrivate(PrivateStmt &stmt);
    void exitBlock(BlockStmt &stmt);
    void exitAsgn(AsgnStmt &stmt);
    void exitIT(ITStmt &stmt);
    void exitITE(ITEStmt &stmt);
    void exitFor(ForStmt &stmt);
    void exitWhile(WhileStmt &stmt);
    void exitBreak(BreakStmt &stmt);
    void exitCont(ContStmt &stmt);
    void exitRet(RetStmt &stmt);
    void exitRetVoid(RetVoidStmt &stmt);
    void exitCall(CallStmt &stmt);
    bool enterFAN(FANStmt &stmt) { return false; }
    void exitFAN(FANStmt &stmt);
    bool enterFADNP(FADNPStmt &stmt) { return false; }
    void exitFADNP(FADNPStmt &stmt);
    void exitFAO(FAOStmt &stmt);
    void exitFAOL(FAOLStmt &stmt);
    void exitFAOH(FAOHStmt &stmt);
  };

  /*******************************************************************/
  //a visitor that transforms at the node level
  /*******************************************************************/
  struct NodeTransformer : public GlobalTransformer
  {
    ///the id of the node being transformed
    size_t nodeId;

    //flags to indicate whether we are processing a function call, or
    //lhs of an assignment
    bool inCall, inLhs;

    NodeTransformer(SyncSeq &ss,Program &p,size_t n,size_t i)
      : GlobalTransformer(ss,p,n),nodeId(i),inCall(0),inLhs(0) {}

    void exitLval(LvalExpr &expr);
    bool enterCall(CallExpr &expr) { return false; }
    void exitCall(CallExpr &expr);
    bool enterEXO(EXOExpr &expr) { return false; }
    void exitEXO(EXOExpr &expr);
    bool enterEXH(EXHExpr &expr) { return false; }
    void exitEXH(EXHExpr &expr);
    bool enterEXL(EXLExpr &expr) { return false; }
    void exitEXL(EXLExpr &expr);
    bool enterAsgn(AsgnStmt &stmt) { return false; }
    void exitAsgn(AsgnStmt &stmt);
  };

  /*******************************************************************/
  //sequentializer for synchronous
  /*******************************************************************/
  class SyncSeq
  {
  public:
    DaigBuilder &builder;
    size_t nodeNum;
    int roundNum;
    CProgram cprog;

    SyncSeq(DaigBuilder &b,size_t n,int r);
    void createGlobVars();
    void createCopyStmts(bool fwd,const Variable &var,StmtList &res,ExprList indx);
    void createRoundCopier();
    void createMainFunc();
    void createInit();
    void createSafety();
    void createNodeFuncs();
    Expr createNondetFunc(const Expr &expr);
    void run();
    void printCProgram(std::ostream &os);
  };
} //namespace daig

#endif //__SYNC_SEQ_HPP__
