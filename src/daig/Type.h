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
#ifndef _DAIG_TYPE_H_
#define _DAIG_TYPE_H_

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


namespace daig
{
  //forward declaration
  class BaseType;

  //share pointer to a base type -- this is the type we will
  //mostly use
  typedef boost::shared_ptr<BaseType> Type;

  //a list of types
  typedef std::list <Type> TypeList;

  //return various basic types
  Type voidType();
  Type intType();
  Type charType();
  Type ucharType();
  Type boolType();

  /**
    * @class BaseType
    * @brief A base type
    */
  class BaseType
  {
  public:
    int qual; //qual = 0 means no qualifier
    int type; //the actual type -- void, char, int, bool etc.
    std::list<int> dims; //dimensions -- empty means non-array type

    //constructors
    BaseType() : qual(0),type(-1) {}
    BaseType(int t) : qual(0),type(t) {}
    BaseType(int q,int t) : qual(q),type(t) {}
    BaseType(const std::list<int> &d) : qual(0),type(-1),dims(d) {}

    std::string toString() const;
    void setQual(int q) { qual = q; }
    void setDims(const std::list<int> &d) { dims = d; }

    ///print the type with appropriate indentation
    void print (std::ostream &os,unsigned int indent);

    ///return a copy but instantiate dimension #N with nodeNum
    Type instDim(size_t nodeNum) const;

    ///return a copy with one less dimension
    Type decrDim() const;

    ///return the element type if this is an array type. if not return
    ///this type.
    Type getElemType() const;

    ///return true if this is an array type
    bool isArray() const { return !dims.empty(); }

    ///return the first dimension
    int getFirstDim() const;
  };
}

#endif // _DAIG_TYPE_H_
