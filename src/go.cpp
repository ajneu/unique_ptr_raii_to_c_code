#include <iostream>
#include <memory>


#include "mytype.h"   // old C-interface



#define PLAIN_FUNC_PTR

#ifndef PLAIN_FUNC_PTR
// std::function
#include <functional>
#define STDFUNC(TRet, TArg) std::function<TRet (TArg)>
#else
// plain function pointers
#define STDFUNC(TRet, TArg) TRet(*)(TArg)
#endif




template <typename T, typename F_Free>
class Deleter2 {
public:
  Deleter2() {}
  Deleter2(const F_Free& f) : free_func{f} {}
  void operator()(T *p) {
    std::cout << "deleter for Mytype..." << std::endl;
    free_func(p);
  }
private:
  const F_Free free_func;
};

/*
void  free_Mytype(struct Mytype  *p);
void alloc_Mytype(struct Mytype **p);
*/

template <typename T, typename F_Alloc, typename F_Free, typename... Params>
class Raii_Manager : std::unique_ptr<T, decltype(Deleter2<T, F_Free>())> {
  using Base = std::unique_ptr<T, decltype(Deleter2<T, F_Free>())>;
public:
  
  Raii_Manager(F_Alloc f_alloc, F_Free f2, Params... args) : Base{invoke_alloc(f_alloc, args...), Deleter2<T, F_Free>(f2)}
  {
  }
  ~Raii_Manager()
  {
  }
private:
  static T* invoke_alloc(F_Alloc f_alloc, Params... args)
  {
    T *ptr;
    f_alloc(&ptr, args...);
    return ptr;
  }
};

class MytypeRAII2 : public Raii_Manager<Mytype, void(*)(Mytype **), void(*)(Mytype *)> {
  using Base =             Raii_Manager<Mytype, void(*)(Mytype **), void(*)(Mytype *)>;
public:
  MytypeRAII2() : Base{alloc_Mytype, free_Mytype}
  {}

};






template <typename TRet, typename TArg>
class Deleter {
  using Func = STDFUNC(TRet, TArg *);
public:
  Deleter() {}
  Deleter(const Func& f) : free_func{f} {}
  void operator()(TArg *p) {
    std::cout << "deleter for Mytype..." << std::endl;
    free_func(p);
  }
private:
  const Func free_func;
};




template <typename TRet, typename TArg>
class Unique_ptr_Mytype : protected std::unique_ptr<TArg, decltype(Deleter<TRet, TArg>())> {
private:
  TArg *ptr;
  using Deleter1 = Deleter<TRet, TArg>;
  using BaseUniquePtr = std::unique_ptr<TArg, decltype(Deleter1())>;
  using Func = STDFUNC(TRet, TArg *);
public:
  Unique_ptr_Mytype(const Func& f, TArg *p = nullptr)
    : BaseUniquePtr{p, Deleter1(f)}, ptr{p}
  {
  }

  ~Unique_ptr_Mytype()
  {
    check_allocation();
  }

  TArg *&get() { /* Override the normal get, to return a reference here!
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

  TArg *release()
  {
    check_allocation();
    ptr = nullptr;
    return BaseUniquePtr::release();
  }

  void reset(TArg *p = nullptr)
  {
    check_allocation();
    BaseUniquePtr::reset(p);
  }

  void swap(BaseUniquePtr& other)
  {
    check_allocation();
    BaseUniquePtr::swap(other);
  }


  typename std::add_lvalue_reference<TArg>::type operator*() const
  {
    const_cast<Unique_ptr_Mytype *>(this)->check_allocation();
    return *ptr;
    //return BaseUniquePtr::operator*();
  }

  TArg* operator->() const
  {
    const_cast<Unique_ptr_Mytype *>(this)->check_allocation();
    return ptr;
    //return BaseUniquePtr::operator->();
  }

};


class MytypeRAII : public Unique_ptr_Mytype<void, Mytype> {
  using Base = Unique_ptr_Mytype<void, Mytype>;
public:
  MytypeRAII(Mytype *p = nullptr) : Base{free_Mytype, p}
  {}
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
    MytypeRAII mt2;
    alloc_Mytype(&(mt2.get()));
    // ... // std::cout << mt2->i << std::endl;
  }

  std::cout << '\n' << std::endl;
  
  {
    // RAII-approach
    MytypeRAII mt3;
    alloc_Mytype(&(mt3.get()));
    // ... // std::cout << mt3->i << std::endl;
    alloc_Mytype(&(mt3.get())); // yes doing this again is safe!
  }

  std::cout << '\n' << std::endl;
    
  {
    // RAII2-approach
    MytypeRAII2 mt4;
    // ... // std::cout << mt4->i << std::endl;

  }
  return 0;
}
