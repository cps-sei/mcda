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


#include <stdio.h>
#include <iostream>
#include "DaigBuilder.hpp"

/*********************************************************************/
//pointer to the builder to be passed to the parser and lexer
/*********************************************************************/
daig::DaigBuilder *builder = NULL;

/*********************************************************************/
//the parser routine
/*********************************************************************/
extern int yyparse();

/*********************************************************************/
//the file pointer for lex
/*********************************************************************/
extern FILE *yyin;

/*********************************************************************/
//constructor
/*********************************************************************/
daig::DaigBuilder::DaigBuilder(const std::string &fn,
                               const std::map<std::string,std::string> &constDef,
                               const bool d,
                               const bool sim)
  : fileName(fn),debug(d),is_sim(sim)
{
  program.constDef = constDef;
}

/*********************************************************************/
//run the parser
/*********************************************************************/
void daig::DaigBuilder::run()
{
  ::builder = this;
  ::yyin = fopen(fileName.c_str(),"r");
  if(!::yyin) {
    std::cerr << "ERROR: could not open file " << fileName << '\n';
    exit(1);
  }

  ::yyparse();
  fclose(::yyin);
  program.sanityCheck();
}

/*********************************************************************/
//print the program
/*********************************************************************/
void daig::DaigBuilder::printProgram(std::ostream &os)
{
  program.print(os,0);
}

/*********************************************************************/
//end of DaigBuilder.cpp
/*********************************************************************/
