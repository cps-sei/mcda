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
#include "Function_Visitor.hpp"
#include "daslc/daig-parser.hpp"
#include "boost/foreach.hpp"


daig::madara::Function_Visitor::Function_Visitor (
  Function & function, const Node & node, size_t nodeNum,
  DaigBuilder & builder, std::stringstream & buffer)
  : function_ (function), node_ (node), nodeNum_(nodeNum),
    builder_ (builder), buffer_ (buffer), indentation_ (2),
    privatize_ (false), assignment_ (0)
{

}


bool
daig::madara::Function_Visitor::enterInt (IntExpr & expression)
{
  return false;
}


void
daig::madara::Function_Visitor::exitInt (IntExpr & expression)
{
  buffer_ << "Integer (";
  buffer_ << expression.data;
  buffer_ << ")";
}


bool
daig::madara::Function_Visitor::enterLval (LvalExpr & expression)
{
  return false;
}


void
daig::madara::Function_Visitor::exitLval (LvalExpr & expression)
{
  if (expression.indices.size () > 0)
  {
    buffer_ << expression.var;

    // indexing into a multi-dimensional array requires a different operator
    if (expression.indices.size () > 1)
      buffer_ << "(";
    else
      buffer_ << "[";

    // iterate over each index
    bool started = false;
    BOOST_FOREACH (Expr & expr, expression.indices)
    {
      if (started)
        buffer_ << ", ";

      visit (expr);

      if (!started)
        started = true;
    }

    if (expression.indices.size () > 1)
      buffer_ << ")";
    else
      buffer_ << "]";


    if (function_.temps.find (expression.var) == function_.temps.end ()
        && expression.indices.size () != 1)
    {
      buffer_ << ".to_integer ()";
    }
  }
  else
  {
    // if this is substitution variable, do the substitution
    if (id_map_.find (expression.var) != id_map_.end ())
      buffer_ << id_map_[expression.var];

    // otherwise, we need to determine if this is a MADARA container or not
    else
    {
      if (function_.temps.find (expression.var) == function_.temps.end ())
      {
        buffer_ << "*";
      }
      buffer_ << expression.var;
    }
  }
}


bool
daig::madara::Function_Visitor::enterComp (CompExpr & expression)
{
  return false;
}


void
daig::madara::Function_Visitor::exitComp (CompExpr & expression)
{
  if(expression.op == TNODENUM) {
    buffer_ << nodeNum_;
    return;
  }

  ExprList::iterator arg = expression.args.begin ();

  if (expression.args.size () == 2)
    visit (*arg);

  buffer_ << " " << expression.opToString () << " ";

  ++arg;

  visit (*arg);
}


bool
daig::madara::Function_Visitor::enterCall (CallExpr & expression)
{
  return false;
}



void
daig::madara::Function_Visitor::exitCall (CallExpr & expression)
{
  buffer_ << expression.func->toString ();
  buffer_ << " (";

  bool started = false;
  BOOST_FOREACH (Expr & expr, expression.args)
  {
    if (started)
      buffer_ << ", ";

    visit (expr);

    if (!started)
      started = true;
  }

  buffer_ << ")";
}


bool
daig::madara::Function_Visitor::enterEXO (EXOExpr & expression)
{
  return false;
}


void
daig::madara::Function_Visitor::exitEXO (EXOExpr & expression)
{
  std::string spacer (indentation_, ' '), sub_spacer (indentation_ + 2, ' ');

  unsigned int processes = builder_.program.processes.size ();
  
  bool started_i = false;
  for (unsigned int i = 0; i < processes; ++i)
  {
    if (started_i)
    {
      buffer_ << " || \n" << sub_spacer;
    }

    buffer_ << "(";
    buffer_ << "*id == " << i << " && (";

    bool started_j = false;
    for (unsigned int j = 0; j < processes; ++j)
    {
      // we want to check others for existence, not ourselves
      if (i == j)
        continue; 

      if (started_j)
        buffer_ << " || ";

      id_map_ [expression.id] = j;
      
      visit (expression.arg);

      id_map_.erase (expression.id);

      if (!started_j)
        started_j = true;
    }

    buffer_ << "))";

    if (!started_i)
      started_i = true;
  }
}


bool
daig::madara::Function_Visitor::enterEXH (EXHExpr & expression)
{
  return false;
}


