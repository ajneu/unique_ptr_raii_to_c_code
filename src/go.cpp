#include <iostream>
#include <memory>

#include "mytype.h"   // old C-interface


struct Deleter_Mytype {
  void operator()(Mytype *p) {
    std::cout << "deleter for Mytype..." << std::endl;
    free_Mytype(p);
  }
};

class Unique_ptr_Mytype : protected std::unique_ptr<Mytype, decltype(Deleter_Mytype())> {
private:
  Mytype *ptr;
  using BaseUniquePtr = std::unique_ptr<Mytype, decltype(Deleter_Mytype())>;
public:
  Unique_ptr_Mytype(Mytype *p = nullptr)
    : BaseUniquePtr{p, Deleter_Mytype()}, ptr{p}
  {
  }

  ~Unique_ptr_Mytype()
  {
    check_allocation();
  }

  Mytype *&get() { /* Override the normal get, to return a reference here!
                      This allows us to take the address of the return-value: &(u_ptr.get()) */
    check_allocation();
    return ptr;
  }

  void check_allocation() /* This is necessary, since with get() returning a reference above,
                             the value in ptr can be changed, without this class realizing!!
                             (Note: check_allocation has to be called everywhere)
                           */
  {
    if (ptr != BaseUniquePtr::get()) {
      // value of ptr differs from the old copy held by unique_ptr

      BaseUniquePtr::reset(ptr); /* here we now delete that previous copy (if not nullptr) help by unique_ptr, 
                                    and then give unique_ptr a copy of the current new value of ptr
                                 */
#ifndef NDEBUG
      std::cout << "(ptr changed)" << std::endl;
#endif
    }
  }

  Mytype *release()
  {
    check_allocation();
    ptr = nullptr;
    return BaseUniquePtr::release();
  }

  void reset(Mytype *p = nullptr)
  {
    check_allocation();
    BaseUniquePtr::reset(p);
  }

  void swap(unique_ptr& other)
  {
    check_allocation();
    BaseUniquePtr::swap(other);
  }


  typename std::add_lvalue_reference<Mytype>::type operator*() const
  {
    const_cast<Unique_ptr_Mytype *>(this)->check_allocation();
    return *ptr;
    //return BaseUniquePtr::operator*();
  }

  Mytype* operator->() const
  {
    const_cast<Unique_ptr_Mytype *>(this)->check_allocation();
    return ptr;
    //return BaseUniquePtr::operator->();
  }

};

int main()
{
  {
    //typical usage
    ///////////////
    Mytype *mt;
    alloc_Mytype(&mt);
    // ... // std::cout << mt->i << std::endl;
    free_Mytype(mt);   // danger: this can be forgotten!!
  }

  std::cout << '\n' << std::endl;
  
  {
    // RAII-approach
    Unique_ptr_Mytype mt2;
    alloc_Mytype(&(mt2.get()));
    // ... // std::cout << mt2->i << std::endl;
  }

  std::cout << '\n' << std::endl;
  
  {
    // RAII-approach
    Unique_ptr_Mytype mt3;
    alloc_Mytype(&(mt3.get()));
    // ... // std::cout << mt3->i << std::endl;
    alloc_Mytype(&(mt3.get())); // yes doing this again is safe!
  }

  return 0;
}
