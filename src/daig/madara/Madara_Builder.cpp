
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

#include "Madara_Builder.hpp"
#include "Function_Visitor.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include "daslc/daig-parser.hpp"


void
daig::madara::Madara_Builder::build ()
{
  build_header_includes ();
  build_target_thunk_includes ();
  // open daig namespace after including ALL libraries
  open_daig_namespace ();
  build_common_global_variables ();
  build_program_variables ();
  build_common_filters ();
  build_pre_exit ();
  // build target thunk WITHOUT includes
  build_target_thunk ();
  build_parse_args ();
  build_functions_declarations ();
  build_functions ();
  // close daig namespace
  close_daig_namespace ();
  buffer_ << "using namespace daig;\n";
  build_main_function ();
}

void
daig::madara::Madara_Builder::build_header_includes ()
{
  buffer_ << "#include <string>\n";
  buffer_ << "#include <vector>\n";
  buffer_ << "#include <sstream>\n";
  buffer_ << "#include <assert.h>\n";
  buffer_ << "\n";
  if (do_vrep_) buffer_ << "#include \"vrep/DaslVrep.hpp\"\n";
  buffer_ << "#include \"madara/knowledge_engine/Knowledge_Base.h\"\n";
  buffer_ << "#include \"madara/knowledge_engine/containers/Integer_Vector.h\"\n";
  buffer_ << "#include \"madara/knowledge_engine/containers/Double_Vector.h\"\n";
  buffer_ << "#include \"madara/knowledge_engine/containers/Vector_N.h\"\n";
  buffer_ << "#include \"madara/knowledge_engine/containers/Integer.h\"\n";
  buffer_ << "#include \"madara/knowledge_engine/containers/Double.h\"\n";
  buffer_ << "#include \"madara/knowledge_engine/containers/String.h\"\n";
  buffer_ << "\n";
}

void
daig::madara::Madara_Builder::build_target_thunk_includes ()
{
  const std::string include_lines = remove_include_lines_from_target_thunk ();
  buffer_ << include_lines << '\n';
}

std::string
daig::madara::Madara_Builder::remove_include_lines_from_target_thunk (void)
{
  Program::TargetType::const_iterator it =
    builder_.program.targets.find (target_);

  if (it != builder_.program.targets.end ())
  {
    std::pair<std::string, std::string> blocks = split_include_and_non_include_blocks (it->second);
    builder_.program.targets[target_] = blocks.second;

    return blocks.first;
  }

  return "";
}

std::pair<std::string, std::string>
daig::madara::Madara_Builder::split_include_and_non_include_blocks (const std::string target_str)
{
  std::vector<std::string> all_lines;
  std::vector<std::string> include_lines;
  std::vector<std::string> non_include_lines;

  boost::split (all_lines, target_str, boost::is_any_of ("\n"));
  for (std::vector<std::string>::iterator it = all_lines.begin (); it != all_lines.end (); ++it)
  {
    std::string line = *it;
    if (line.find ("#include") == 0)
    {
      include_lines.push_back (line);
    }
    else
    {
      non_include_lines.push_back (line);
    }
  }

  // all lines starting with #include
  std::string include_str ("");
  for (std::vector<std::string>::iterator it = include_lines.begin (); it != include_lines.end (); ++it)
  {
    include_str += *it + "\n";
  }

  // all lines not starting with #include
  std::string non_include_str ("");
  for (std::vector<std::string>::iterator it = non_include_lines.begin (); it != non_include_lines.end (); ++it)
  {
    non_include_str += *it + "\n";
  }

  std::pair<std::string, std::string> blocks (include_str, non_include_str);
  return blocks;
}

