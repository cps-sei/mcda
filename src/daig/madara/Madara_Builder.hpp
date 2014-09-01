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
      virtual void build (void);

      /**
       * Builds the header includes
       **/
      virtual void build_header_includes (void);

      /**
       * Builds the target thunk includes
       */
      virtual void build_target_thunk_includes (void);

      /**
       * Builds the common global MADARA generated variables
       **/
      virtual void build_common_global_variables (void);

      /**
       * Builds the program's MADARA generated variables
       **/
      virtual void build_program_variables (void);

      /**
       * Builds the program's MADARA generated variables
       **/
      virtual void build_program_variable (const Variable & var);

      /**
       * Builds the program's MADARA generated variables
       **/
      virtual void build_program_variable_init (const Variable & var);

      /**
       * Builds commonly used filters
       */
      virtual void build_common_filters (void);

      /**
       * Builds filter for packet drop simulation
       */
      virtual void build_drop_filter (void);

      /**
       * Helper function of build_common_filters
       */
      virtual void build_common_filter (const std::string filter_name,
                                        std::stringstream & filter_content,
                                        bool is_receive_filter);

      /**
       * Builds a function which will be called before node exits
       */
      virtual void build_pre_exit (void);

      /**
       * Builds the target-specific thunk from the DASL program
       */
      virtual void build_target_thunk (void);

      /**
       * Builds the arguments parser
       **/
      virtual void build_parse_args (void) = 0;

      /**
       * Builds variable value parsing
       * @return help printout for variable
       **/
      virtual std::string build_parse_args (const Variable & var);

      /**
       * Builds all function declarations to prevent undefined references
       **/
      virtual void build_functions_declarations (void);

      /**
       * Builds a function
       * @param  function  a defined function in the parsed program
       **/
      virtual void build_function_declaration (const daig::Node & node, daig::Function & function);

      /**
       * Builds all functions
       **/
      virtual void build_functions (void);

      /**
       * Builds a function
       * @param  function  a defined function in the parsed program
       **/
      virtual void build_function (const daig::Node & node, daig::Function & function);

      /**
       * Builds UPDATE_TRUE_VARS function
       */
      virtual void build_update_true_vars_function ();

      /**
       * Builds true variable update
       */
      virtual void build_update_true_var (const Variable & var);

      /**
       * Builds the main function
       **/
      virtual void build_main_function (void) = 0;

      /**
       * Builds the top part (common in Sync and Async MOC) of the main function
       **/
      virtual void build_top_main_function (void);

      /**
       * Builds the program's MADARA generated variable bindings in main
       **/
      virtual void build_program_variables_bindings (void) = 0;

      virtual void build_program_common_variables_bindings (void);

      virtual void build_program_specific_variables_bindings (void);

      /**
       * Builds a MADARA generated variable binding in main
       **/
      virtual void build_program_variable_binding (const Variable & var);

      /**
       * Builds a MADARA generated variable binding in main
       **/
      virtual void build_program_variable_assignment (const Variable & var);

      /**
       * Builds the section of main that defines MADARA callable functions
       **/
      virtual void build_main_define_functions (void);

      /**
       * Builds a function definition for MADARA
       * @param  function  a defined function in the parsed program
       **/
      virtual void build_main_define_function (const daig::Node & node, daig::Function & function);

      /**
       * Begins daig namespace
       */
      virtual void open_daig_namespace (void);

      /**
       * Ends daig namespace
       */
      virtual void close_daig_namespace (void);

      /**
       * Clears the underlying buffer
       **/
      virtual void clear_buffer (void);

      /**
       * Prints the MADARA program to a stream
       * @param  os  the stream to print to
       **/
      virtual void print (std::ostream & os);


    protected:
      
      /// the result of the DASL parsing function
      DaigBuilder & builder_;

      /// the target to build against
      std::string target_;

      /// whether to generate VREP code
      bool do_vrep_;

      /// character buffer for holding results of build
      std::stringstream buffer_;

      /**
       * Removes #include lines from target thunk and returns them
       */
      std::string remove_include_lines_from_target_thunk (void);

      /**
       * Splits target_str into 2 blocks of code;
       * first block contains lines starting with #include;
       * second block contains lines not starting with #include
       */
      std::pair<std::string, std::string>
      split_include_and_non_include_blocks (const std::string target_str);

    };
  } // namespace madara
} //namespace daig

#endif //__MADARA_MADARA_BUILDER_H__
