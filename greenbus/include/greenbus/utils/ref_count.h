// This code is based on section 16.22 from Marshall Cline's C++ FAQ Lite document
// http://www.parashift.com/c++-faq-lite/freestore-mgmt.html#faq-16.22

// DONOTRELICENSE

/* #define DBG_REF_COUNT(x) printf(x) */
#define DBG_REF_COUNT(x)


template <class T>
class ref_count {
public:
  /*   private: */
  class internal: public T
  {
  public:
    internal() : count_(0) /*...*/ { }  // All ctors set count_ to 0 !
  private:
    friend class ref_count;     // A friend class
    unsigned count_;
    // count_ must be initialized to 0 by all constructors
    // count_ is the number of ref_count objects that point at this
  };
  internal* p_;    // p_ is never NULL
public:
  internal* operator-> () { return p_; }
  internal& operator* ()  { return *p_; }
  bool operator == (const ref_count<T>& other) const {
    return other.p_ == p_;
  }
/*   ref_count()   {  } */
  ref_count(internal* p)    : p_(p) { DBG_REF_COUNT("creating ref_count\n"); ++p_->count_; }  // p must not be NULL
  ~ref_count()           { DBG_REF_COUNT("deleting ref_count\n"); if (--p_->count_ == 0) delete p_; }
  ref_count(const ref_count& p) : p_(p.p_) { DBG_REF_COUNT("copy constructor ref_count\n"); ++p_->count_; }
  ref_count& operator= (const ref_count<T>& p)
  { // DO NOT CHANGE THE ORDER OF THESE STATEMENTS!
    // (This order properly handles self-assignment)
    // (This order also properly handles recursion, e.g., if a internal contains ref_counts)
    DBG_REF_COUNT("assign operator ref_count\n"); 
    internal* const old = p_;
    p_ = p.p_;
    ++p_->count_;
    if (--old->count_ == 0) delete old;
    return *this;
  }

  template <class X>
  operator ref_count<X>& () {
    return reinterpret_cast<ref_count<X>&>(*this);
  }

};