void
daig::madara::Madara_Builder::build_common_global_variables ()
{
  buffer_ << "// typedefs\n";
  buffer_ << "typedef   Madara::Knowledge_Record::Integer   Integer;\n\n";
  buffer_ << "// namespace shortcuts\n";
  buffer_ << "namespace engine = Madara::Knowledge_Engine;\n";
  buffer_ << "namespace containers = engine::Containers;\n\n";
  buffer_ << "// default transport variables\n";
  buffer_ << "std::string host (\"\");\n";
  buffer_ << "const std::string default_multicast (\"239.255.0.1:4150\");\n";
  buffer_ << "Madara::Transport::QoS_Transport_Settings settings;\n";
  buffer_ << "// Application-specific logger\n";
  buffer_ << "std::ofstream logger;\n";
  buffer_ << "\n";

  if (do_vrep_)
  {
    buffer_ << "//vrep IP address and port\n";
    buffer_ << "std::string vrep_host (\"\");\n";
    buffer_ << "int vrep_port (-1);\n";
    buffer_ << "//vrep model: 1 = quad, 2 = ant\n";
    buffer_ << "int vrep_model (1);\n";
    buffer_ << '\n';
    buffer_ << "//the DaslVrep interface object\n";
    buffer_ << "DaslVrep *vrep_interface = NULL;\n";
    buffer_ << '\n';
    buffer_ << "//the node handle for vrep\n";
    buffer_ << "simxInt vrep_node_id = -1;\n";
    buffer_ << '\n';
    buffer_ << "//the VREP MOVE_TO () function\n";
    buffer_ << "int VREP_MOVE_TO (unsigned char x,unsigned char y)\n";
    buffer_ << "{\n";
    buffer_ << "  return vrep_interface->moveNodeTo (vrep_node_id,x,y,1);\n";
    buffer_ << "}\n";
    buffer_ << '\n';
  }

  buffer_ << "// Containers for commonly used variables\n";
  buffer_ << "containers::Integer id;\n";
  buffer_ << "containers::Integer num_processes;\n";
  buffer_ << "containers::Integer round_count;\n";
  buffer_ << "engine::Knowledge_Update_Settings private_update (true);\n";
  buffer_ << "\n";
  buffer_ << "// number of participating processes\n";
  buffer_ << "Integer processes (";
  buffer_ << builder_.program.processes.size ();
  buffer_ << ");\n\n";

  //-- only generate this code if heartbeats are used
  if (builder_.program.sendHeartbeats) {
    buffer_ << "// Whether the current broadcast sends global updates\n";
    buffer_ << "bool send_global_updates (true);\n\n";
  }
}

void
daig::madara::Madara_Builder::build_program_variables ()
{
  // Constants
  buffer_ << "// Defining program-specific constants\n";

  Program::ConstDef & consts = builder_.program.constDef;
  for (Program::ConstDef::iterator i = consts.begin (); i != consts.end (); ++i)
  {
    buffer_ << "#define ";
    buffer_ << i->first;
    buffer_ << " ";
    buffer_ << i->second;
    buffer_ << "\n";
  }

  buffer_ << "\n";

  // Global and local variables
  Nodes & nodes = builder_.program.nodes;
  for (Nodes::iterator n = nodes.begin (); n != nodes.end (); ++n)
  {
    buffer_ << "// Defining program-specific global variables\n";
    Variables & vars = n->second.globVars;
    for (Variables::iterator i = vars.begin (); i != vars.end (); ++i)
    {
      Variable & var = i->second;
      build_program_variable (var);
    }

    buffer_ << "\n";

    buffer_ << "// Defining program-specific local variables\n";
    Variables & locals = n->second.locVars;
    for (Variables::iterator i = locals.begin (); i != locals.end (); ++i)
    {
      Variable & var = i->second;
      build_program_variable (var);
    }
  }

  buffer_ << "\n";
}

