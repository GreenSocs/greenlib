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

// make GreenBus communicative
#define VERBOSE 

#include "systemc.h"

#ifdef USE_DUST
   // uncomment the following line if you want to enable DUST structure analysis
   //#include "greenbus/utils/dust_model.h"
   // uncomment the following line if you want to enable DUST strucutre analysis and transaction recording
   #include "greenbus/utils/dust_model_and_transactions.h"
   // uncomment the following line if you want to enable DUST strucutre analysis and full transaction introspection
   //#include "greenbus/utils/dust_model_and_introspection.h"
#endif

#include "greenbus/transport/GP/GP.h"
#include "greenbus/api/shipAPI.h"
#include "sys/time.h"

#include "greenbus/utils/gb_config.h"


//#define USE_PV
#define USE_PVT
#define CLK_PERIOD 10

// make ship example communicative
//#define DEBUG

// print comma separated benchmark results
//#define CSV_MODE

/// how many objects are transferred sequentially for each measurement?
#define OBJ_COUNT 10
/// how many benchmark cycles?
#define LOOPS 14

/// initial size of ship objects
#define OBJ_SIZE 1024 // 1kb


//------------------------------------------------------------------------------------------------------
// Time measurement helper functions
//------------------------------------------------------------------------------------------------------

// time measure functions 
timeval tv_start, tv_stop;
inline void startMeasure() {
  gettimeofday(&tv_start, NULL);
}

inline void stopMeasure() {
  gettimeofday(&tv_stop, NULL);
#ifndef CSV_MODE
  cout << "+++ Duration +++ " << tv_stop.tv_sec-tv_start.tv_sec << "s " 
       << (((tv_stop.tv_usec-tv_start.tv_usec)<0) ? 
           -1*(tv_start.tv_usec-tv_stop.tv_usec) :
           (tv_stop.tv_usec-tv_start.tv_usec)) << "us." << endl;
#else
  cout << (tv_stop.tv_sec-tv_start.tv_sec)*1000000 +
          (((tv_stop.tv_usec-tv_start.tv_usec)<0) ? 
           -1*(tv_start.tv_usec-tv_stop.tv_usec) :
           (tv_stop.tv_usec-tv_start.tv_usec));
#endif
}



//------------------------------------------------------------------------------------------------------
// A simple SHIP object
//------------------------------------------------------------------------------------------------------

/// This is the datatype which is transferred through the channel under test
class shipobj : ship_serializable_if {
public:
  /// default constructor, creates an empty ship object
  shipobj() 
  {
    m_data.resize(OBJ_SIZE);
	}

  /// create a new shipobj with given size and fill it with test data
  shipobj(uint size) 
  {
    m_data.resize(size);
    // fill with test data
    for (uint i=0; i<size; i++)
      m_data[i] = shipobj::fill++;
  }

  /// copy constructor
  shipobj(const shipobj& obj) 
  {
    *this = obj; // use copy-operator
  }
  
  /// destructor
  virtual ~shipobj() {
  }
  
  /// copy-operator
  shipobj& operator = (const shipobj& obj) {
    if (this != &obj) { // !self-assignment?
      m_data = obj.m_data;
    }
    return *this;
  }
  
  /// return a serial presentation of this ship object
  virtual const gs_uint32 serialize(GSDataType &data) { 
    data.setData(m_data);
    return data.getSize();
  }
  
  /// copy serial data array into this ship object
  virtual const gs_uint32 deserialize(GSDataType &data) { 
    m_data.resize(data.getSize());
      
    for (uint i=0; i<data.getSize(); i++)
      m_data[i] = data[i];
    return data.getSize();
  }
  
  /// get the serial data stream length of this ship object
  const unsigned int getSerialLength() 
  { 
    return m_data.size();
  }

  /// print contents of this SHIP object
  void print() {
//    cout << "shipobj [ ";
//    for (uint i=0; i<m_data.size(); i++)
//      cout << m_data[i] << " ";
//    cout << "]" << endl;
  }
  
  std::vector<gs_uint8> m_data;

