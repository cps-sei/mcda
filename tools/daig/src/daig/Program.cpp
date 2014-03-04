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
 * 3. The names �Carnegie Mellon University,� "SEI� and/or �Software
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
 *      INSTITUTE MATERIAL IS FURNISHED ON AN �AS-IS� BASIS. CARNEGIE MELLON
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
#include <boost/foreach.hpp>
#include "Node.h"
#include "Program.h"

void
daig::Program::print (std::ostream &os,unsigned int indent)
{
  std::string spacer (indent, ' ');

  //print MOC
  os << spacer << moc.to_string_type () << ";\n\n";

  //print CONST definitions
  BOOST_FOREACH(ConstDef::value_type &cd,constDef)
    os << spacer << "CONST " << cd.first << " = " << cd.second << ";\n";
  os << '\n';

  //print external function declarations
  BOOST_FOREACH(daig::Functions::value_type &v, externalFuncs) {
    os << spacer << "EXTERN";
    v.second.printDecl(os, 1);
  }
  os << '\n';

  //print nodes
  for (daig::Nodes::iterator i = nodes.begin (); i != nodes.end (); ++i)
    i->second.print (os,indent);

  //print program definition
  os << spacer << "PROGRAM = ";
  size_t count = 0;
  BOOST_FOREACH(const Process &p,processes) {
    if(count) os << " || ";
    os << p.getNode() << '(' << p.getId() << ")";
    ++count;
  }
  os << ";\n\n";

  //print functions like INIT and safety
  for (daig::Functions::iterator i = funcs.begin (); i != funcs.end (); ++i)
    i->second.print (os,indent);
}
