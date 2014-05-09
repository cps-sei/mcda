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
#include <assert.h>
#include <boost/foreach.hpp>
#include "Type.h"
#include "Variable.h"
#include "Expression.h"
#include "Statement.h"
#include "daslc/daig-parser.hpp"

/*********************************************************************/
//return basic types
/*********************************************************************/
daig::Type daig::voidType() { return Type(new daig::BaseType(TVOID)); }
daig::Type daig::intType() { return Type(new daig::BaseType(TINT)); }
daig::Type daig::charType() { return Type(new daig::BaseType(TCHAR)); }
daig::Type daig::ucharType() { return Type(new daig::BaseType(TUNSIGNED,TCHAR)); }
daig::Type daig::boolType() { return Type(new daig::BaseType(TBOOL)); }

/*********************************************************************/
///convert base type to string
/*********************************************************************/
std::string daig::BaseType::toString() const
{
  std::string res;

  if(qual == TSIGNED) res = "signed ";
  else if(qual == TUNSIGNED) res = "unsigned ";
  else assert(qual == 0 && "ERROR : illegal type qualifier");

  if(type == TVOID) res += "void";
  else if(type == TBOOL) res += "_Bool";
  else if(type == TINT) res += "int";
  else if(type == TCHAR) res += "char";
  else assert(0 && "ERROR : illegal simple type");

  return res;
}

/*********************************************************************/
///print the type with appropriate indentation
/*********************************************************************/
void daig::BaseType::print (std::ostream &os,unsigned int indent)
{
  os << std::string(' ',indent) << toString();
}

/*********************************************************************/
///return a copy but instantiate dimension #N with nodeNum
/*********************************************************************/
daig::Type daig::BaseType::instDim(size_t nodeNum) const
{
  BaseType *res = new BaseType(*this);

  BOOST_FOREACH(int &i,res->dims)
    if(i == -1) i = static_cast<int>(nodeNum);

  return Type(res);
}

///return a copy with one less dimension
daig::Type daig::BaseType::decrDim() const
{
  assert(!dims.empty() && "ERROR: cannot decrease dimensions of non-array type!");
  BaseType *res = new BaseType(*this);
  res->dims.pop_front();
  return Type(res);
}

///return the element type if this is an array type. if not return
///this type.
daig::Type daig::BaseType::getElemType() const
{
  BaseType *res = new BaseType(*this);
  res->dims.clear();
  return Type(res);
}

///return the first dimension
int daig::BaseType::getFirstDim() const
{
  assert(!dims.empty() && "ERROR: cannot return first dimension of non-array type!");
  return *(dims.begin());
}

/*********************************************************************/
//end of file
/*********************************************************************/
