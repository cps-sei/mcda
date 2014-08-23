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

#include "Sync_Builder.hpp"
#include "Function_Visitor.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include "daslc/daig-parser.hpp"

daig::madara::Sync_Builder::Sync_Builder (daig::DaigBuilder & builder,
                                          const std::string &target,
                                          const bool do_vrep)
  : Madara_Builder(builder,target,do_vrep) {}

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

  Madara_Builder::build ();
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
  buffer_ << "// Application-specific logger\n";
  buffer_ << "std::ofstream logger;\n";
  buffer_ << "\n";

  //-- only generate this code if heartbeats are used
  if(builder_.program.sendHeartbeats) {
    buffer_ << "// Whether the current broadcast sends global updates\n";
    buffer_ << "bool send_global_updates (false);\n\n";
  }
  
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
  buffer_ << "    else if (arg1 == \"--app-logfile\")\n";
  buffer_ << "    {\n";
  buffer_ << "      if (i + 1 < argc)\n";
  buffer_ << "      {\n";
  buffer_ << "        std::stringstream buffer (argv[i + 1]);\n";
  buffer_ << "        logger.open (buffer.str ().c_str (), std::ofstream::app);\n";
  buffer_ << "      }\n";
  buffer_ << "      \n";
  buffer_ << "      ++i;\n";
  buffer_ << "    }\n";

  Nodes & nodes = builder_.program.nodes;
  for (Nodes::iterator n = nodes.begin (); n != nodes.end (); ++n)
  {
    buffer_ << "\n    // Providing init for global variables\n";
    Variables & vars = n->second.globVars;
    for (Variables::iterator i = vars.begin (); i != vars.end (); ++i)
    {
      Variable & var = i->second;
      variable_help << Madara_Builder::build_parse_args (var);
    }
    
    buffer_ << "\n    // Providing init for local variables\n";
    Variables & locals = n->second.locVars;
    for (Variables::iterator i = locals.begin (); i != locals.end (); ++i)
    {
      Variable & var = i->second;
      variable_help << Madara_Builder::build_parse_args (var);
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

void
daig::madara::Sync_Builder::build_refresh_modify_globals ()
{
  buffer_ << "Madara::Knowledge_Record\n";
  buffer_ << "REMODIFY_GLOBALS";
  buffer_ << " (engine::Function_Arguments & args,\n";
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
  // is this an array type?
  if (var.type->dims.size () == 1)
  {
    buffer_ << "  " << var.name;
    buffer_ << ".set (*id, ";
    buffer_ << var.name;
    buffer_ << "[*id]);\n";
  }
  // is this an n-dimensional array type?
  else if (var.type->dims.size () > 1)
  {
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
    buffer_ << spacer << var.name << ".set (index, " << var.name << "(" << index_str << "*id).to_integer ());\n";

    //close for loops
    for(int i = dims.size () - 2;i >= 0;--i) {
      std::string spacer(2*i+2,' ');
      buffer_ << spacer << "}\n";
    }

    // we currently have no way of specifying owned locations in an array
    buffer_ << "\n";
  }
  else
  {
    buffer_ << "  " << var.name;
    buffer_ << " = *";
    buffer_ << var.name;
    buffer_ << ";\n";
  }
}

void
daig::madara::Sync_Builder::build_main_function ()
{
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
  if(!builder_.program.callbacks.empty() || builder_.program.sendHeartbeats) {
    buffer_ << "  // add commonly used filters\n";
    buffer_ << "  settings.add_send_filter (add_auxiliaries);\n";

    if (builder_.program.callbackExists ("on_receive_filter"))
      {
        buffer_ << "  // add user-defined receive filter\n";
        std::string usr_filter =
          builder_.program.getCallback ("on_receive_filter");
        buffer_ << "  settings.add_receive_filter (" <<
          usr_filter << ");\n";
      }

    buffer_ << "  settings.add_receive_filter (set_heartbeat);\n";
    buffer_ << "  settings.add_receive_filter (remove_auxiliaries);\n";
    buffer_ << "\n";
  }

  buffer_ << "  Madara::Knowledge_Engine::Wait_Settings wait_settings;\n";
  buffer_ << "  wait_settings.max_wait_time = max_barrier_time;\n";
  buffer_ << "  wait_settings.poll_frequency = .1;\n\n";

  buffer_ << "  // create the knowledge base with the transport settings\n";
  buffer_ << "  Madara::Knowledge_Engine::Knowledge_Base knowledge (host, settings);\n\n";
  
  build_program_variables_bindings ();
  build_main_define_functions ();
  
  // set the values for id and processes
  buffer_ << "  // Initialize commonly used local variables\n";  
  buffer_ << "  id = Integer (settings.id);\n";
  buffer_ << "  num_processes = processes;\n";
  buffer_ << '\n';

  build_special_variables_init ();

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

  if (builder_.program.sendHeartbeats)
    buffer_ << "    knowledge.expand_statement (\"++.round_count; ROUND (); UPDATE_TRUE_LOCS (); ++mbarrier.{.id}\"));\n";
  else
    buffer_ << "    knowledge.expand_statement (\"ROUND (); UPDATE_TRUE_LOCS (); ++mbarrier.{.id}\"));\n";
    

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

  // For now, use the first node definition
  Node & node = builder_.program.nodes.begin()->second;

  if (node.funcs.find ("NODE_INIT") != node.funcs.end ())
  {
    buffer_ << "  // Call node initialization function\n";
    buffer_ << "  wait_settings.delay_sending_modifieds = true;\n";
    buffer_ << "  knowledge.evaluate (\"NODE_INIT ()\", wait_settings);\n";
    buffer_ << '\n';
  }

  //-- for periodic nodes
  if(builder_.program.period) {
    std::string period = boost::lexical_cast<std::string>(builder_.program.period);
    buffer_ << "  ACE_Time_Value current = ACE_OS::gettimeofday ();\n"
            << "  ACE_Time_Value next_epoch = current;\n"
            << "  ACE_Time_Value period; period.msec(" << period << ");\n"
            << '\n';
  }

  buffer_ << "  while (1)\n";
  buffer_ << "  {\n";
  
  //-- for periodic nodes
  if(builder_.program.period) {
    std::string period = boost::lexical_cast<std::string>(builder_.program.period);
    buffer_ << "    // wait for next period\n"
            << "    current = ACE_OS::gettimeofday ();\n"
            << "    if(current < next_epoch) {\n"
            << "      Madara::Utility::sleep (next_epoch - current);\n"
            << "      next_epoch += period;\n"
            << "    } else {\n"
            << "      unsigned long current_msec = current.msec();\n"
            << "      unsigned long next_epoch_msec = next_epoch.msec();\n"
            << "      unsigned long diff_msec = current_msec - next_epoch_msec;\n"
            << "      next_epoch_msec += (diff_msec / " << period << " + 1) * " << period << ";\n"
            << "      next_epoch.msec(next_epoch_msec);\n"
            << "      Madara::Utility::sleep (next_epoch - current);\n"
            << "      next_epoch += period;\n"
            << "    }\n"
            << '\n';
  }

  buffer_ << "    // Pre-round barrier increment\n";
  buffer_ << "    wait_settings.delay_sending_modifieds = true;\n";
  buffer_ << "    knowledge.evaluate (\"++mbarrier.{.id}\", wait_settings);\n\n";

  buffer_ << "    // Call periodic functions in the order they appear in dasl program\n";
  BOOST_FOREACH (std::string func_name, node.periodic_func_names)
  {
    int period = node.periods[func_name];
    buffer_ << "    knowledge.evaluate (\"(.round_count > 0 && .round_count % " << period << " == 0)";
    buffer_ << " => " << func_name << " ()\", wait_settings);\n";
  }
  buffer_ << '\n';

  buffer_ << "    // remodify our globals and send all updates\n";
  buffer_ << "    wait_settings.send_list.clear ();\n";
  buffer_ << "    wait_settings.delay_sending_modifieds = false;\n";
  if(builder_.program.sendHeartbeats)
    buffer_ << "    send_global_updates = true;\n\n";

  buffer_ << "    // first barrier for new data from previous round\n";
  buffer_ << "    knowledge.wait (barrier_logic, wait_settings);\n\n";

  buffer_ << "    // Send only barrier information\n";
  buffer_ << "    wait_settings.send_list = barrier_send_list;\n";
  if(builder_.program.sendHeartbeats)
    buffer_ << "    send_global_updates = false;\n\n";
  
  buffer_ << "    // Execute main user logic\n";
  buffer_ << "    wait_settings.delay_sending_modifieds = true;\n";
  buffer_ << "    knowledge.evaluate (round_logic, wait_settings);\n\n";

  buffer_ << "    // second barrier for waiting on others to finish round\n";
  buffer_ << "    // Increment barrier and only send barrier update\n";
  buffer_ << "    wait_settings.delay_sending_modifieds = false;\n";
  buffer_ << "    knowledge.wait (barrier_logic, wait_settings);\n\n";

  buffer_ << "  }\n\n";

  if (do_vrep_)
  {
    buffer_ << "  delete vrep_interface;\n\n";
  }
  buffer_ << "  return 0;\n";
  buffer_ << "}\n";
}