void
daig::madara::Madara_Builder::build_program_variable (const Variable & var)
{
  if (var.type->dims.size () == 1)
  {
    // 1-dimensional array

    if (var.type->type == TINT)
    {
      buffer_ << "containers::Integer_Array ";
    }
    else if (var.type->type == TDOUBLE_TYPE)
    {
      buffer_ << "containers::Double_Array ";
    }
    else
    {
      // For now, default to integer array
      buffer_ << "containers::Integer_Array ";
    }
  }
  else if (var.type->dims.size () > 1)
  {
    // multi-dimensional array

    buffer_ << "containers::Array_N ";
  }
  else
  {
    // non-array

    if (var.type->type == TINT)
    {
      buffer_ << "containers::Integer ";
    }
    else if (var.type->type == TDOUBLE_TYPE)
    {
      buffer_ << "containers::Double ";
    }
    else
    {
      // For now, default to integer
      buffer_ << "containers::Integer ";
    }
  }

  buffer_ << var.name;
  buffer_ << ";\n";

  build_program_variable_init (var);
  buffer_ << "\n";
}


void
daig::madara::Madara_Builder::build_program_variable_init (const Variable & var)
{
  if (var.type->dims.size () <= 1)
  {
    // non-array or 1-dimensional array

    if (var.type->type == TINT)
    {
      buffer_ << "Integer var_init_";
      buffer_ << var.name;
      buffer_ << " (0);\n";
    }
    else if (var.type->type == TDOUBLE_TYPE)
    {
      buffer_ << "double var_init_";
      buffer_ << var.name;
      buffer_ << " (0.0);\n";
    }
    else
    {
      // For now, default to integer
      buffer_ << "Integer var_init_";
      buffer_ << var.name;
      buffer_ << " (0);\n";
    }
  }
}

void
daig::madara::Madara_Builder::build_common_filters (void)
{
  //-- only generate this code if heartbeats are used
  if (builder_.program.sendHeartbeats)
  {
    buffer_ << "// Set heartbeat when receive global updates from other node\n";
    std::stringstream set_heartbeat;
    set_heartbeat << "  if (incoming_records[\"send_global_updates\"].is_true ())\n";
    set_heartbeat << "  {\n";
    set_heartbeat << "    // Record round of receiving global updates\n";
    set_heartbeat << "    Integer sender_id = incoming_records[\"id\"].to_integer ();\n";
    set_heartbeat << "    heartbeats.set (sender_id, *round_count);\n";
    set_heartbeat << "  }\n";
    build_common_filter ("set_heartbeat", set_heartbeat, "incoming_records");

    buffer_ << "// Add auxiliary variables to the outgoing records\n";
    std::stringstream add_auxiliaries;
    add_auxiliaries << "  // Node id\n";
    add_auxiliaries << "  outgoing_records[\"id\"] = vars.get (\".id\");\n";
    add_auxiliaries << "  // Whether the outgoing records contain global updates\n";
    add_auxiliaries << "  if (send_global_updates)\n";
    add_auxiliaries << "  {\n";
    add_auxiliaries << "    outgoing_records[\"send_global_updates\"] = Integer (1);\n";
    add_auxiliaries << "  }\n";
    add_auxiliaries << "  else\n";
    add_auxiliaries << "  {\n";
    add_auxiliaries << "    outgoing_records[\"send_global_updates\"] = Integer (0);\n";
    add_auxiliaries << "  }\n";
    build_common_filter ("add_auxiliaries", add_auxiliaries, "outgoing_records");

    buffer_ << "// Strip auxiliary variables from incoming records\n";
    std::stringstream remove_auxiliaries;
    remove_auxiliaries << "  // erase auxiliary variables before the context tries to apply it locally\n";
    remove_auxiliaries << "  incoming_records.erase (\"id\");\n";
    remove_auxiliaries << "  incoming_records.erase (\"send_global_updates\");\n";
    build_common_filter ("remove_auxiliaries", remove_auxiliaries, "incoming_records");
  }
}

void
daig::madara::Madara_Builder::build_common_filter (
    const std::string filter_name,
    std::stringstream & filter_content,
    std::string records)
{
  buffer_ << "Madara::Knowledge_Record\n";
  buffer_ << filter_name << " (Madara::Knowledge_Map & " << records << ",\n";
  buffer_ << "  const Madara::Transport::Transport_Context & context,\n";
  buffer_ << "  Madara::Knowledge_Engine::Variables & vars)\n";
  buffer_ << "{\n";
  buffer_ << "  Madara::Knowledge_Record result;\n";
  buffer_ << filter_content.str ();
  buffer_ << "  return result;\n";
  buffer_ << "}\n\n";
}

