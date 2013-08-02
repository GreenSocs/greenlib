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


#ifndef __ATTRIBUTEBASE_H__
#define __ATTRIBUTEBASE_H__

#include <string>
#include <vector>
#include "assert.h"


namespace tlm {

//---------------------------------------------------------------------------
/**
 * Virtual root for all attributes. It is used by the quark iterators
 * to store references of template specialised attributes.
 */
//---------------------------------------------------------------------------
class AttributeRoot {
public:
  AttributeRoot(){}
  virtual ~AttributeRoot(){}
  
  virtual const std::string &getName() const =0;
  virtual const gs_uint32 getSize() const =0;
  virtual const gs_uint8* serialize()=0;
  virtual void setName(const std::string &_name)=0;
};



// forward declaration necessary for stream operator declarations
// see http://www.ica1.uni-stuttgart.de/Courses_and_Lectures/C++/script/node23.html
template <class T> class AttributeBase;
template <class T> class AttributeStackBase;

// stream operator declarations (implementation is in attributes.h)
template <class T> std::ostream& operator<< (std::ostream &, const AttributeBase<T> &);
template <class T> std::istream& operator>> (std::istream &, AttributeBase<T> &);

template <class T> std::ostream& operator<< (std::ostream &, const AttributeStackBase<T> &);
template <class T> std::istream& operator>> (std::istream &, AttributeStackBase<T> &);

//---------------------------------------------------------------------------
/**
 * Basic attribute class.
 */
//---------------------------------------------------------------------------
template <class T>
class AttributeBase : public AttributeRoot {

  /// output stream operator 
  friend std::ostream& operator<< <>(std::ostream &, const AttributeBase<T> &);

  /// input stream operator
  friend std::istream& operator>> <>(std::istream &, AttributeBase<T> &);



#ifdef USE_DUST
  friend class scv_extensions<AttributeBase<T> >;
#endif

private:
  T value;
  std::string *name;

public:  
  /// Default constructor (no initialisation of attribute value)
  AttributeBase() 
    : name(NULL) 
  {}

  /// Initialising constructor
  AttributeBase(const T &obj) 
    : value(obj), 
      name(NULL) 
  {}

  /// Initialising constructor (slow!)
  AttributeBase(const T &obj, const std::string &n) 
    : value(obj) 
  {
    name = new std::string(n);
  }

  /// Copy constructor 
  AttributeBase(const AttributeBase<T> &obj) :
    name(NULL)
  { 
    *this = obj; // use copy operator
  }
  
  /// Destructor
  virtual ~AttributeBase() 
  {
    if (name!=NULL)
      delete name;
  }  
  
  /// copy operator for AttributeBase
  AttributeBase<T>& operator = (const AttributeBase<T> &obj) { 
    // check for self-assignment
    if (&obj==this)
      return *this;
    
    value=obj.value;
    if (obj.name!=NULL) {
      name=new std::string(*obj.name);
    }
    return *this;
  }
  
  /// Assign attribute value operator
  T& operator = (const T &obj) { 
    value=obj; 
    return value;
  }

  /// Return attribute value operator
  operator const T&() const {
    return value;
  }
  
  operator T&() { return value; }

  /// Equals operator
  //bool operator == (const AttributeBase<T> &obj) {
  //  return value==obj.value && name==obj.name;
  //}

  /// Get attribute name
  inline virtual const std::string &getName() const { 
    assert(name!=NULL);
    return *name; 
  } 
  
  /// Set attribute name
  virtual void setName(const std::string &_name)
  { 
    if (name!=NULL)
      delete name;
    name=new std::string(_name);
  }    

  /// Get size of attribute
  inline virtual const gs_uint32 getSize() const { 
    return sizeof(T); 
  } 

  /// Serialize attribute
  inline virtual const gs_uint8* serialize() { 
    return (gs_uint8*) &( this->value); 
  }
  
};

template <class T>
class AttributeStackBase : public AttributeRoot {

  /// output stream operator 
  friend std::ostream& operator<< <>(std::ostream &, const AttributeStackBase<T> &);

  /// input stream operator
  friend std::istream& operator>> <>(std::istream &, AttributeStackBase<T> &);
  
  typedef AttributeBase<T> TYPE;

private:
  std::vector<TYPE> values;
  TYPE* recent;
  TYPE* first;
  TYPE* last;
  int m_size;  
  std::string *name;

public:  
  /// Default constructor (no initialisation of attribute value)
  AttributeStackBase() 
    : name(NULL) 
  {
    init();
  }

  /// Initialising constructor
  AttributeStackBase(const T &obj) 
    : name(NULL) 
  {
    init();
    *recent=obj;
  }

  /// Initialising constructor (slow!)
  AttributeStackBase(const T &obj, const std::string &n) 
  {
    init();
    *recent=obj;
    name = new std::string(n);
  }

  void init(){
    if (values.size()!=16)
      values.resize(16);
    recent=first=&(values[0]);
    m_size=16;
    last=&(values[15]);
  }

  /// Copy constructor 
  AttributeStackBase(const AttributeStackBase<T> &obj) :
    name(NULL)
  { 
    *this = obj; // use copy operator
  }
  
  /// Destructor
  virtual ~AttributeStackBase() 
  {
    if (name!=NULL)
      delete name;
  }  
  
  /// copy operator for AttributeBase
  AttributeStackBase<T>& operator = (const AttributeStackBase<T> &obj) { 
    // check for self-assignment
    if (&obj==this)
      return *this;
    
    values=obj.values;
    recent=obj.recent;
    first=obj.first;
    last=obj.last;
    if (obj.name!=NULL) {
      name=new std::string(*obj.name);
    }
    return *this;
  }
  
  /// Assign attribute value operator
  T& operator = (const T &obj) { 
    *recent=obj; 
    return *recent;
  }

  /// Return attribute value operator
  operator const T&() const {
    return *recent;
  }

  const TYPE& get() const {
    return *recent;
  }
  
  operator T&() { return *recent; }
  
  void push(const T& obj){
    if (recent==last){ //stack is full
      values.resize(m_size*2);
      first=&(values[0]);
      recent=&(values[m_size-1]);
      m_size*=2;
      last=&(values[m_size-1]);
    }
    *(++recent)=obj;
  }

  void pop(){
    assert(recent!=first);
    recent--;
  }
  
  void reset(){
    recent=first;
  }

  /// Equals operator
  //bool operator == (const AttributeBase<T> &obj) {
  //  return value==obj.value && name==obj.name;
  //}

  /// Get attribute name
  inline virtual const std::string &getName() const { 
    assert(name!=NULL);
    return *name; 
  } 
  
  /// Set attribute name
  virtual void setName(const std::string &_name)
  { 
    if (name!=NULL)
      delete name;
    name=new std::string(_name);
  }    

  /// Get size of attribute
  inline virtual const gs_uint32 getSize() const { 
    return sizeof(T); 
  } 

  /// Serialize attribute
  inline virtual const gs_uint8* serialize() { 
    return (gs_uint8*) (this->recent); //if serialized only serialize the recent value.
  }
  
};

  

} // end of namespace tlm

#endif