void
daig::madara::Function_Visitor::exitEXH (EXHExpr & expression)
{
  std::string spacer (indentation_, ' '), sub_spacer (indentation_ + 2, ' ');

  unsigned int processes = builder_.program.processes.size ();
  
  bool started_i = false;
  for (unsigned int i = 0; i + 1 < processes; ++i)
  {
    if (started_i)
    {
      buffer_ << " || \n" << sub_spacer;
    }

    buffer_ << "(";
    buffer_ << "*id == " << i << " && (";

    bool started_j = false;
    for (unsigned int j = i + 1; j < processes; ++j)
    {
      if (started_j)
        buffer_ << " || ";

      id_map_ [expression.id] = j;
      
      visit (expression.arg);

      id_map_.erase (expression.id);

      if (!started_j)
        started_j = true;
    }

    buffer_ << "))";

    if (!started_i)
      started_i = true;
  }
}


bool
daig::madara::Function_Visitor::enterEXL (EXLExpr & expression)
{
  return false;
}


void
daig::madara::Function_Visitor::exitEXL (EXLExpr & expression)
{
  std::string spacer (indentation_, ' '), sub_spacer (indentation_ + 2, ' ');

  unsigned int processes = builder_.program.processes.size ();
  
  bool started_i = false;
  for (unsigned int i = 1; i < processes; ++i)
  {
    if (started_i)
    {
      buffer_ << " || \n" << sub_spacer;
    }

    buffer_ << "(";
    buffer_ << "*id == " << i << " && (";

    bool started_j = false;
    for (unsigned int j = 0; j < i; ++j)
    {
      if (started_j)
        buffer_ << " || ";

      id_map_ [expression.id] = j;
      
      visit (expression.arg);

      id_map_.erase (expression.id);

      if (!started_j)
        started_j = true;
    }

    buffer_ << "))";

    if (!started_i)
      started_i = true;
  }
}