  static int fill;
};

int shipobj::fill = 0;




//------------------------------------------------------------------------------------------------------
// The Producer
//------------------------------------------------------------------------------------------------------

/// The producer continuously sends data to the channel under test.
class producer 
: public sc_module,
  public gb_configurable
{
public:
  shipMasterAPI<shipobj> out_port;
 
  SC_CTOR(producer) 
  : out_port("out") 
    {
#ifdef USE_PVT
      out_port.mode = SHIP_MODE_BA_P;
      out_port.clk_period = CLK_PERIOD;
      cout << name() << " I am a SHIP master running in PVT mode, clk_period="<<CLK_PERIOD<<"." << endl;
#else
      cout << name() << " I am a SHIP master running in PV mode." << endl;
#endif

      SC_THREAD(play);
    }    
  
    void play() {
      for (int c=0; c<LOOPS; c++) {

#ifndef CSV_MODE
        cout << endl << name() << " sending " << OBJ_COUNT << " SHIP objects of " << m_size << " size... " << endl;
#else
        cout << m_size << ", ";
#endif

        shipobj obj(m_size);
        
        {
          startMeasure();
          for(uint i=0; i<OBJ_COUNT; i++) {
            //shipobj obj(m_size);           // this slows down things SIGNIFICANTLY
            out_port.send(obj);
            wait(1, SC_NS);
          }
          stopMeasure();
        }


#ifndef CSV_MODE
        cout << endl << name() << " requesting " << OBJ_COUNT << " SHIP objects..." << endl;
#else
        cout << ", ";
#endif
        
        {
          startMeasure();
          for(uint i=0; i<OBJ_COUNT; i++) {
            shipobj obj;
            out_port.request(obj);
            
#ifdef DEBUG
            cout << name() << " got ";
            obj.print();
#endif
            
            wait(1, SC_NS);
          }
          stopMeasure();
          cout << endl;
        }

        // increase obj size for next cycle
        m_size *= 2;
        //m_size += OBJ_SIZE;
      }
    }  
    
    static uint m_size;
};

uint producer::m_size = OBJ_SIZE;



//------------------------------------------------------------------------------------------------------
// The Consumer
//------------------------------------------------------------------------------------------------------

/// The consumer eats data objects from the channel under test.
class consumer 
: public sc_module,
  public gb_configurable
{
public:
  shipSlaveAPI<shipobj> in_port;

  SC_HAS_PROCESS(consumer);
  consumer(sc_module_name name) 
  : sc_module(name),
	  in_port("in")
    {
#ifdef USE_PVT
      in_port.mode = SHIP_MODE_BA_P;
      in_port.clk_period = CLK_PERIOD;
      cout << sc_module::name() << " I am a SHIP slave running in PVT mode, clk_period="<<CLK_PERIOD<<"." << endl;
#else
      cout << sc_module::name() << " I am a SHIP slave running in PV mode." << endl;
#endif

      SC_THREAD(play);
    }
 
  void play() {
    for(;;) {
      ship_command cmd = in_port.waitEvent();

      if (cmd.cmd == SHIP_SEND) {
        shipobj obj;
        in_port.recv(obj);

#ifdef DEBUG
        cout << name() << " got ";
        obj.print();
#endif

      }
      else if (cmd.cmd == SHIP_REQUEST) {
        shipobj obj(producer::m_size);
        in_port.reply(obj);
      }
    }
  }  
};


int sc_main(int argc, char *argv[]) {
 
#ifdef USE_DUST
  DUST_INITIALIZE;
#endif

  if (argc < 2) {
    cout << endl << "SHIP Example usage:" << endl;   
    cout << "   " << argv[0] << " [gb_configfile]" << endl << endl;
    return EXIT_FAILURE;
  }

 
  // build system hierarchy
  producer p("Producer");
  consumer c("Consumer");
  
#ifdef CSV_MODE
  cout << endl << "bytesize, send-usec, request-usec" << endl;
#endif
  
  gb_config(argv[1]);

  sc_start();
  
  return EXIT_SUCCESS;
}  


