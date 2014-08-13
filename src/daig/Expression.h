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

#ifndef _DAIG_EXPRESSION_H_
#define _DAIG_EXPRESSION_H_

/**
 * @file Expression.h
 * @author James Edmondson <jedmondson@gmail.com>
 *
 * This file contains a class definition for an expression.
 **/

#include <stdio.h>
#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>


namespace daig
{
  //forward declaration
  class Expression;

  //share pointer to a base expression -- this is the type we will
  //mostly use
  typedef boost::shared_ptr<Expression> Expr;

  //a list of expressions
  typedef std::list <Expr> ExprList;

  /**
    * @class Expression
    * @brief An abstract base class
    */
  class Expression
  {
  public:
    virtual std::string toString() const = 0;
  };

  //an integer expression
  class IntExpr : public Expression
  {
  public:
    int data;
    IntExpr(int d) : data(d) {}
    std::string toString() const { return boost::lexical_cast<std::string>(data); }
  };

  //a double expression
  class DoubleExpr : public Expression
  {
  public:
    double data;
    // special double value, e.g., NAN
    // use string to represent special value
    std::string special_data;
    bool is_special_data;
    DoubleExpr(double d) : data(d), is_special_data(false) {}
    DoubleExpr(std::string s) : special_data(s), is_special_data(true) {}
    std::string toString() const { if (is_special_data) return special_data;
                                 else return boost::lexical_cast<std::string>(data);}
  };

  //an lvalue expression
  class LvalExpr : public Expression
  {
  public:
    //the base variable name -- this is always non-empty
    std::string var; 
    //an optional node id -- can be empty
    std::string node; 
    //a list of indices
    ExprList indices;

    LvalExpr(const std::string &v) : var(v) {}
    LvalExpr(const std::string &v,const std::string &n) : var(v), node(n) {}
    LvalExpr(const std::string &v,const ExprList &i) : var(v), indices(i) {}
    LvalExpr(const std::string &v,const std::string &n,const ExprList &i)
      : var(v), node(n), indices(i) {}
    std::string toString() const {
      std::string res = var;
      if(!node.empty()) res += "." + node;
      BOOST_FOREACH(const Expr &ep,indices) res += "[" + ep->toString() + "]";
      return res;
    }
  };

  //a complex expression
  class CompExpr : public Expression
  {
  public:
    int op;
    ExprList args;

    CompExpr(int o) : op(o) {}

    CompExpr(int o,const Expr &e) : op(o)
    { args.push_back(e); }

    CompExpr(int o,const Expr &e1,const Expr &e2) 
      : op(o)
    { args.push_back(e1); args.push_back(e2); }

    CompExpr(int o,const Expr &e1,const Expr &e2,const Expr &e3) 
      : op(o)
    { args.push_back(e1); args.push_back(e2); args.push_back(e3); }

    CompExpr(int o,const ExprList &a) : op(o),args(a) {}

    std::string opToString() const;
    std::string toString() const;
  };

  //an function call expression
  class CallExpr : public Expression
  {
  public:
    Expr func;
    ExprList args;

    CallExpr(const Expr &f,const ExprList &a) : func(f),args(a) {}
    std::string toString() const {
      std::string res = func->toString() + "(";

      size_t count = 0;
      BOOST_FOREACH(const Expr &a,args) {
        if(count) res += ",";
        res += a->toString();
        ++count;
      }

      return res + ")";
    }
  };

  //an exists any expression
  class EXAExpr : public Expression
  {
  public:
    std::string id;
    Expr arg;

    EXAExpr(const std::string &i,const Expr &a) : id(i),arg(a) {}
    std::string toString() const {
      return "EXISTS_ANY(" + id + "," + arg->toString() + ")";
    }
  };

  //an exists other expression
  class EXOExpr : public Expression
  {
  public:
    std::string id;
    Expr arg;

    EXOExpr(const std::string &i,const Expr &a) : id(i),arg(a) {}
    std::string toString() const {
      return "EXISTS_OTHER(" + id + "," + arg->toString() + ")";
    }
  };

  //an exists higher expression
  class EXHExpr : public Expression
  {
  public:
    std::string id;
    Expr arg;

    EXHExpr(const std::string &i,const Expr &a) : id(i),arg(a) {}
    std::string toString() const {
      return "EXISTS_HIGHER(" + id + "," + arg->toString() + ")";
    }
  };

  //an exists lower expression
  class EXLExpr : public Expression
  {
  public:
    std::string id;
    Expr arg;

    EXLExpr(const std::string &i,const Expr &a) : id(i),arg(a) {}
    std::string toString() const {
      return "EXISTS_LOWER(" + id + "," + arg->toString() + ")";
    }
  };

  typedef std::vector <int> Expressions;
}

#endif // _DAIG_EXPRESSION_H_
