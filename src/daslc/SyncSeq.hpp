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

#ifndef __SYNC_SEQ_HPP__
#define __SYNC_SEQ_HPP__

#include <iostream>
#include "DaigBuilder.hpp"
#include "daig/CProgram.h"
#include "CopyVisitor.hpp"

namespace daig {

  //forward declaration
  class SyncSeq;

  //new namespace to avoid name collisions
  namespace syncseq {

    /*****************************************************************/
    //a visitor that transforms at the global level
    /*****************************************************************/
    struct GlobalTransformer : public CopyVisitor
    {
      ///reference to callee class for callbacks
      SyncSeq &syncSeq;

      //the DASL program being transformed
      daig::Program &prog;

      //the number of nodes
      size_t nodeNum;

      //map from variables to constants for substitution
      std::map<std::string,size_t> idMap;

      //constructors
      GlobalTransformer(SyncSeq &ss,daig::Program &p,size_t n) 
        : syncSeq(ss),prog(p),nodeNum(n) {}

      //update substitution mapping
      void addIdMap(const std::string &s,size_t i);
      void delIdMap(const std::string &s);

      //dispatchers
      void exitLval(LvalExpr &expr);
      void exitComp(CompExpr &expr);
      void exitAtomic(AtomicStmt &stmt);
      void exitPrivate(PrivateStmt &stmt);
      void exitCall(CallStmt &stmt);
      bool enterFAN(FANStmt &stmt) { return false; }
      void exitFAN(FANStmt &stmt);
      bool enterFADNP(FADNPStmt &stmt) { return false; }
      void exitFADNP(FADNPStmt &stmt);
    };

    /*****************************************************************/
    //a visitor that transforms at the node level
    /*****************************************************************/
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
      bool enterFAO(FAOStmt &stmt) { return false; }
      void exitFAO(FAOStmt &stmt);
      bool enterFAOL(FAOLStmt &stmt) { return false; }
      void exitFAOL(FAOLStmt &stmt);
      bool enterFAOH(FAOHStmt &stmt) { return false; }
      void exitFAOH(FAOHStmt &stmt);
    };

  } //namespace syncseq

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

    SyncSeq(DaigBuilder &b,int r);
    void createGlobVars();
    void createCopyStmts(bool fwd,const Variable &var,StmtList &res,ExprList indx);
    void createRoundCopier();
    void createMainFunc();
    void createInit();
    void createSafety();
    void createNodeFuncs();
    Expr createNondetFunc(const Expr &expr);
    void run();
  };
} //namespace daig

#endif //__SYNC_SEQ_HPP__