void
daig::madara::Madara_Builder::build_pre_exit ()
{
  buffer_ << "void pre_exit ()\n";
  buffer_ << "{\n";
  buffer_ << "  if (logger.is_open ())\n";
  buffer_ << "  {\n";
  buffer_ << "    logger.close ();\n";
  buffer_ << "  }\n";

  if (do_vrep_)
  {
    buffer_ << "\n  delete vrep_interface;\n";
  }

  buffer_ << "}\n\n";
}

void
daig::madara::Madara_Builder::build_target_thunk (void)
{
  buffer_ << "// target (" << target_ << ") specific thunk\n";

  // we use target_ as a key to all related thunks
  Program::TargetType::const_iterator it =
    builder_.program.targets.find (target_);

  // if there was any such target, print it
  if (it != builder_.program.targets.end ())
  {
    buffer_ << it->second << "\n\n";
  }
}

std::string
daig::madara::Madara_Builder::build_parse_args (const Variable & var)
{
  std::stringstream return_value;

  // we do not allow setting multi-dimensional vars from command line
  if (var.type->dims.size () <= 1)
  {
    buffer_ << "    else if (arg1 == \"--var_";
    buffer_ << var.name;
    buffer_ << "\")\n";
    buffer_ << "    {\n";
    buffer_ << "      if (i + 1 < argc)\n";
    buffer_ << "      {\n";
    buffer_ << "        std::stringstream buffer (argv[i + 1]);\n";
    buffer_ << "        buffer >> var_init_";
    buffer_ << var.name;
    buffer_ << ";\n";
    buffer_ << "      }\n";
    buffer_ << "      \n";
    buffer_ << "      ++i;\n";
    buffer_ << "    }\n";

    // build the help string
    return_value << "        \" [--var_";
    return_value << var.name;
    return_value << "] sets the initial value of a generated variable\\n\"\\\n";
  }

  return return_value.str ();
}

void
daig::madara::Madara_Builder::build_functions_declarations ()
{
  buffer_ << "// Forward declaring global functions\n\n";
  Functions & funcs = builder_.program.funcs;
  for (Functions::iterator i = funcs.begin (); i != funcs.end (); ++i)
  {
    build_function_declaration (Node (), i->second);
  }

  buffer_ << "\n// Forward declaring node functions\n\n";
  Nodes & nodes = builder_.program.nodes;
  for (Nodes::iterator n = nodes.begin (); n != nodes.end (); ++n)
  {
    Functions & funcs = n->second.funcs;
    for (Functions::iterator i = funcs.begin (); i != funcs.end (); ++i)
    {
      build_function_declaration (n->second, i->second);
    }
  }

  buffer_ << "\n";
}


void
daig::madara::Madara_Builder::build_function_declaration (
  const daig::Node & node, daig::Function & function)
{
  if (function.name == "INIT" || function.name == "SAFETY")
    return;

  buffer_ << "Madara::Knowledge_Record\n";
  buffer_ << node.name << "_" << function.name;
  buffer_ << " (engine::Function_Arguments & args, engine::Variables & vars);\n";
}


void
daig::madara::Madara_Builder::build_functions (void)
{
  if (builder_.is_sim)
  {
    buffer_ << "// Set \"true\" variables to the real values\n";
    buffer_ << "// of the variables they track\n";
    build_update_true_vars_function ();
  }

  buffer_ << "// Defining global functions\n\n";
  Functions & funcs = builder_.program.funcs;
  for (Functions::iterator i = funcs.begin (); i != funcs.end (); ++i)
  {
    build_function (Node (), i->second);
  }

  buffer_ << "// Defining node functions\n\n";
  Nodes & nodes = builder_.program.nodes;
  for (Nodes::iterator n = nodes.begin (); n != nodes.end (); ++n)
  {
    Functions & funcs = n->second.funcs;
    for (Functions::iterator i = funcs.begin (); i != funcs.end (); ++i)
    {
      build_function (n->second, i->second);
    }
  }

  buffer_ << "\n";
}

