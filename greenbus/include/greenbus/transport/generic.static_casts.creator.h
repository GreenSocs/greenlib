// LICENSETEXT
// 
//   Copyright (C) 2007 : GreenSocs Ltd
//       http://www.greensocs.com/ , email: info@greensocs.com
// 
//   Developed by :
// 
//   Christian Schroeder, Wolfgang Klingauf
//     Technical University of Braunschweig, Dept. E.I.S.
//     http://www.eis.cs.tu-bs.de
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

template<typename ACC1, typename ACC2, typename ACC3, typename BASE>
struct transactionCreator : public virtual ACC1, public virtual ACC2, public virtual ACC3{
  transactionCreator(){ 
    ACC1::cr=this;
    /*
    cout<<"CTOR"<<endl
        <<"creator : "<<hex<<this<<endl
        <<"acc1    : "<<((ACC1*)(this))<<endl
        <<"acc2    : "<<((ACC2*)(this))<<endl
        <<"acc3    : "<<((ACC3*)(this))<<endl
        <<"base    : "<<((BASE*)(this))<<endl;*/
    }
  ~transactionCreator(){
    //cout<<"delete"<<endl;
  }
};

#ifndef TRANSACTION_MANDATORY_STUFF

#define TRANSACTION_MANDATORY_STUFF(ACC1,ACC2,ACC3, BASE)\
friend class transactionCreator<ACC1,ACC2,ACC3,BASE>; \
public: \
  transactionCreator<ACC1,ACC2,ACC3,BASE>* getSuperClassPtr () {return cr;} \
protected: \
  transactionCreator<ACC1,ACC2,ACC3,BASE>* cr

#endif
