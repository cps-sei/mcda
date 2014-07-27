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


#ifndef __MADARA_MADARA_BUILDER_H__
#define __MADARA_MADARA_BUILDER_H__

#include "daslc/DaigBuilder.hpp"

namespace daig
{
  namespace madara
  {
    /*******************************************************************/
    // this is the base class for various code generators for DASL
    // programs that target MADARA
    /*******************************************************************/
    class Madara_Builder
    {
    public:
      /**
       * Constructor
       * @param  builder   the source for building a program
       **/
      Madara_Builder (DaigBuilder & builder,const std::string &target,const bool do_vrep)
        : builder_ (builder), target_ (target), do_vrep_ (do_vrep) {}

      ///we need a virtual destructor
      virtual ~Madara_Builder() {}

      /**
       * Builds the underlying character stream that can then be printed
       **/
      virtual void build (void) = 0;

      /**
       * Prints the MADARA program to a stream
       * @param  os  the stream to print to
       **/
      virtual void print (std::ostream & os) = 0;


    protected:
      
      /// the result of the DASL parsing function
      DaigBuilder & builder_;

      ///the target to build against
      std::string target_;

      ///whether to generate VREP code
      bool do_vrep_;
    };
  } // namespace madara
} //namespace daig

#endif //__MADARA_MADARA_BUILDER_H__
