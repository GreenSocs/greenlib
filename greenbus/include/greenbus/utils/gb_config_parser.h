// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Wolfgang Klingauf, Robert Guenzel
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
// 
//   Mark Burton, Marcus Bartholomeu
//     GreenSocs Ltd
// 
// 
//   This program is free software.
// 
//   If you have no applicable agreement with GreenSocs Ltd, this software
//   is licensed to you, and you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
// 
//   If you have a applicable agreement with GreenSocs Ltd, the terms of that
//   agreement prevail.
// 
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
// 
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
//   02110-1301  USA 
// 
// ENDLICENSETEXT

#ifndef __gb_config_parser_h__
#define __gb_config_parser_h__

#define TRACENAME "GB_CONFIG_PARSER"

namespace tlm {

enum gb_config_parse_result {
  IGNORE = 0,
  MODULE,
  PARAM,
  BUS,
  CONNECT
};

#define BUFSIZE 1024
static char line[BUFSIZE];
static char token[BUFSIZE];


typedef pair<std::vector<std::string>, gb_config_parse_result> parseresult;


/**
 * Strip heading and trailing white spaces and tabs from a std::string.
 */
//static void strip(std::string& str)
//{
//   if(str.empty()) return;
//
//   int startIndex = str.find_first_not_of(" \t");
//   int endIndex = str.find_last_not_of(" \t");
//   std::string tmp = str;
//   str.erase();
//   str = tmp.substr(startIndex, (endIndex-startIndex+ 1) );
//}


/**
 * Classify a token.
 * @param A token.
 * @return A classification of the given token.
 */
static parseresult parse(const std::string &token_) {
  std::string::size_type idx;
  std::vector<std::string> tokenlist;
  std::string token(token_);
  
  // test for empty token --> ignore
  if (token == "")
    return parseresult(tokenlist, IGNORE);

  // test for comment --> remove 
  idx = token.find("#"); 
  if (idx != std::string::npos)
    token = token.substr(0, idx);
   
  // test for tabs and spaces and create token list
  while((idx = token.find_first_of(" \t")) != std::string::npos) {
    if (idx > 0) { // is a word
      tokenlist.push_back(token.substr(0,idx));
    }    
    token = token.substr(idx+1);
  }
  
  if (token.size() > 0) {
    tokenlist.push_back(token);
  }  

  if (tokenlist.empty())
    return make_pair(tokenlist, IGNORE);

  if (tokenlist.size() == 1) 
    return make_pair(tokenlist, MODULE);
  else 
    return make_pair(tokenlist, PARAM);

}


static sc_object* getSCObject(const std::string &obj) {
  char ch[256];
  // TODO: enable the following line for SystemC-2.2
#ifdef SYSTEMC21V1
  sc_simcontext *sim;             // deprecated with SystemC-2.2
  sim = sc_get_curr_simcontext(); // deprecated with SystemC-2.2
  const std::vector<sc_object *> &objs = sim->get_child_objects();
#else
  const std::vector<sc_object *> &objs = sc_get_top_level_objects();
#endif

  // walk through module hierarchy
  const std::vector<sc_object *> *childs = &objs;
  std::string a = "", z = obj;
  std::string::size_type idx;

  while (1) {
    if ((idx = z.find(".")) != std::string::npos) { // extract parent
      a = z.substr(0,idx); // parent name without path
      z = z.substr(idx+1); // reminding path to child
    }
    else
      a = z;
    bool found = false;
    for (unsigned i=0; i<childs->size() && !found; i++) {
      sc_object *o = (*childs)[i];
      std::string oname(o->name());
      //cout << "Position: "<< oname <<", looking for " << a << endl;
      oname = oname.substr(oname.rfind(".")+1);
      if (oname == a) {
        if (idx!=std::string::npos) { // parent found
          found = true;
          sc_module *m = dynamic_cast<sc_module*>(o);
          childs = &(m->get_child_objects());
        }
        else { // obj found
          return o;
        }
      }
    }

    if (!found) {
      sprintf(ch, "Cannot find sc_object [%s] in simulation context.", obj.c_str());
      SC_REPORT_WARNING(TRACENAME, ch);
      return NULL;
    }
  }   
}


static sc_time_unit parse_time_unit(const std::string &s) {
  
  if (s == "SC_FS") return SC_FS;
  if (s == "SC_PS") return SC_PS;
  if (s == "SC_NS") return SC_NS;
  if (s == "SC_US") return SC_US;
  if (s == "SC_MS") return SC_MS;
  if (s == "SC_SEC") return SC_SEC;
  
  return static_cast<sc_time_unit>(-1);
}



/**
 * Create a bus from a std::vector of BUS command parameters.
 * @param v: std::vector{bus, clock_period, time_base}
 */
static bool gb_create_bus(busmap *m, std::vector<std::string> &v) {
  char ch[256];

  sc_time_unit tu = parse_time_unit(v[3]);
  if (tu == -1) {
    sprintf(ch, "Unknown time_unit given in BUS command!");
    SC_REPORT_WARNING(TRACENAME, ch);
    return false;
  }
  sc_time t(atoi(v[2].c_str()), tu);
  
  // find create bus function in busstd::map
  busmap::iterator pos = m->find(v[1]);
  if (pos == m->end()) {
    sprintf(ch, "Unknown bus type specified in BUS command. See router/gsrouter.h for available busses.");
    SC_REPORT_WARNING(TRACENAME, ch);
    return false;
  }

  // create the bus
  sc_module_name busname(v[0].c_str());
  pos->second(busname, t);

#ifndef GB_TRACE_DEACTIVATE_OUTPUT
  cout << "GB_CONFIG: Creating bus [" << v[0] << "] with type " << v[1] << " and clock period " << t << endl;
#endif

  return true;
}



/**
 * Bind greenbus ports from a std::vector of CONNECT command parameters.
 */
static bool gb_connect(std::vector<std::string> &v) {
  unsigned p=0;
  char ch[256];

  while (p<v.size()) {
    sc_object *iport = getSCObject(v[p++]);
    sc_object *tport = getSCObject(v[p++]);
    if (!iport || !tport) {
      sprintf(ch, "Port binding failed. One of the specified ports does not exist.");
      SC_REPORT_WARNING(TRACENAME, ch);
      return false;
    }

    if (v.size() == 2) { // point-to-point binding
      GenericMasterPort *mout = NULL;
      GenericMasterBlockingPort *bout = NULL;
      GenericRouterInitiatorPort *rout = NULL;
      GenericSlavePort *sin = NULL;
      GenericSlaveBlockingPort *bin = NULL;
      GenericRouterTargetPort *rin = NULL;

      mout = dynamic_cast<GenericMasterPort*>(iport);
      if (!mout)
        bout = dynamic_cast<GenericMasterBlockingPort*>(iport);
      if (!mout && !bout)
        rout = dynamic_cast<GenericRouterInitiatorPort*>(iport);
      sin  = dynamic_cast<GenericSlavePort*>(tport);
      if (!sin)
        bin = dynamic_cast<GenericSlaveBlockingPort*>(tport);
      if (!sin && !bin)
        rin = dynamic_cast<GenericRouterTargetPort*>(tport);
      if((mout && sin) || (bout && sin) || (bout && bin) || (mout && bin)) {      
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
        cout << "GB_CONFIG: Binding initiator to target: " << iport->name() << "(" << tport->name() << ")" << endl;
#endif
        if (mout && sin)
	        (*mout)(*sin);
        else if (bout && sin)
          (*bout)(*sin);
        else if (bout && bin)
          (*bout)(*bin);
        else if (mout && bin)
          (*mout)(*bin);
      }
      else if ((mout && rin) || (bout && rin)) {
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
        cout << "GB_CONFIG: Binding initiator to router: " << iport->name() << "(" << tport->name() << ")" << endl;
#endif
        if (mout)
	        (*mout)(*rin);
        else 
          (*bout)(*rin);
      }
      else if ((rout && sin) || (rout && bin)) {
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
        cout << "GB_CONFIG: Binding router to target: " << iport->name() << "(" << tport->name() << ")" << endl;
#endif
				if (sin)
	        (*rout)(*sin);
        else
          (*rout)(*bin);
      }
      else if (rout && rin) {
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
        cout << "GB_CONFIG: Binding router to router: " << iport->name() << "(" << tport->name() << ")" << endl;
#endif
        (*rout)(*rin);
      }        

      else {
        sprintf(ch, "Initiator-target port binding failed. One of the specified ports is not an tlm_port.");
        SC_REPORT_WARNING(TRACENAME, ch);
        return false;
      }
    }

    else { // initiator-routers-target binding  
      // TODO: enable with blocking ports (too tired tonight *gaehn*)
      if (p == 2) { // initiator(router)
        GenericMasterPort &out = *(dynamic_cast<GenericMasterPort*>(iport));
        GenericRouterTargetPort &in = *(dynamic_cast<GenericRouterTargetPort*>(tport));
        if(&in && &out) {      
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
          cout << "GB_CONFIG: Binding initiator to router: " << iport->name() << "(" << tport->name() << ")" << endl;
#endif
          out(in);
        }
        else {
          sprintf(ch, "Initiator-router port binding failed. Initiator port is not tlm_port, or target port is not tlm_multi_port.");
          SC_REPORT_WARNING(TRACENAME, ch);
          return false;
        }
      }
      else if (p==v.size()) { // router(target)
        GenericRouterInitiatorPort &out = *(dynamic_cast<GenericRouterInitiatorPort*>(iport));
        GenericSlavePort &in = *(dynamic_cast<GenericSlavePort*>(tport));
        if(&in && &out) {      
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
          cout << "GB_CONFIG: Binding router to target: " << iport->name() << "(" << tport->name() << ")" << endl;
#endif
          out(in);
        }
        else {
          sprintf(ch, "Router-target port binding failed. Router port is not tlm_multi_port, or target port is not tlm_port.");
          SC_REPORT_WARNING(TRACENAME, ch);
          return false;
        }          
      }
      else { // router(router)
        GenericRouterInitiatorPort &out = *(dynamic_cast<GenericRouterInitiatorPort*>(iport));
        GenericRouterTargetPort &in = *(dynamic_cast<GenericRouterTargetPort*>(tport));
        if(&in && &out) {      
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
          cout << "GB_CONFIG: Binding router to router: " << iport->name() << "(" << tport->name() << ")" << endl;
#endif
          out(in);
        }
        else {
          sprintf(ch, "Router-router port binding failed. One or both ports are not tlm_multi_port.");
          SC_REPORT_WARNING(TRACENAME, ch);
          return false;
        }                    
      }
    }
  }
  return true;
}



static void executeCommand(parseresult &p) {
  char ch[256];

  switch (p.second) {
  case CONNECT:
    {
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
      cout << "GB_CONFIG: Executing command CONNECT(";
      for (unsigned i=0; i<p.first.size(); i++)
        cout<<p.first[i]<<",";
      cout<<")"<<endl;
#endif
      
      if (p.first.size()%2 != 0) {
        sprintf(ch, "CONNECT command must have even number of parameters.");
        SC_REPORT_WARNING(TRACENAME, ch);
        return;
      }
      
      gb_connect(p.first);
    }
    break;
    
  case BUS:
    {
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
      cout << "GB_CONFIG: Executing command BUS(";
      for (unsigned i=0; i<p.first.size(); i++)
        cout<<p.first[i]<<",";
      cout<<")"<<endl;
#endif
      
      // create busstd::map (see router/gsrouter.h)
      static busmap *m_busmap = NULL;
      if (m_busmap == NULL)
        m_busmap = create_busmap();
      
      if (p.first.size() != 4) {
        sprintf(ch, "BUS command must have four parameters: BUS(module_name, bus_type, clock_period, time_base)");
        SC_REPORT_WARNING(TRACENAME, ch);
        return;
      }
      
      gb_create_bus(m_busmap, p.first);
    }
    break;
    
  default:
    SC_REPORT_WARNING(TRACENAME, "Command not implemented");
  }
}


#endif
