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
 *      are those of the author (s) and do not necessarily reflect the views of
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
#include "Sync_Builder.hpp"
#include "Function_Visitor.hpp"
#include <boost/algorithm/string.hpp>
#include <vector>
#include "daslc/daig-parser.hpp"

daig::madara::Sync_Builder::Sync_Builder (daig::DaigBuilder & builder,
                                          const std::string &target,
                                          const bool do_vrep)
  : builder_ (builder), target_ (target), do_vrep_ (do_vrep)
{

}

void
daig::madara::Sync_Builder::build ()
{
  // check if we have a valid model of computation
  std::string moc = builder_.program.moc.to_string_type ();
  if (moc != "SYNC" && moc != "MOC_SYNC")
  {
    buffer_ << "ERROR: The MADARA Sync_Builder class only supports the ";
    buffer_ << "synchronous model of computation.\n";
    return;
  }

  build_header_includes ();
  build_target_thunk_includes ();
  // open daig namespace after including ALL libraries
  open_daig_namespace ();
  build_common_global_variables ();
  build_program_variables ();
  build_id_filters ();
  // build target thunk WITHOUT includes
  build_target_thunk ();
  build_external_functions ();
  build_parse_args ();
  build_functions_declarations ();
  build_functions ();
  // close daig namespace
  close_daig_namespace ();
  build_main_function ();
}

void
daig::madara::Sync_Builder::build_header_includes ()
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
daig::madara::Sync_Builder::build_target_thunk_includes ()
{
  const std::string include_lines = remove_include_lines_from_target_thunk ();
  buffer_ << include_lines << '\n';
}

void
daig::madara::Sync_Builder::build_common_global_variables ()
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
  buffer_ << "\n";

  buffer_ << "// Packet drop simulation options\n";
  buffer_ << "enum drop_policy_t { NONE, DISTANCE_DROP };\n";
  buffer_ << "drop_policy_t drop_policy = NONE;\n";
  buffer_ << "\n";

  buffer_ << "// Barrier timeout handling options\n";
  buffer_ << "enum barrier_timeout_handler_t { NOOP, IGNORE_TIMED_OUT_NODES };\n";
  buffer_ << "barrier_timeout_handler_t barrier_timeout_handler = NOOP;\n";
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
  buffer_ << "containers::Integer_Array barrier;\n";
  buffer_ << "containers::Integer id;\n";
  buffer_ << "containers::Integer num_processes;\n";
  buffer_ << "double max_barrier_time (-1);\n";
  buffer_ << "engine::Knowledge_Update_Settings private_update (true);\n";
  buffer_ << "\n";
  buffer_ << "// number of participating processes\n";
  buffer_ << "Integer processes (";
  buffer_ << builder_.program.processes.size ();
  buffer_ << ");\n\n";
}

void
daig::madara::Sync_Builder::build_target_thunk (void)
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

void daig::madara::Sync_Builder::build_id_filters (void)
{
  if (builder_.program.callbackExists ("on_receive_filter"))
  {
    buffer_ << "// Add the id to the outgoing records\n";
    buffer_ << "Madara::Knowledge_Record\n";
    buffer_ << "add_id (Madara::Knowledge_Map & outgoing_records,\n";
    buffer_ << "  const Madara::Transport::Transport_Context & context,\n";
    buffer_ << "  Madara::Knowledge_Engine::Variables & vars)\n";
    buffer_ << "{\n";
    buffer_ << "  Madara::Knowledge_Record result;\n";
    buffer_ << "  outgoing_records[\"id\"] = vars.get (\".id\");\n";
    buffer_ << "  return result;\n";
    buffer_ << "}\n\n";
  
    buffer_ << "// Strip the id from incoming records\n";
    buffer_ << "Madara::Knowledge_Record\n";
    buffer_ << "remove_id (Madara::Knowledge_Map & incoming_records,\n";
    buffer_ << "  const Madara::Transport::Transport_Context & context,\n";
    buffer_ << "  Madara::Knowledge_Engine::Variables & vars)\n";
    buffer_ << "{\n";
    buffer_ << "  Madara::Knowledge_Record result;\n";
    buffer_ << "  // erase the id before the context tries to apply it locally\n";
    buffer_ << "  incoming_records.erase (\"id\");\n";
    buffer_ << "  return result;\n";
    buffer_ << "}\n\n";
  }
}