void
daig::madara::Madara_Builder::build_function (
  const daig::Node & node, daig::Function & function)
{
  if (function.name == "INIT" || function.name == "SAFETY")
    return;

  buffer_ << "Madara::Knowledge_Record\n";
  buffer_ << node.name << "_" << function.name;
  buffer_ << " (engine::Function_Arguments & args, engine::Variables & vars)\n";
  buffer_ << "{\n";
  buffer_ << "  // Declare local variables\n";

  buffer_ << "  Integer result (0);\n";
  BOOST_FOREACH (Variables::value_type & variable, function.temps)
  {
    Variable & var = variable.second;

    if (var.type->type == TINT)
    {
      buffer_ << "  Integer ";
    }
    else if (var.type->type == TDOUBLE_TYPE)
    {
      buffer_ << "  double ";
    }
    else if (var.type->type == TBOOL) {
      buffer_ << "  bool ";
    }
    else
    {
      // For now, default to Integer
      buffer_ << "  Integer ";
    }

    buffer_ << var.name;

    // if var is an array, declare dimension(s)
    BOOST_FOREACH (int d, var.type->dims)
    {
      buffer_ << "[" << d << "]";
    }

    buffer_ << ";\n";
  }

  buffer_ << "\n";

  buffer_ << "  // Function arguments\n";
  for (int i = 0; i < function.ordered_params.size (); i++)
  {
    Variable & param = function.ordered_params[i];

    if (param.type->type == TINT)
    {
      buffer_ << "  Integer ";
      buffer_ << param.name;
      buffer_ << " = args[" << i << "].to_integer ();\n";
    }
    else if (param.type->type == TDOUBLE_TYPE)
    {
      buffer_ << "  double ";
      buffer_ << param.name;
      buffer_ << " = args[" << i << "].to_double ();\n";
    }
    else
    {
      // For now, default to Integer
      buffer_ << "  Integer ";
      buffer_ << param.name;
      buffer_ << " = args[" << i << "].to_integer ();\n";
    }

    // For now, no support for array arguments yet
  }

  buffer_ << '\n';

  Function_Visitor visitor (function, node, builder_, buffer_, do_vrep_);

  //transform the body of safety
  BOOST_FOREACH (const Stmt & statement, function.body)
  {
    visitor.visit (statement);
  }

  buffer_ << "\n  // Insert return statement, in case user program did not\n";
  buffer_ << "  return result;\n";
  buffer_ << "}\n\n";
}

void
daig::madara::Madara_Builder::build_update_true_vars_function ()
{
  buffer_ << "Madara::Knowledge_Record\n";
  buffer_ << "UPDATE_TRUE_VARS (engine::Function_Arguments & args, engine::Variables & vars)\n";
  buffer_ << "{\n";
  buffer_ << "  Madara::Knowledge_Record result;\n";

  Nodes & nodes = builder_.program.nodes;
  for (Nodes::iterator n = nodes.begin (); n != nodes.end (); ++n)
  {
    Variables & tracked_vars = n->second.trackedGlobVars;
    for (Variables::iterator it = tracked_vars.begin (); it != tracked_vars.end (); ++it)
    {
      Variable & var = it->second;
      build_update_true_var (var);
    }
  }

  buffer_ << "  return result;\n";
  buffer_ << "}\n\n";
}

