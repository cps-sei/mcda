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

#include <assert.h>
#include <boost/foreach.hpp>
#include "Type.h"
#include "Variable.h"
#include "Expression.h"
#include "Statement.h"
#include "daslc/daig-parser.hpp"

/*********************************************************************/
//convert an operator to string
/*********************************************************************/
std::string daig::CompExpr::opToString() const
{
  switch(op) {
  case TNODENUM : return "#N";
  case TMINUS : return "-";
  case TPLUS : return "+";
  case TCEQ : return "==";
  case TCNE : return "!=";
  case TCLT : return "<";
  case TCLE : return "<=";
  case TCGT : return ">";
  case TCGE : return ">=";
  case TMUL : return "*";
  case TDIV : return "/";
  case TMOD : return "%";
  case TLAND : return "&&";
  case TLOR : return "||";
  case TLNOT : return "!";
  case TBWNOT : return "~";
  case TBWAND : return "&";
  case TBWOR : return "|";
  case TBWXOR : return "^";
  case TBWLSH : return "<<";
  case TBWRSH : return ">>";
  default: assert(0 && "ERROR : unknown operator!!");
  }
  return "";
}

/*********************************************************************/
//convert complex expression to string
/*********************************************************************/
std::string daig::CompExpr::toString() const
{
  if(args.size() == 0) return opToString();

  if(args.size() == 1) {
    ExprList::const_iterator it = args.begin();
    return opToString() + "(" + (*it)->toString() + ")";
  }

  if(args.size() == 2) {
    ExprList::const_iterator it = args.begin();
    std::string res = "(" + (*it)->toString() + ")";
    for(++it;it != args.end();++it)
      res += opToString() + "(" + (*it)->toString() + ")";
    return res;
  }

  if(args.size() == 3 && op == TQUEST) {
    ExprList::const_iterator it = args.begin();
    std::string res = "(" + (*it)->toString() + ")";
    ++it; res += std::string("?") + "(" + (*it)->toString() + ")";
    ++it; res += std::string(":") + "(" + (*it)->toString() + ")";
    return res;
  }

  assert(0 && "ERROR: unknown COMPLEX expression!!");

  return "";
}

/*********************************************************************/
//end of file
/*********************************************************************/