std::string
daig::madara::Sync_Builder::remove_include_lines_from_target_thunk (void)
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
daig::madara::Sync_Builder::split_include_and_non_include_blocks (const std::string target_str)
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
daig::madara::Sync_Builder::build_program_variables ()
{
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
    
    buffer_ << "\n// Defining program-specific local variables\n";
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
daig::madara::Sync_Builder::build_program_variable_init (
  const Variable & var)
{
  if (var.type->dims.size () <= 1)
  {
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
      // Default to integer
      buffer_ << "Integer var_init_";
      buffer_ << var.name;
      buffer_ << " (0);\n";
    }
  }
}

void
daig::madara::Sync_Builder::build_program_variable (const Variable & var)
{
  // is this an array type?
  if (var.type->dims.size () == 1)
  {
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
      // Default to integer array
      buffer_ << "containers::Integer_Array ";
    }
  }
  // multi-dimensional array type?
  else if (var.type->dims.size () > 1)
  {
    buffer_ << "containers::Array_N ";
  }
  // non-array type
  else
  {
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
      // Default to integer
      buffer_ << "containers::Integer ";
    }
  }
  buffer_ << var.name;
  buffer_ << ";\n";

  build_program_variable_init (var);
  buffer_ << "\n";
}

void
daig::madara::Sync_Builder::build_program_variables_bindings ()
{ 
  buffer_ << "  // Binding common variables\n";
  buffer_ << "  barrier.set_name (\"mbarrier\", knowledge, ";
  buffer_ << builder_.program.processes.size ();
  buffer_ << ");\n";

  buffer_ << "  id.set_name (\".id\", knowledge);\n";
  buffer_ << "  num_processes.set_name (\".processes\", knowledge);\n";
  buffer_ << "\n";

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

  buffer_ << "\n";
}