void
daig::madara::Madara_Builder::build_update_true_var (const Variable & var)
{
  if (var.type->dims.size () == 1)
  {
    // 1-dimensional array

    buffer_ << "  true_" << var.name;
    buffer_ << ".set (*id, ";
    buffer_ << var.name;
    buffer_ << "[*id]);\n";
  }
  else if (var.type->dims.size () > 1)
  {
    // n-dimensional array

    //by convention, a node owns all array elements where the last
    //index equals its id. we will create a set of nested for loops to
    //remodify all such elements
    std::vector<int> dims;
    BOOST_FOREACH(int i,var.type->dims) dims.push_back(i);

    //open for loops
    std::string index_str = "";
    for(int i = 0;i < dims.size () - 1;++i) {
      std::string spacer(2*i+2,' ');
      buffer_ << spacer << "for(Integer i" << i << " = 0;i" << i <<" < "
              << dims[i] << "; ++i" << i << ") {\n";
      index_str += "i" + boost::lexical_cast<std::string>(i) + ", ";
    }

    std::string spacer(2*dims.size(),' ');
    buffer_ << spacer << "containers::Array_N::Index index (" << dims.size() << ");\n";
    for(int i = 0;i < dims.size () - 1;++i)
      buffer_ << spacer << "index[" << i << "] = i" << i << ";\n";
    buffer_ << spacer << "index[" << dims.size() << "] = *id;\n";
    buffer_ << spacer << "true_" << var.name << ".set (index, " << var.name << "(" << index_str << "*id).to_integer ());\n";

    //close for loops
    for(int i = dims.size () - 2;i >= 0;--i) {
      std::string spacer(2*i+2,' ');
      buffer_ << spacer << "}\n";
    }
  }
  else
  {
    // non-array

    buffer_ << "  true_" << var.name;
    buffer_ << " = *";
    buffer_ << var.name;
    buffer_ << ";\n";
  }
}

void
daig::madara::Madara_Builder::build_main_define_functions ()
{
  if (builder_.is_sim)
  {
    buffer_ << "  // Defining special functions for MADARA\n\n";

    buffer_ << "  knowledge.define_function (\"UPDATE_TRUE_VARS\", UPDATE_TRUE_VARS);\n";
    buffer_ << '\n';
  }

  buffer_ << "  // Defining global functions for MADARA\n\n";
  Functions & funcs = builder_.program.funcs;
  for (Functions::iterator i = funcs.begin (); i != funcs.end (); ++i)
  {
    build_main_define_function (Node (), i->second);
  }

  buffer_ << "\n";

  buffer_ << "  // Defining node functions for MADARA\n\n";
  Nodes & nodes = builder_.program.nodes;
  for (Nodes::iterator n = nodes.begin (); n != nodes.end (); ++n)
  {
    Functions & funcs = n->second.funcs;
    for (Functions::iterator i = funcs.begin (); i != funcs.end (); ++i)
    {
      build_main_define_function (n->second, i->second);
    }
  }

  buffer_ << "\n";
}

void
daig::madara::Madara_Builder::build_main_define_function (const Node & node,
  Function & function)
{
  if (function.name != "INIT" && function.name != "SAFETY")
  {
    buffer_ << "  knowledge.define_function (\"";
    buffer_ << function.name;
    buffer_ << "\", ";
    buffer_ << node.name << "_" << function.name;
    buffer_ << ");\n";
  }
}

void
daig::madara::Madara_Builder::build_program_common_variables_bindings ()
{
  buffer_ << "  // Binding common variables\n";
  buffer_ << "  id.set_name (\".id\", knowledge);\n";
  buffer_ << "  num_processes.set_name (\".processes\", knowledge);\n";
  buffer_ << "  round_count.set_name (\".round_count\", knowledge);\n";
}

void
daig::madara::Madara_Builder::build_program_specific_variables_bindings ()
{
  Nodes & nodes = builder_.program.nodes;
  for (Nodes::iterator n = nodes.begin (); n != nodes.end (); ++n)
  {
    buffer_ << "  // Binding program-specific global variables\n";
    Variables & vars = n->second.globVars;
    for (Variables::iterator i = vars.begin (); i != vars.end (); ++i)
    {
      Variable & var = i->second;
      build_program_variable_binding (var);
    }

    buffer_ << "\n  // Binding program-specific local variables\n";
    Variables & locals = n->second.locVars;
    for (Variables::iterator i = locals.begin (); i != locals.end (); ++i)
    {
      Variable & var = i->second;
      build_program_variable_binding (var);
    }
  }
}

