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

#ifndef _MemoryUtils_h
#define _MemoryUtils_h

#define BOOST_SP_DISABLE_THREADS

#ifndef DONT_USE_BOOST_POOL
#  ifdef SC_INCLUDE_DYNAMIC_PROCESSES
#    warning Warning: Boost pool is incompatible with dynamic processes at SystemC 2.1
#  endif
#endif

#ifndef DONT_USE_BOOST_POOL
# include "boost/pool/object_pool.hpp"
#endif
#include "boost/shared_ptr.hpp"
#include "boost/shared_array.hpp"
#include "boost/bind.hpp"
#include "shared_ptr.h"

namespace tlm{

  template <class T>
  class ObjectPool{
    struct entry{
      public:
      entry(T* content){
        that=content;
        next=NULL;
      }
      
      T* that;
      entry* next;
    };
    
  public:
    ObjectPool(int size): used(NULL){ // (CS) Warning: potential memory issue when size is big
      unused=new entry(new T());  //create first one
      mine.push_back(unused->that);
      for (int i=0; i<size-1; i++){
        entry* e=new entry(new T());
        e->next=unused;
        unused=e;
        mine.push_back(unused->that);
      }
    }
    
    ~ObjectPool(){
      //delete all T* that belong to this pool
      for (unsigned int i=0; i<mine.size(); i++){
        delete mine[i];
      }
      
      //delete all unused elements
      while (unused){
        entry* e=unused;
        unused=unused->next;
        delete e;
      }

      //delete all used elements
      while (used){
        entry* e=used;
        used=used->next;
        delete e;
      }
    }
    
    bool is_from(T* cont){ //slow!!!
      for (int i=0; i<mine.size(); i++){
        if (mine[i]==cont) return true;
      }
      return false;
    }
    
    T* construct(){
      entry* e;
      if (unused==NULL){
        e=new entry(new T());
        mine.push_back(e->that);
      }
      else{
        e=unused;
        unused=unused->next;
      }
      e->next=used;
      used=e;
      return used->that; //if all elements of pool are used, just create a new one and go on      
    }

    void free (T* cont){
      assert(used);
      entry* e=used;
      used=e->next;
      e->that=cont;
      e->next=unused;
      unused=e;
    }
    
  private:
    entry* unused;
    entry* used;
    std::vector<T*> mine; //just for clean up and is_from
  };


  //---------------------------------------------------------------------------
  /**
   * Abstraction for object creation, plain as can be
   */
  //---------------------------------------------------------------------------
  template <typename T,bool needsRelease=false>
  struct ObjectFactory
  {
    T create() {
      return T();
    }
  };

#ifdef USE_STATIC_CASTS
#define PTRTYPE tlm::shared_ptr
#else
#define PTRTYPE boost::shared_ptr
#endif

  //---------------------------------------------------------------------------
  /**
   * Specialization for shared pointers from a pool, for objects that
   * do not need release
   */
  //---------------------------------------------------------------------------
  template <typename T>
  struct ObjectFactory<PTRTYPE<T>, false>
  {
    ObjectFactory( int poolSize=50/*512*/ ) : // (CS) Warning: potential memory issue when size is 512
      m_pool( poolSize )
      {
        (livingFactories)[this]=true;
      };

    ~ObjectFactory(){
      (livingFactories)[this]=false; // necessary to avoid segfault at GreenBus deconstruction
    }

    inline PTRTYPE<T> create()
    {
      T* tmp=m_pool.construct(); // ruft constructor auf!
      // return a shared_ptr that calls our release function on deletion
      return PTRTYPE<T>(tmp,
                                  boost::bind(&ObjectFactory<PTRTYPE<T> >::st_release, tmp, this) );
    };
    
    bool is_from(PTRTYPE<T> t) 
      {
        return m_pool.is_from(t.get());
      };
    
  private:

    inline void release( T* p )
    {
      m_pool.free( p );
    }
#ifdef DONT_USE_BOOST_POOL
    ObjectPool<T> m_pool;
#else
    boost::object_pool<T> m_pool;
#endif
    
    
    //this static method is called whenever a T* of factory fact is about to be freed.
    //the method checks whether the factory is still alive and if so frees T, otherwise nothing will happen.
    static void st_release( T* p, ObjectFactory<PTRTYPE<T>, false>* fact)
    {
      if ((livingFactories)[fact]){ 
#ifdef DONT_USE_BOOST_POOL
        p->reset();
#endif
        fact->release(p);
      }
    }

    static std::map< ObjectFactory<PTRTYPE<T>, false>*, bool> livingFactories;

  };
  
  template <typename T>
  std::map< ObjectFactory<PTRTYPE<T>, false>*, bool> ObjectFactory<PTRTYPE<T>, false>::livingFactories;



#ifndef DONT_USE_BOOST_POOL
  //---------------------------------------------------------------------------
  /**
   * Specialization for shared pointers from a pool, for objects that
   * need release
   */
  //---------------------------------------------------------------------------
  template <typename T>
  struct ObjectFactory<PTRTYPE<T>,true >
  {
    ObjectFactory( int poolSize=512 ) :
      m_pool( poolSize )
    {}

    PTRTYPE<T> create()
    {
      return PTRTYPE<T>(m_pool.construct(), // let TransactionContainer decide whether to reinitialize
                                  boost::bind(&ObjectFactory<PTRTYPE<T>,true>::release,
                                              this, _1 ) );
    }

  private:

    void release( T* p )
    {
      p->release(); // this is the only difference from needsRelease=false
      m_pool.free( p );
    }

    boost::object_pool<T> m_pool;

  };
#endif


#ifndef DONT_USE_BOOST_POOL
  //---------------------------------------------------------------------------
  /**
   * Specialization for shared arrays
   */
  //---------------------------------------------------------------------------
  template <typename T>
  struct ObjectFactory<boost::shared_array<T> >
  {
    ObjectFactory( int poolSize=sizeof(T) ) :
      m_pool( poolSize )
    {}

    boost::shared_array<T> create( int n )
    {
      T* pTmp = (T*)m_pool.ordered_malloc( n );
      return boost::shared_array<T>(
            pTmp,
            boost::bind( &boost::pool<>::ordered_free, &m_pool, _1, n ) );
    }

  private:

    boost::pool<> m_pool;

  };
#endif


  //---------------------------------------------------------------------------
  /**
   * Abstraction for object de-referencing
   */
  //---------------------------------------------------------------------------
  template <typename T>
  struct TypeChar {
    typedef T Type;
  };

  template <typename T>
  struct TypeChar<PTRTYPE<T> > {
    typedef T Type;
  };
      
  template <typename T>
  const T& deref( const T& t ) {
    return t;
  }

  template <typename T>
  const T& deref( const PTRTYPE<T>& sp ) {
    return *sp;
  }

  template <typename T>
  T& deref( T& t ) {
    return t;
  }

  template <typename T>
  T& deref( PTRTYPE<T>& sp ) {
    return *sp;
  }

#undef PTRTYPE
} // end of namespace tlm


#endif /* _MemoryUtils_h */