void
daig::madara::Sync_Builder::build_program_variable_binding (
  const Variable & var)
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
daig::madara::Sync_Builder::build_program_variable_assignment (
  const Variable & var)
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
daig::madara::Sync_Builder::build_parse_args ()
{
  std::stringstream variable_help;

  buffer_ << "// handle arguments from the command line\n";
  buffer_ << "void handle_arguments (int argc, char ** argv)\n";
  buffer_ << "{\n";
  buffer_ << "  for (int i = 1; i < argc; ++i)\n";
  buffer_ << "  {\n";
  buffer_ << "    std::string arg1 (argv[i]);\n";
  buffer_ << "\n";
  buffer_ << "    if (arg1 == \"-m\" || arg1 == \"--multicast\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "      {\n";
  buffer_ << "        settings.hosts.push_back (argv[i + 1]);\n";
  buffer_ << "        settings.type = Madara::Transport::MULTICAST;\n";
  buffer_ << "      }\n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-b\" || arg1 == \"--broadcast\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "      {\n";
  buffer_ << "        settings.hosts.push_back (argv[i + 1]);\n";
  buffer_ << "        settings.type = Madara::Transport::BROADCAST;\n";
  buffer_ << "      }\n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-u\" || arg1 == \"--udp\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "      {\n";
  buffer_ << "        settings.hosts.push_back (argv[i + 1]);\n";
  buffer_ << "        settings.type = Madara::Transport::UDP;\n";
  buffer_ << "      }\n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-o\" || arg1 == \"--host\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "        host = argv[i + 1];\n";
  buffer_ << "        \n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-mb\" || arg1 == \"--max-barrier-time\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "      {\n";
  buffer_ << "        std::stringstream buffer (argv[i + 1]);\n";
  buffer_ << "        buffer >> max_barrier_time;\n";
  buffer_ << "      }\n";
  buffer_ << "      \n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-d\" || arg1 == \"--domain\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "        settings.domains = argv[i + 1];\n";
  buffer_ << "        \n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-i\" || arg1 == \"--id\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "      {\n";
  buffer_ << "        std::stringstream buffer (argv[i + 1]);\n";
  buffer_ << "        buffer >> settings.id;\n";
  buffer_ << "      }\n";
  buffer_ << "      \n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-l\" || arg1 == \"--level\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "      {\n";
  buffer_ << "        std::stringstream buffer (argv[i + 1]);\n";
  buffer_ << "        buffer >> MADARA_debug_level;\n";
  buffer_ << "      }\n";
  buffer_ << "      \n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-p\" || arg1 == \"--drop-rate\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "      {\n";
  buffer_ << "        double drop_rate;\n";
  buffer_ << "        std::stringstream buffer (argv[i + 1]);\n";
  buffer_ << "        buffer >> drop_rate;\n";
  buffer_ << "        \n";
  buffer_ << "        settings.update_drop_rate (drop_rate,\n";
  buffer_ << "          Madara::Transport::PACKET_DROP_DETERMINISTIC);\n";
  buffer_ << "      }\n";
  buffer_ << "      \n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"--distance-drop\")\n";
  buffer_ << "    {\n";
  buffer_ << "      drop_policy = DISTANCE_DROP;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"--ignore-timed-out-nodes\")\n";
  buffer_ << "    {\n";
  buffer_ << "      barrier_timeout_handler = IGNORE_TIMED_OUT_NODES;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-f\" || arg1 == \"--logfile\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "      {\n";
  buffer_ << "        Madara::Knowledge_Engine::Knowledge_Base::log_to_file (argv[i + 1]);\n";
  buffer_ << "      }\n";
  buffer_ << "      \n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";
  buffer_ << "    else if (arg1 == \"-r\" || arg1 == \"--reduced\")\n";
  buffer_ << "    {\n";
  buffer_ << "      settings.send_reduced_message_header = true;\n";
  buffer_ << "    }\n";


  Nodes & nodes = builder_.program.nodes;
  for (Nodes::iterator n = nodes.begin (); n != nodes.end (); ++n)
  {
    buffer_ << "    // Providing init for global variables\n";
    Variables & vars = n->second.globVars;
    for (Variables::iterator i = vars.begin (); i != vars.end (); ++i)
    {
      Variable & var = i->second;
      variable_help << build_parse_args (var);
    }
    
    buffer_ << "\n    // Providing init for local variables\n";
    Variables & locals = n->second.locVars;
    for (Variables::iterator i = locals.begin (); i != locals.end (); ++i)
    {
      Variable & var = i->second;
      variable_help << build_parse_args (var);
    }
  }

  if (do_vrep_)
  {
    buffer_ << "\n    // Providing init for VREP variables";
    buffer_ << "\n    else if (arg1 == \"-vq\" || arg1 == \"--vrep-quad\")";
    buffer_ << "\n    {";
    buffer_ << "\n       vrep_model = 1;";
    buffer_ << "\n    }";
    buffer_ << "\n    else if (arg1 == \"-va\" || arg1 == \"--vrep-ant\")";
    buffer_ << "\n    {";
    buffer_ << "\n       vrep_model = 2;";
    buffer_ << "\n    }";
    buffer_ << "\n    else if (arg1 == \"-vh\" || arg1 == \"--vrep-host\")";
    buffer_ << "\n    {";
    buffer_ << "\n      if (i + 1 < argc)";
    buffer_ << "\n      {";
    buffer_ << "\n        vrep_host = argv[i + 1];";
    buffer_ << "\n      }";
    buffer_ << "\n";
    buffer_ << "\n      ++i;";
    buffer_ << "\n    }";
    buffer_ << "\n    else if (arg1 == \"-vp\" || arg1 == \"--vrep-port\")";
    buffer_ << "\n    {";
    buffer_ << "\n      if (i + 1 < argc)";
    buffer_ << "\n      {";
    buffer_ << "\n        std::stringstream buffer (argv[i + 1]);";
    buffer_ << "\n        buffer >> vrep_port;";
    buffer_ << "\n      }";
    buffer_ << "\n";
    buffer_ << "\n      ++i;";
    buffer_ << "\n    }\n";
  }

  buffer_ << "    else\n";
  buffer_ << "    {\n";
  buffer_ << "      MADARA_DEBUG (MADARA_LOG_EMERGENCY, (LM_DEBUG, \n";
  buffer_ << "        \"\\nProgram summary for %s:\\n\\n\"\\\n";
  buffer_ << "        \" [-b|--broadcast ip:port] the broadcast ip to send and listen to\\n\"\\\n";
  buffer_ << "        \" [-d|--domain domain]     the knowledge domain to send and listen to\\n\"\\\n";
  buffer_ << "        \" [-f|--logfile file]      log to a file\\n\"\\\n";
  buffer_ << "        \" [-i|--id id]             the id of this agent (should be non-negative)\\n\"\\\n";
  buffer_ << "        \" [-l|--level level]       the logger level (0+, higher is higher detail)\\n\"\\\n";
  buffer_ << "        \" [-m|--multicast ip:port] the multicast ip to send and listen to\\n\"\\\n";
  buffer_ << "        \" [-mb|--max-barrier-time time] time in seconds to barrier for other processes\\n\"\\\n";
  buffer_ << "        \" [-o|--host hostname]     the hostname of this process (def:localhost)\\n\"\\\n";
  buffer_ << "        \" [-r|--reduced]           use the reduced message header\\n\"\\\n";
  buffer_ << "        \" [-u|--udp ip:port]       the udp ips to send to (first is self to bind to)\\n\"\\\n";

  buffer_ << variable_help.str ();

  if (do_vrep_)
  {
    buffer_ << "        \" [-vq|--vrep-quad] use Quadricopter model in VREP (default)\\n\"\\\n";
    buffer_ << "        \" [-va|--vrep-ant] use Ant model in VREP\\n\"\\\n";
    buffer_ << "        \" [-vh|--vrep-host] sets the IP address of VREP\\n\"\\\n";
    buffer_ << "        \" [-vp|--vrep-port] sets the IP port of VREP\\n\"\\\n";
  }

  buffer_ << "        , argv[0]));\n";
  buffer_ << "      exit (0);\n";
  buffer_ << "    }\n";
  buffer_ << "  }\n";
  buffer_ << "}\n\n";
}

std::string
daig::madara::Sync_Builder::build_parse_args (const Variable & var)
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
daig::madara::Sync_Builder::build_functions_declarations ()
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
daig::madara::Sync_Builder::build_refresh_modify_globals ()
{
  buffer_ << "Madara::Knowledge_Record\n";
  buffer_ << "REMODIFY_GLOBALS ";
  buffer_ << " (engine::Function_Arguments &,\n";
  buffer_ << "  engine::Variables & vars)\n";
  buffer_ << "{\n";
  
  buffer_ << "  // Remodifying common global variables\n";
  buffer_ << "  barrier.set (*id, barrier[*id]);\n\n";

  Nodes & nodes = builder_.program.nodes;
  for (Nodes::iterator n = nodes.begin (); n != nodes.end (); ++n)
  {
    buffer_ << "  // Remodifying program-specific global variables\n";
    Variables & vars = n->second.globVars;
    for (Variables::iterator i = vars.begin (); i != vars.end (); ++i)
    {
      Variable & var = i->second;
      build_refresh_modify_global (var);
    }
  }
  
  buffer_ << "  return Integer (0);\n";
  buffer_ << "}\n\n";
}

void
daig::madara::Sync_Builder::build_refresh_modify_global (const Variable & var)
{
  buffer_ << "  ";

  // is this an array type?
  if (var.type->dims.size () == 1)
  {
    buffer_ << var.name;
    buffer_ << ".set (*id, ";
    buffer_ << var.name;
    buffer_ << "[*id]);\n";
  }
  // is this an n-dimensional array type?
  else if (var.type->dims.size () > 1)
  {
    // we currently have no way of specifying owned locations in an array
    buffer_ << "\n";
  }
  else
  {
    buffer_ << var.name;
    buffer_ << " = *";
    buffer_ << var.name;
    buffer_ << ";\n";
  }
}

void
daig::madara::Sync_Builder::build_external_functions (void)
{
  //
  //buffer_ << "/**\n";
  //buffer_ << " * Defining external function stubs so this code will compile.\n\n";
  //buffer_ << " * TODO: THESE STUBS SHOULD BE REMOVED OR FILLED IN WITH ACTUAL\n";
  //buffer_ << "   IMPLEMENTATIONS.\n";
  //buffer_ << " **/\n\n";
  //Functions & funcs = builder_.program.externalFuncs;
  //for (Functions::iterator i = funcs.begin (); i != funcs.end (); ++i)
  //{
  //  build_external_function (i->second);
  //}
}

void
daig::madara::Sync_Builder::build_external_function (
  const Function & function)
{
  buffer_ << "Integer ";
  buffer_ << function.name << " ";
  buffer_ << " (";

  bool started = false;
  BOOST_FOREACH (const Variables::value_type & variable, function.params)
  {
    if (started)
      buffer_ << ", ";

    buffer_ << variable.second.type->toString ();
    buffer_ << " ";
    buffer_ << variable.second.name;

    if (!started)
      started = true;
  }

  buffer_ << ")\n";
  buffer_ << "{\n";
  buffer_ << "  // TODO: Fill this function in or remove the function\n";
  buffer_ << "  //       and add a header include for the actual function\n";


  buffer_ << "\n";
  buffer_ << "  return 1;\n";
  buffer_ << "}\n\n";
}

void
daig::madara::Sync_Builder::build_functions (void)
{
  build_refresh_modify_globals ();

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
daig::madara::Sync_Builder::build_function_declaration (
  const daig::Node & node, daig::Function & function)
{
  if (function.name == "INIT" || function.name == "SAFETY")
    return;

  buffer_ << "Madara::Knowledge_Record\n";
  buffer_ << function.name << " ";
  buffer_ << " (engine::Function_Arguments & args, engine::Variables & vars);\n";
}

void
daig::madara::Sync_Builder::build_function (
  const daig::Node & node, daig::Function & function)
{
  if (function.name == "INIT" || function.name == "SAFETY")
    return;

  buffer_ << "Madara::Knowledge_Record\n";
  buffer_ << node.name << "_" << function.name << " ";
  buffer_ << " (engine::Function_Arguments & args, engine::Variables & vars)\n";
  buffer_ << "{\n";
  buffer_ << "  // Declare local variables\n";
  
  buffer_ << "  Integer result (0);\n";
  BOOST_FOREACH (Variables::value_type & variable, function.temps)
  {
    if (variable.second.type->type == TINT)
    {
      buffer_ << "  Integer ";
      buffer_ << variable.second.name;
      buffer_ << ";\n";
    }
    else if (variable.second.type->type == TDOUBLE_TYPE)
    {
      buffer_ << "  double ";
      buffer_ << variable.second.name;
      buffer_ << ";\n";
    }
    else
    {
      // Default to integer
      buffer_ << "  Integer ";
      buffer_ << variable.second.name;
      buffer_ << ";\n";
    }
  }
  
  buffer_ << "\n";

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
daig::madara::Sync_Builder::build_main_function ()
{
  buffer_ << "int main (int argc, char ** argv)\n";
  buffer_ << "{\n";
  buffer_ << "  using namespace daig;\n";
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

  if (builder_.program.callbackExists ("on_receive_filter"))
  {
    buffer_ << "  // add user-defined receive filter\n";
    buffer_ << "  settings.add_send_filter (add_id);\n";
    std::string drop_policy_callback =
      builder_.program.getCallback ("on_receive_filter");
    buffer_ << "  settings.add_receive_filter (" <<
      drop_policy_callback << ");\n";
    
    buffer_ << "  settings.add_receive_filter (remove_id);\n\n";
  }

  buffer_ << "  Madara::Knowledge_Engine::Wait_Settings wait_settings;\n";
  buffer_ << "  wait_settings.max_wait_time = max_barrier_time;\n";
  buffer_ << "  wait_settings.poll_frequency = .1;\n\n";

  buffer_ << "  // create the knowledge base with the transport settings\n";
  buffer_ << "  Madara::Knowledge_Engine::Knowledge_Base knowledge (host, settings);\n\n";
  
  build_program_variables_bindings ();
  build_main_define_functions ();

  // TODO: allow options of barrier timout handlers

  buffer_ << "  if (barrier_timeout_handler == IGNORE_TIMED_OUT_NODES)\n";
  buffer_ << "  {\n";
  buffer_ << "    // Add barrier timeout handler\n";
  
  std::string pre_round_timeout_callback;
  std::string post_round_timeout_callback;

  if (builder_.program.callbackExists ("on_pre_round_barrier_timeout"))
  {
    pre_round_timeout_callback = builder_.program.getCallback (
      "on_pre_round_barrier_timeout");
    buffer_ << "    knowledge.define_function (\"" <<
      pre_round_timeout_callback << "\", " << pre_round_timeout_callback
      << ");\n";
  }
  if (builder_.program.callbackExists ("on_post_round_barrier_timeout"))
  {
    post_round_timeout_callback = builder_.program.getCallback (
    "on_post_round_barrier_timeout");
    buffer_ << "    knowledge.define_function (\"" <<
      post_round_timeout_callback << "\", " << post_round_timeout_callback
      << ");\n";
  }
  buffer_ << "  }\n\n";
  
  // set the values for id and processes
  buffer_ << "  id = Integer (settings.id);\n";
  buffer_ << "  num_processes = processes;\n\n";

  // build the barrier string
  
  buffer_ << "  std::map <std::string, bool>  barrier_send_list;\n";
  buffer_ << "  barrier_send_list [knowledge.expand_statement (";
  buffer_ << "\"mbarrier.{.id}\")] = true;\n\n";
  
  if (do_vrep_)
  {
    buffer_ << "  // Building the barrier string for this node. NOTE: VREP_BARRIER ADDED\n";
    buffer_ << "  std::stringstream barrier_string, barrier_sync, vrep_barrier;\n";
  }
  else
  {
    buffer_ << "  // Building the barrier string for this node\n";
    buffer_ << "  std::stringstream barrier_string, barrier_sync;\n";
  }

  buffer_ << "  bool started = false;\n\n";

  if (do_vrep_)
  {
    buffer_ << "  // Create VREP_BARRIER string\n";
    buffer_ << "  vrep_barrier << \"vrep.ready.{.id} = 1 ;> (\";\n\n";
  }

  buffer_ << "  barrier_string << \"REMODIFY_GLOBALS () ;> \";\n";
  buffer_ << "  barrier_sync << \"mbarrier.\";\n";
  buffer_ << "  barrier_sync << settings.id;\n";
  buffer_ << "  barrier_sync << \" = (mbarrier.\";\n";
  buffer_ << "  barrier_sync << settings.id;\n\n";
  buffer_ << "  // create barrier check for all lower ids\n";
  buffer_ << "  for (unsigned int i = 0; i < settings.id; ++i)\n";
  buffer_ << "  {\n";
  buffer_ << "    if (started)\n";
  buffer_ << "    {\n";
  buffer_ << "      barrier_string << \" && \";\n";
  if (do_vrep_) buffer_ << "      vrep_barrier << \" && \";\n";
  buffer_ << "    }\n\n";

  if (do_vrep_)
  {
    buffer_ << "    vrep_barrier << \"vrep.ready.\";\n";
    buffer_ << "    vrep_barrier << i;\n\n";
  }

  buffer_ << "    barrier_string << \"mbarrier.\";\n";
  buffer_ << "    barrier_string << i;\n";
  buffer_ << "    barrier_string << \" >= mbarrier.\";\n";
  buffer_ << "    barrier_string << settings.id;\n";
  buffer_ << "    barrier_sync << \" ; \";\n";
  buffer_ << "    barrier_sync << \"mbarrier.\";\n";
  buffer_ << "    barrier_sync << i;\n\n";
  buffer_ << "    if (!started)\n";
  buffer_ << "      started = true;\n";
  buffer_ << "  }\n\n";
  buffer_ << "  // create barrier check for all higher ids\n";
  buffer_ << "  for (int64_t i = settings.id + 1; i < processes; ++i)\n";
  buffer_ << "  {\n";
  buffer_ << "    if (started)\n";
  buffer_ << "    {\n";
  buffer_ << "      barrier_string << \" && \";\n";
  if (do_vrep_) buffer_ << "      vrep_barrier << \" && \";\n";
  buffer_ << "    }\n\n";

  if (do_vrep_)
  {
    buffer_ << "    vrep_barrier << \"vrep.ready.\";\n";
    buffer_ << "    vrep_barrier << i;\n\n";
  }

  buffer_ << "    barrier_string << \"mbarrier.\";\n";
  buffer_ << "    barrier_string << i;\n";
  buffer_ << "    barrier_string << \" >= mbarrier.\";\n";
  buffer_ << "    barrier_string << settings.id;\n";
  buffer_ << "    barrier_sync << \" ; \";\n";
  buffer_ << "    barrier_sync << \"mbarrier.\";\n";
  buffer_ << "    barrier_sync << i;\n\n";
  buffer_ << "    if (!started)\n";
  buffer_ << "      started = true;\n";
  buffer_ << "  }\n\n";
  buffer_ << "  barrier_sync << \")\";\n";
  if (do_vrep_) buffer_ << "  vrep_barrier << \")\";\n";
  buffer_ << '\n';

  buffer_ << "  // Compile frequently used expressions\n";
  buffer_ << "  engine::Compiled_Expression round_logic = knowledge.compile (\n";
  buffer_ << "    knowledge.expand_statement (\"ROUND (); ++mbarrier.{.id}\"));\n";
  buffer_ << "  engine::Compiled_Expression barrier_logic = \n";
  buffer_ << "    knowledge.compile (barrier_string.str ());\n";
  buffer_ << "  engine::Compiled_Expression barrier_sync_logic = \n";
  buffer_ << "    knowledge.compile (barrier_sync.str ());\n";
  if (do_vrep_)
  {
    buffer_ << "  engine::Compiled_Expression vrep_barrier_logic =\n";
    buffer_ << "    knowledge.compile (vrep_barrier.str ());\n\n";
    buffer_ << "  // SETUP VREP HERE\n";
    buffer_ << '\n';
    buffer_ << "  // create the DV object\n";
    buffer_ << "  switch (vrep_model)\n";
    buffer_ << "  {\n";
    buffer_ << "    case 2: vrep_interface = new TrackerAnt (X,Y); break;\n";
    buffer_ << "    case 1: default: vrep_interface = new QuadriRotor (X,Y);\n";
    buffer_ << "  }\n";
    buffer_ << "  vrep_interface->setDebug (true);\n";
    buffer_ << '\n';
    buffer_ << "  // connect to VREP\n";
    buffer_ << "  vrep_interface->connect ( (char*)vrep_host.c_str (), vrep_port);\n";
    buffer_ << '\n';
    buffer_ << "  // create this node\n";
    buffer_ << "  vrep_node_id = vrep_interface->createNode ();\n";
    buffer_ << '\n';
    buffer_ << "  // place this node and sleep for a second\n";
    buffer_ << "  vrep_interface->placeNodeAt (vrep_node_id, var_init_x, var_init_y, 1);\n";
    buffer_ << "  Madara::Utility::sleep (1);\n";
    buffer_ << '\n';
    buffer_ << "  wait_settings.max_wait_time = max_barrier_time;\n";
    buffer_ << '\n';
    buffer_ << "  // Barrier for all processes before running the simulation\n";
    buffer_ << "  knowledge.wait (vrep_barrier_logic, wait_settings);\n";
    buffer_ << '\n';
    buffer_ << "  if (settings.id == 0)\n";
    buffer_ << "  {\n";
    buffer_ << "    std::cout << \"Starting VREP simulator on id 0.\\n\";\n";
    buffer_ << "    vrep_interface->startSim ();\n";
    buffer_ << "  }\n";
  }
  buffer_ << '\n';

  // For now, use the first node
  Node & node = builder_.program.nodes.begin()->second;

  if (node.node_init_func_name.empty())
  {
    buffer_ << "  // Call node initialization function\n";
    buffer_ << "  knowledge.evaluate (\"" << node.node_init_func_name << " ()\");\n\n";
  }

  buffer_ << "  while (1)\n";
  buffer_ << "  {\n";
  
  buffer_ << "    knowledge.evaluate (\"++mbarrier.{.id}\", wait_settings);\n\n";

  buffer_ << "    // Call periodic functions, if any\n";
  for (std::map <std::string, int>::iterator it = node.periodic_func_names.begin ();
       it != node.periodic_func_names.end();
       ++it)
  {
    buffer_ << "    knowledge.evaluate (\"(mbarrier.{.id} % " << it->second << " == 0)";
    buffer_ << " => " << it->first << " ()\");\n";
  }
  buffer_ << '\n';

  buffer_ << "    // remodify our globals and send all updates\n";
  buffer_ << "    wait_settings.send_list.clear ();\n";
  buffer_ << '\n';
  buffer_ << "    // first barrier for new data from previous round\n";
  buffer_ << "    Integer wait_result = knowledge.wait (barrier_logic, wait_settings).to_integer ();\n";
  buffer_ << "    if (wait_result == 0 && barrier_timeout_handler != NOOP)\n";
  buffer_ << "    {\n";

  if (pre_round_timeout_callback != "")
  {
    buffer_ << "      // Execute barrier timeout handler\n";
    buffer_ << "      knowledge.evaluate (\"" << pre_round_timeout_callback << " ()\");\n";
  }

  buffer_ << "    }\n\n";

  buffer_ << "    // Send only barrier information\n";
  buffer_ << "    wait_settings.send_list = barrier_send_list;\n\n";
  
  buffer_ << "    // Synchronize barrier information for late joiners\n";
  buffer_ << "    knowledge.evaluate (barrier_sync_logic, wait_settings);\n\n";

  buffer_ << "    // Execute main user logic\n";
  buffer_ << "    knowledge.evaluate (round_logic, wait_settings);\n\n";

  buffer_ << "    // Reset heartbeats\n";
  buffer_ << "    for (Integer i = 0; i < processes; i++)\n";
  buffer_ << "    {\n";
  buffer_ << "      heartbeats.set (i, 0);\n";
  buffer_ << "    }\n\n";
  
  buffer_ << "    // second barrier for waiting on others to finish round\n";
  buffer_ << "    // Increment barrier and only send barrier update\n";
  buffer_ << "    wait_result = knowledge.wait (barrier_logic, wait_settings).to_integer ();\n";
  buffer_ << "    if (wait_result == 0 && barrier_timeout_handler != NOOP)\n";
  buffer_ << "    {\n";

  if (post_round_timeout_callback != "")
  {
    buffer_ << "      // Execute barrier timeout handler\n";
    buffer_ << "      knowledge.evaluate (\"" << post_round_timeout_callback << " ()\");\n";
  }
  
  buffer_ << "    }\n\n";
  buffer_ << "  }\n\n";
  if (do_vrep_)
  {
    buffer_ << "  delete vrep_interface;\n\n";
  }
  buffer_ << "  return 0;\n";
  buffer_ << "}\n";
}

void
daig::madara::Sync_Builder::build_main_define_functions ()
{
  buffer_ << "  // Defining common functions\n\n";

  buffer_ << "  knowledge.define_function (\"REMODIFY_GLOBALS\", ";
  buffer_ << "REMODIFY_GLOBALS);\n\n";

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
daig::madara::Sync_Builder::build_main_define_function (const Node & node,
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
daig::madara::Sync_Builder::clear_buffer ()
{
  buffer_.str ("");
}

void
daig::madara::Sync_Builder::print (std::ostream & os)
{
  os << buffer_.str ();
}

void
daig::madara::Sync_Builder::open_daig_namespace ()
{
  buffer_ << "// begin daig namespace\n";
  buffer_ << "namespace daig\n";
  buffer_ << "{\n";
}

void
daig::madara::Sync_Builder::close_daig_namespace ()
{
  buffer_ << "} // end daig namespace\n";
  buffer_ << "\n";
}