void
daig::madara::Madara_Builder::build_program_variable_binding (const Variable & var)
{
  buffer_ << "  ";
  buffer_ << var.name;
  buffer_ << ".set_name (\"";

  // local variables will have a period in front of them
  if (var.scope == Variable::LOCAL)
    buffer_ << ".";

  buffer_ << var.name;
  buffer_ << "\", knowledge";

  // is this an array type?
  if (var.type->dims.size () == 1)
  {
    buffer_ << ", ";
    buffer_ << builder_.program.processes.size ();
  }

  buffer_ << ");\n";

  build_program_variable_assignment (var);

  buffer_ << "\n";
}

void
daig::madara::Madara_Builder::build_program_variable_assignment (const Variable & var)
{
  // is this an array type?
  if (var.type->dims.size () == 1)
  {
    buffer_ << "  " << var.name;
    buffer_ << ".set (settings.id, var_init_";
    buffer_ << var.name;
    buffer_ << ");\n";
  }
  else if (var.type->dims.size () == 0)
  {
    buffer_ << "  " << var.name;
    buffer_ << " = ";
    buffer_ << "var_init_";
    buffer_ << var.name;
    buffer_ << ";\n";
  }
}

void
daig::madara::Madara_Builder::build_top_main_function ()
{
  // Top part of main()

  buffer_ << "int main (int argc, char ** argv)\n";
  buffer_ << "{\n";
  buffer_ << "  settings.type = Madara::Transport::MULTICAST;\n";
  buffer_ << "\n";
  buffer_ << "  // handle any command line arguments\n";
  buffer_ << "  handle_arguments (argc, argv);\n";
  buffer_ << "\n";
  buffer_ << "  if (settings.hosts.size () == 0)\n";
  buffer_ << "  {\n";
  buffer_ << "    // setup default transport as multicast\n";
  buffer_ << "    settings.hosts.push_back (default_multicast);\n";
  buffer_ << "  }\n\n";

  buffer_ << "  settings.queue_length = 100000;\n\n";

  //-- if either callbacks or heartbeats
  if (!builder_.program.callbacks.empty() || builder_.program.sendHeartbeats) {
    buffer_ << "  // add filters\n";
    buffer_ << "  settings.add_send_filter (add_auxiliaries);\n";

    if (builder_.program.callbackExists ("on_receive_filter"))
      {
        std::string usr_filter =
          builder_.program.getCallback ("on_receive_filter");
        buffer_ << "  settings.add_receive_filter (" <<
          usr_filter << ");\n";
      }

    buffer_ << "  settings.add_receive_filter (set_heartbeat);\n";
    buffer_ << "  settings.add_receive_filter (remove_auxiliaries);\n";
    buffer_ << "\n";
  }

  buffer_ << "  // create the knowledge base with the transport settings\n";
  buffer_ << "  Madara::Knowledge_Engine::Knowledge_Base knowledge (host, settings);\n\n";

  build_program_variables_bindings ();
  build_main_define_functions ();

  // Initialize common variables
  buffer_ << "  // Initialize commonly used local variables\n";
  buffer_ << "  id = Integer (settings.id);\n";
  buffer_ << "  num_processes = processes;\n";
  buffer_ << "  round_count = Integer (0);\n";
  buffer_ << '\n';

  if (builder_.program.sendHeartbeats) {
    buffer_ << "  for (Integer i = 0; i < processes; i++)\n";
    buffer_ << "  {\n";
    buffer_ << "    heartbeats.set (i, -1);\n";
    buffer_ << "  }\n";
  }
}

void
daig::madara::Madara_Builder::open_daig_namespace ()
{
  buffer_ << "// begin daig namespace\n";
  buffer_ << "namespace daig\n";
  buffer_ << "{\n";
}

void
daig::madara::Madara_Builder::close_daig_namespace ()
{
  buffer_ << "} // end daig namespace\n";
  buffer_ << "\n";
}

void
daig::madara::Madara_Builder::clear_buffer ()
{
  buffer_.str ("");
}

void
daig::madara::Madara_Builder::print (std::ostream & os)
{
  os << buffer_.str ();
}