bool
daig::madara::Function_Visitor::enterAtomic (AtomicStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitAtomic (AtomicStmt & statement)
{
  std::string spacer (indentation_, ' ');

  // functions in MADARA are inherently atomic. Additional work is unnecessary

  visit (statement.data);
}


bool
daig::madara::Function_Visitor::enterPrivate (PrivateStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitPrivate (PrivateStmt & statement)
{
  std::string spacer (indentation_, ' ');

  buffer_ << "\n" << spacer << "// Enabling PRIVATE-only modifications\n\n";

  privatize_ = true;

  visit (statement.data);

  privatize_ = false;

  buffer_ << spacer << "// Disabling PRIVATE-only modifications\n\n";

}


bool
daig::madara::Function_Visitor::enterBlock (BlockStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitBlock (BlockStmt & statement)
{
  std::string spacer (indentation_, ' ');

  //transform the body of safety
  BOOST_FOREACH (const Stmt & i, statement.data)
  {
    visit (i);
  }
}


bool
daig::madara::Function_Visitor::enterAsgn (AsgnStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitAsgn (AsgnStmt & statement)
{
  std::string spacer (indentation_, ' '), sub_spacer (indentation_ + 2, ' ');

  // keep track of assignment;
  assignment_ = &statement;

  LvalExpr *lhs  = dynamic_cast<LvalExpr*>(statement.lhs.get ());
  
  if (lhs)
  {
    if (lhs->indices.size () == 0)
    {
      bool is_private = false;
      if (privatize_ &&
          node_.globVars.find (lhs->var) != node_.globVars.end ())
      {
        is_private = true;
        buffer_ << spacer;
        buffer_ << "// treat global variable change as a local update\n";
        buffer_ << spacer;
        buffer_ << "engine::Knowledge_Update_Settings old_update_settings (\n";
        buffer_ << sub_spacer;
        buffer_ << lhs->var;
        buffer_ << ".set_settings (private_update));\n";
      }

      buffer_ << spacer;
      buffer_ << lhs->var;
      buffer_ << " = ";
      visit (statement.rhs);

      buffer_ << ";\n";

      if (is_private)
      {
        buffer_ << spacer;
        buffer_ << "// revert global variable update settings\n";
        buffer_ << spacer;
        buffer_ << lhs->var;
        buffer_ << ".set_settings (old_update_settings);\n\n";
      }
    }
    else if (lhs->indices.size () == 1)
    {
      buffer_ << spacer;
      buffer_ << lhs->var;
      buffer_ << ".set (";
      visit (*(lhs->indices.begin ()));
      buffer_ << ", ";
      visit (statement.rhs);

      if (privatize_)
        buffer_ << ", private_update";

      buffer_ << ");\n";
    }
    else
    {
      buffer_ << spacer;
      buffer_ << "{\n";

      buffer_ << sub_spacer;
      buffer_ << "containers::Array_N::Index index (" << lhs->indices.size ();
      buffer_ << ");\n";
      
      int i = 0;
      BOOST_FOREACH (Expr & expression, lhs->indices)
      {
        buffer_ << sub_spacer << "index[";
        buffer_ << i;
        buffer_ << "] = ";
        visit (expression);
        buffer_ << ";\n";
        ++i;
      }
      
      buffer_ << sub_spacer << lhs->var;
      buffer_ << ".set (index, ";
      visit (statement.rhs);
      buffer_ << ");\n";

      if (privatize_)
        buffer_ << ", private_update";

      buffer_ << spacer << "}\n";
    }
  }

  // assignment over
  assignment_ = 0;
}


bool
daig::madara::Function_Visitor::enterIT (ITStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitIT (ITStmt & statement)
{
  daig::Expr & expression = statement.cond;

  std::string spacer (indentation_, ' ');

  buffer_ << spacer << "if (";

  visit (statement.cond);

  buffer_ << ")\n";

  indentation_ += 2;
  
  buffer_ << spacer << "{\n";
  
  visit (statement.tbranch);

  buffer_ << spacer << "}\n";
  indentation_ -= 2;
}


bool
daig::madara::Function_Visitor::enterITE (ITEStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitITE (ITEStmt & statement)
{
  std::string spacer (indentation_, ' ');

  buffer_ << spacer << "if (";

  visit (statement.cond);

  buffer_ << ")\n";

  indentation_ += 2;
  
  buffer_ << spacer << "{\n";
  
  visit (statement.tbranch);

  buffer_ << spacer << "}\n";

  buffer_ << spacer << "else\n";
  buffer_ << spacer << "{\n";
  
  visit (statement.ebranch);

  buffer_ << spacer << "}\n";

  indentation_ -= 2;
}


bool
daig::madara::Function_Visitor::enterFor (ForStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitFor (ForStmt & statement)
{
  std::string spacer (indentation_, ' ');

  buffer_ << spacer << "for (";
  
  BOOST_FOREACH (const Stmt & init, statement.init)
  {
    visit (init);
  }

  buffer_ << "; ";

  BOOST_FOREACH (const Expr & expr, statement.test)
  {
    visit (expr);
  }
  
  buffer_ << "; ";
  
  BOOST_FOREACH (const Stmt & update, statement.update)
  {
    visit (update);
  }
  
  buffer_ << ")\n";

  indentation_ += 2;
  
  buffer_ << spacer << "{\n";
  
  visit (statement.body);

  buffer_ << spacer << "}\n";
  indentation_ -= 2;
}


bool
daig::madara::Function_Visitor::enterWhile (WhileStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitWhile (WhileStmt & statement)
{
  std::string spacer (indentation_, ' ');

  buffer_ << spacer << "while (";
  
  visit (statement.cond);
  
  buffer_ << ")\n";

  indentation_ += 2;
  
  buffer_ << spacer << "{\n";
  
  visit (statement.body);

  buffer_ << spacer << "}\n";
  indentation_ -= 2;
}


bool
daig::madara::Function_Visitor::enterBreak (BreakStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitBreak (BreakStmt & statement)
{
  std::string spacer (indentation_, ' ');

  buffer_ << spacer << "break;\n";
}


bool
daig::madara::Function_Visitor::enterCont (ContStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitCont (ContStmt & statement)
{
  std::string spacer (indentation_, ' ');

  buffer_ << spacer << "continue;\n";
}


bool
daig::madara::Function_Visitor::enterRet (RetStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitRet (RetStmt & statement)
{
  std::string spacer (indentation_, ' ');

  buffer_ << spacer << "return result;\n";
}


bool
daig::madara::Function_Visitor::enterRetVoid (RetVoidStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitRetVoid (RetVoidStmt & statement)
{
  std::string spacer (indentation_, ' ');

  buffer_ << spacer << "return result;\n";
}


bool
daig::madara::Function_Visitor::enterCall (CallStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitCall (CallStmt & statement)
{
  std::string spacer (indentation_, ' '), sub_spacer (indentation_ + 2, ' ');

  CallExpr * data  = dynamic_cast<CallExpr*> (statement.data.get ());
  
  if (data)
  {
    std::string func_name = data->func->toString ();

    daig::Functions & externs = builder_.program.externalFuncs;

    if (func_name == "ASSUME" || externs.find (func_name) != externs.end ())
    {
      buffer_ << spacer;

      if (func_name == "ASSUME")
        buffer_ << "assert";
      else
        buffer_ << func_name;

      buffer_ << " (";
    
      bool started = false;
      BOOST_FOREACH (Expr & expr, data->args)
      {
        if (started)
          buffer_ << ", ";

        visit (expr);

        if (!started)
          started = true;
      }

      buffer_ << ");\n";
    }
    else
    {
      if (assignment_)
      {
        // complete the assignment and redo it later;
        buffer_ << "0;\n";
      }

      buffer_ << spacer << "{\n";
      
      buffer_ << sub_spacer;
      buffer_ << "engine::Function_Arguments args";
      if (data->args.size () > 0)
      {
        buffer_ << " (";
        buffer_ << data->args.size ();
        buffer_ << ")";
      }
      buffer_ << ";\n";

      unsigned int i = 0;
      BOOST_FOREACH (Expr & expr, data->args)
      {
        buffer_ << sub_spacer;
        buffer_ << "args [" << i << "] = ";

        visit (expr);

        buffer_ << ";\n";
        ++i;
      }

      buffer_ << "\n";
      
      if (assignment_)
      {
        buffer_ << sub_spacer;

        LvalExpr * lhs  = dynamic_cast<LvalExpr*>(assignment_->lhs.get ());
  
        if (lhs)
        {
          if (lhs->indices.size () == 0)
          {
            bool is_private = false;
            if (privatize_ &&
                node_.globVars.find (lhs->var) != node_.globVars.end ())
            {
              is_private = true;
              buffer_ << spacer;
              buffer_ << "// treat global variable change as a local update\n";
              buffer_ << spacer;
              buffer_ << "engine::Knowledge_Update_Settings old_update_settings (\n";
              buffer_ << sub_spacer;
              buffer_ << lhs->var;
              buffer_ << ".set_settings (private_update));\n";
            }

            buffer_ << spacer;
            buffer_ << lhs->var;
            buffer_ << " = ";

          }
        }
      }
      else
      {
        buffer_ << sub_spacer;
      }

      if (node_.funcs.find (func_name) != node_.funcs.end ())
      {
        buffer_ << node_.name << "_";
      }
      buffer_ << func_name << " (args, vars);\n";

      buffer_ << spacer << "}\n";
    }
  }
}


bool
daig::madara::Function_Visitor::enterFAN (FANStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitFAN (FANStmt & statement)
{
  std::string spacer (indentation_, ' '), spacer_2 (indentation_ + 2, ' '),
    spacer_3 (indentation_ + 4, ' ');

  buffer_ << spacer;
  buffer_ << "// FORALL_NODES\n";
  buffer_ << spacer;
  buffer_ << "for (";
  buffer_ << "Integer ";
  buffer_ << statement.id;
  buffer_ << " = 0; ";
  buffer_ << statement.id;
  buffer_ << " < processes; ++";
  buffer_ << statement.id;
  buffer_ << ")\n";


  indentation_ += 2;
  
  buffer_ << spacer << "{\n";
  
  bool i_added = false;

  if (function_.temps.find (statement.id) == function_.temps.end ())
  {
    i_added = true;
    function_.temps [statement.id] = Variable (statement.id);
  }
  
  visit (statement.data);

  if (i_added)
    function_.temps.erase (statement.id);

  buffer_ << spacer << "}\n\n";
  indentation_ -= 2;
}


bool
daig::madara::Function_Visitor::enterFADNP (FADNPStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitFADNP (FADNPStmt & statement)
{
  std::string spacer (indentation_, ' '), spacer_2 (indentation_ + 2, ' '),
    spacer_3 (indentation_ + 4, ' '), spacer_4 (indentation_ + 6, ' ');

  buffer_ << spacer;
  buffer_ << "// FORALL_DISTINCT_NODE_PAIRS\n";
  buffer_ << spacer;
  buffer_ << "for (";
  buffer_ << "unsigned int ";
  buffer_ << statement.id1;
  buffer_ << " = 0; ";
  buffer_ << statement.id1;
  buffer_ << " < processes; ++";
  buffer_ << statement.id1;
  buffer_ << ")\n";
  
  buffer_ << spacer << "{\n";
  
  buffer_ << spacer_2;
  buffer_ << "for (";
  buffer_ << "unsigned int ";
  buffer_ << statement.id2;
  buffer_ << " = 0; ";
  buffer_ << statement.id2;
  buffer_ << " < processes; ++";
  buffer_ << statement.id2;
  buffer_ << ")\n";
  
  buffer_ << spacer_2 << "{\n";
  
  buffer_ << spacer_3 << "if (" << statement.id1;
  buffer_ << " == ";
  buffer_ << statement.id2;
  buffer_ << ")\n";
  
  buffer_ << spacer_4 << "continue;\n\n";

  indentation_ += 4;
  
  bool id1_added = false, id2_added = false;

  if (function_.temps.find (statement.id1) == function_.temps.end ())
  {
    id1_added = true;
    function_.temps [statement.id1] = Variable (statement.id1);
  }
  
  if (function_.temps.find (statement.id2) == function_.temps.end ())
  {
    id2_added = true;
    function_.temps [statement.id2] = Variable (statement.id2);
  }
  
  visit (statement.data);

  if (id1_added)
    function_.temps.erase (statement.id1);
  
  if (id2_added)
    function_.temps.erase (statement.id2);
  
  buffer_ << spacer_2 << "}\n";
  
  buffer_ << spacer << "}\n\n";

  indentation_ -= 4;
}


bool
daig::madara::Function_Visitor::enterFAO (FAOStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitFAO (FAOStmt & statement)
{
  std::string spacer (indentation_, ' '), spacer_2 (indentation_ + 2, ' '),
    spacer_3 (indentation_ + 4, ' ');
  
  buffer_ << spacer;
  buffer_ << "// FORALL_OTHER\n";
  buffer_ << spacer;
  buffer_ << "for (";
  buffer_ << "unsigned int ";
  buffer_ << statement.id;
  buffer_ << " = 0; ";
  buffer_ << statement.id;
  buffer_ << " < processes; ++";
  buffer_ << statement.id;
  buffer_ << ")\n";
  
  indentation_ += 2;
  
  buffer_ << spacer << "{\n";
  
  bool i_added = false;

  if (function_.temps.find (statement.id) == function_.temps.end ())
  {
    i_added = true;
    function_.temps [statement.id] = Variable (statement.id);
  }
  
  buffer_ << spacer_2 << "if (" << statement.id << " == *id)\n";
  buffer_ << spacer_3 << "continue;\n\n";
  
  visit (statement.data);

  if (i_added)
    function_.temps.erase (statement.id);

  buffer_ << spacer << "}\n\n";
  indentation_ -= 2;
}


bool
daig::madara::Function_Visitor::enterFAOL (FAOLStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitFAOL (FAOLStmt & statement)
{
  std::string spacer (indentation_, ' '), spacer_2 (indentation_ + 2, ' '),
    spacer_3 (indentation_ + 4, ' ');
  
  buffer_ << spacer;
  buffer_ << "// FORALL_OTHER_LOWER\n";
  buffer_ << spacer;
  buffer_ << "for (";
  buffer_ << "unsigned int ";
  buffer_ << statement.id;
  buffer_ << " = 0; ";
  buffer_ << statement.id;
  buffer_ << " < *id; ++";
  buffer_ << statement.id;
  buffer_ << ")\n";
  
  indentation_ += 2;
  
  buffer_ << spacer << "{\n";
  
  bool i_added = false;

  if (function_.temps.find (statement.id) == function_.temps.end ())
  {
    i_added = true;
    function_.temps [statement.id] = Variable (statement.id);
  }
  
  visit (statement.data);

  if (i_added)
    function_.temps.erase (statement.id);

  buffer_ << spacer << "}\n\n";
  indentation_ -= 2;
}


bool
daig::madara::Function_Visitor::enterFAOH (FAOHStmt & statement)
{
  return false;
}


void
daig::madara::Function_Visitor::exitFAOH (FAOHStmt & statement)
{
  std::string spacer (indentation_, ' '), spacer_2 (indentation_ + 2, ' '),
    spacer_3 (indentation_ + 4, ' ');
  
  buffer_ << spacer;
  buffer_ << "// FORALL_OTHER_HIGHER\n";
  buffer_ << spacer;
  buffer_ << "for (";
  buffer_ << "unsigned int ";
  buffer_ << statement.id;
  buffer_ << " = *id + 1; ";
  buffer_ << statement.id;
  buffer_ << " < processes; ++";
  buffer_ << statement.id;
  buffer_ << ")\n";
  
  indentation_ += 2;
  
  buffer_ << spacer << "{\n";
  
  bool i_added = false;

  if (function_.temps.find (statement.id) == function_.temps.end ())
  {
    i_added = true;
    function_.temps [statement.id] = Variable (statement.id);
  }
  
  visit (statement.data);

  if (i_added)
    function_.temps.erase (statement.id);

  buffer_ << spacer << "}\n\n";
  indentation_ -= 2;
}
