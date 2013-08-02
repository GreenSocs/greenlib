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

#ifndef __gb_config_h__
#define __gb_config_h__

// to deactivate output, use
// #define GB_TRACE_DEACTIVATE_OUTPUT


#include "systemc.h"

#ifndef USE_STATIC_CASTS
#include "greenbus/transport/generic.h"
#endif

#include "greenbus/utils/router_fabric.h"
#include "greenbus/utils/gb_config_parser.h"

#undef TRACENAME
#define TRACENAME "GB_CONFIG"


/**
 * Get an sc_module by its instance name.
 *
 * Example usage:
 *   getConfigurableModuleByName("JpegEncoder")->setParam("compression", "5");
 */
inline gb_configurable *getConfigurableModuleByName(std::string &name) {
  sc_object *o = getSCObject(name);
  if (o) {
    gb_configurable *m = dynamic_cast<gb_configurable*>(o);
    if (m) 
      return m;
  }
  return NULL;
}

 

/**
 * Configure module parameters and create communication architecture
 * from a config file.
 *
 * Example usage:
 *    int sc_main(int argc, char *argv[]) {
 *      jpegEncoder jpg("JpegEncoder");
 *      gb_config("jpeg_simplebus.cfg");
 *      sc_start();
 *    }
 *      
 */
inline void gb_config(const char *filename) {
  unsigned ln = 0;
  sc_module *current_module = NULL;
  char ch[256];

  ifstream is(filename);
  if (!is) {
    SC_REPORT_ERROR("GB_CONFIG", "Cannot open config file");
    return;
  }
  
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
  cout << endl << "GB_CONFIG: Reading configuration from file " << filename << endl;
#endif

  // get simulation context 
#ifdef SYSTEMC21V1
  sc_simcontext *sim; // deprecated with SystemC-2.2
  sim = sc_get_curr_simcontext(); // deprecated with SystemC-2.2
  const std::vector<sc_object *> &objs = sim->get_child_objects();
#else  
  const std::vector<sc_object *> &objs = sc_get_top_level_objects();
#endif  
  
  // TODO: HACK warning --- the following code is quick and dirty. 
  //       There should be better ways to implement the config file parser.

  while(is) {
    is.getline(line, BUFSIZE, '\n');
    ln++;
    std::string::size_type lidx;
    std::string lstr(line);
    if ((lidx = lstr.find("#")) != std::string::npos)  // remove comment
      lstr = lstr.substr(0, lidx);
    std::stringstream s(lstr);
    current_module = NULL; // reset

    // command?
    parseresult p;
    p.second = IGNORE;

    if (lstr.find("CONNECT") != std::string::npos) p.second = CONNECT;
    else if (lstr.find("BUS") != std::string::npos) p.second = BUS;
    if (p.second != IGNORE) { // command found
      if ((lidx = lstr.find("(")) == std::string::npos || lstr.find(")") == std::string::npos) {
        sprintf(ch, "In config file line %d: '(' or ')' missing in command", ln);
        SC_REPORT_WARNING(TRACENAME, ch);
      }
      else { // get parameters
        lstr = lstr.substr(lidx+1);
        while ((lidx = lstr.find_first_of(" \t,)")) != std::string::npos) {
          if (lidx>0 && lstr[0]!=' ' && lstr[0]!='\t') {
            p.first.push_back(lstr.substr(0,lidx));            
            //cout << "found command parameter " << lstr.substr(0,lidx) << endl;
          }
          lstr = lstr.substr(lidx+1);
        }
        if (p.first.size() < 2) {
          sprintf(ch, "In config file line %d: Too few arguments to command", ln);
          SC_REPORT_WARNING(TRACENAME, ch);
        }
        else 
          executeCommand(p);
      }
    }


    else {

      // param
      while(s) {
        s.getline(token, BUFSIZE, '.');
        if (token != "") {
          parseresult p = parse(token);
          
          switch(p.second) {
          case PARAM: // set param value
            if (current_module) {
#ifndef GB_TRACE_DEACTIVATE_OUTPUT
              cout << "GB_CONFIG: Configuring value of param [" << p.first[0] << "] in module [" << current_module->name() << "] to [" << p.first[1] << "]" << endl;
#endif
              gb_configurable *gsc = dynamic_cast<gb_configurable*>(current_module);
              if (gsc)
                gsc->setParam(p.first[0], p.first[1]);
              else {
                sprintf(ch, "In config file line %d: Module [%s] is not of type gb_configurable!", ln, current_module->name());
                SC_REPORT_WARNING(TRACENAME, ch);
              }
            }
            else {
              sprintf(ch, "In config file line %d: No module specified for param [%s].", ln, p.first[0].c_str());
              SC_REPORT_WARNING(TRACENAME, ch);
            }
            break;
            
          case MODULE: // step down in module hierarchy
            {
              const std::vector<sc_object *> *childs;
              if (current_module)
                childs = &(current_module->get_child_objects());
              else 
                childs = &objs;
              
              sc_object *o;
              bool found = false;
              for (unsigned int i=0; i<childs->size() && !found; i++) {
                o = (*childs)[i];
                std::string s(o->name());
                s = s.substr(s.rfind(".")+1);
                if (s == p.first[0]) {
                  /*
                    if (current_module) 
                    cout << "GB_CONFIG: Found matching child of [" << current_module->name() << "] with name [" << p.first[0] << "]" << endl;
                    else
                    cout << "GB_CONFIG: Found matching root module with name [" << p.first[0] << "]" << endl;
                  */
                  current_module = dynamic_cast<sc_module*>(o);
                  found = true;
                }
              }
              
              if (!found) {
                sprintf(ch, "In config file line %d: Module %s not found in module hierarchy.", ln, p.first[0].c_str());
                SC_REPORT_WARNING(TRACENAME, ch);
              }
            }
            break;
            
          case IGNORE: 
            break;
            
          default:
            sprintf(ch, "In config file line %d: Unknown token [%s]\n", ln, token);
            SC_REPORT_WARNING(TRACENAME, ch);
          }
        }
      }
    }    
  }
}


} // namespace tlm

#endif
