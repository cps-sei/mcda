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
#include <iostream>
#include <stdio.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "Variable.h"

/*********************************************************************/
//constructors
/*********************************************************************/
//constructor with name only -- assigns a default type and scope
daig::Variable::Variable(const std::string &n) 
  : name(n),type(daig::Type(new BaseType())),scope(0) {}

//constructor with name and type -- assigns a default scope
daig::Variable::Variable(const std::string &n,const daig::Type &t) 
  : name(n),type(t),scope(0) {}

//constructor with name and dimensions -- assigns a default base type
//and scope
daig::Variable::Variable(const std::string &n,const std::list<int> &d) 
  : name(n),type(daig::Type(new BaseType(d))),scope(0) {}

/*********************************************************************/
///convert to string
/*********************************************************************/
std::string daig::Variable::toString() const
{
  std::string res = type->toString() + " " + name;
  BOOST_FOREACH(int d,type->dims) {
    res += "[" + ((d == -1) ? "#N" : boost::lexical_cast<std::string>(d)) + "]";
  }
  return res;
}

/*********************************************************************/
///print with indentation
/*********************************************************************/
void
daig::Variable::print (std::ostream &os,unsigned int indent)
{
  std::string spacer(indent, ' ');
  os << spacer << toString();
}

/*********************************************************************/
///return a copy but instantiate dimension #N with nodeNum
/*********************************************************************/
daig::Variable daig::Variable::instDim(size_t nodeNum) const
{
  Variable res = *this;
  res.type = res.type->instDim(nodeNum);
  return res;
}

/*********************************************************************/
///return a copy but change name to name+ext
/*********************************************************************/
daig::Variable daig::Variable::instName(std::string ext) const
{
  Variable res = *this;
  res.name = res.name + ext;
  return res;
}

/*********************************************************************/
///return a copy with one less dimension
/*********************************************************************/
daig::Variable daig::Variable::decrDim() const
{
  Variable res = *this;
  res.type = res.type->decrDim();
  return res;  
}

/*********************************************************************/
//end of file
/*********************************************************************/

