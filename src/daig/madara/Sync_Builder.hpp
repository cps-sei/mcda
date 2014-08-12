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


#ifndef __MADARA_SYNC_BUILDER_H__
#define __MADARA_SYNC_BUILDER_H__

#include <iostream>
#include <sstream>
#include "daig/madara/Madara_Builder.hpp"

namespace daig
{
  namespace madara
  {
    /*******************************************************************/
    // Synchronous program builder for MADARA
    /*******************************************************************/
    class Sync_Builder : public Madara_Builder
    {
    public:
      /**
       * Constructor
       * @param  builder   the source for building a program
       **/
      Sync_Builder (DaigBuilder & builder,const std::string &target,const bool do_vrep);

      /**
       * Builds the underlying character stream that can then be printed
       **/
      void build (void);
      
      /**
       * Builds the header includes
       **/
      void build_header_includes (void);

      /**
       * Builds the target thunk includes
       */
      void build_target_thunk_includes (void);

      /**
       * Builds the common global MADARA generated variables
       **/
      void build_common_global_variables (void);

      /**
       * Builds the target-specific thunk from the DASL program
       */
      void build_target_thunk (void);
      
      /**
       * Builds the program's MADARA generated variables
       **/
      void build_program_variables (void);
      
      /**
       * Builds the program's MADARA generated variables
       **/
      void build_program_variable (const Variable & var);
      
      /**
       * Builds the program's MADARA generated variables
       **/
      void build_program_variable_init (const Variable & var);

      /**
       * Initializes special variables if trackLocations and/or sendHeartbeats are set
       */
      void build_special_variables_init ();

      /**
       * Builds UPDATE_TRUE_LOCS function
       */
      void build_update_true_locs ();

      /**
       * Builds the program's MADARA generated variable bindings in main
       **/
      void build_program_variables_bindings (void);
      
      /**
       * Builds a MADARA generated variable binding in main
       **/
      void build_program_variable_binding (const Variable & var);
      
      /**
       * Builds a MADARA generated variable binding in main
       **/
      void build_program_variable_assignment (const Variable & var);

      /**
       * Builds the arguments parser
       **/
      void build_parse_args (void);
      
      /**
       * Builds variable value parsing
       * @return help printout for variable
       **/
      std::string build_parse_args (const Variable & var);

      /**
       * Builds all function declarations to prevent undefined references
       **/
      void build_functions_declarations (void);
      
      /**
       * Builds a function for refreshing modification flag on globals
       **/
      void build_refresh_modify_globals (void);
      
      /**
       * Builds a refresh statement for modification on a global
       **/
      void build_refresh_modify_global (const Variable & var);

      /**
       * Builds a function
       * @param  function  a defined function in the parsed program
       **/
      void build_function_declaration (const daig::Node & node, daig::Function & function);

      /**
       * Builds the main function
       **/
      void build_main_function (void);

      /**
       * Builds the section of main that defines MADARA callable functions
       **/
      void build_main_define_functions (void);
      
      /**
       * Builds a function definition for MADARA
       * @param  function  a defined function in the parsed program
       **/
      void build_main_define_function (const daig::Node & node,
        daig::Function & function);

      /**
       * Builds all functions
       **/
      void build_functions (void);

      /**
       * Builds a function
       * @param  function  a defined function in the parsed program
       **/
      void build_function (const daig::Node & node, daig::Function & function);
      
      /**
       * Builds commonly used filters
       */
      void build_common_filters (void);
      
      /**
       * Builds the main logic loop for execution of ROUND
       **/
      void build_main_logic (void);

      /**
       * Builds a function which will be called before node exits
       */
      void build_pre_exit (void);

      /**
       * Clears the underlying buffer
       **/
      void clear_buffer (void);

      /**
       * Prints the MADARA program to a stream
       * @param  os  the stream to print to
       **/
      void print (std::ostream & os);


    private:
      /// character buffer for holding results of build
      std::stringstream buffer_;

      /**
       * Begins daig namespace
       */
      void open_daig_namespace (void);

      /**
       * Ends daig namespace
       */
      void close_daig_namespace (void);

      /**
       * Helper function of build_common_filters
       */
      void build_common_filters_helper (const std::string filter_name,
                                        std::stringstream & filter_content);

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

#endif //__MADARA_SYNC_BUILDER_H__
