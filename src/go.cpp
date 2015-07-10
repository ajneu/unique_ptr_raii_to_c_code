#include <iostream>
#include <memory>


#include "mytype.h"   // old C-interface


// OPTION: one of the following 2 lines must be commented
//#define FUNC_COPY
#undef  FUNC_COPY

template <typename T, typename F_Free> // F_Free is a function that has *T as parameter, e.g. TRet func(*T)
class Deleter {
public:
  Deleter() {}
  Deleter(const F_Free& f) : free_func{f} {}
  void operator()(T *p) {
    std::cout << "deleter for Mytype..." << std::endl;
    free_func(p);
  }
private:
#ifdef FUNC_COPY
  const F_Free free_func;
#else
  const F_Free &free_func;
#endif
};


template <typename T, typename F_Alloc, typename F_Free, typename... Params>
class Uniq_Ptr_Raii_Func_Wrapper : std::unique_ptr<T, decltype(Deleter<T, F_Free>())> {
  using Base = std::unique_ptr<T, decltype(Deleter<T, F_Free>())>;
public:
  
  Uniq_Ptr_Raii_Func_Wrapper(const F_Alloc &f_alloc, const F_Free &f2, Params... args) : Base{invoke_alloc(f_alloc, args...), Deleter<T, F_Free>(f2)}
  {
  }
  ~Uniq_Ptr_Raii_Func_Wrapper()
  {
  }
private:
  static T* invoke_alloc(const F_Alloc& f_alloc, Params... args)
  {
    T *ptr;
    f_alloc(&ptr, args...);
    return ptr;
  }
};


#ifdef FUNC_COPY
class MytypeRAII2 : public Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(&alloc_Mytype), decltype(&free_Mytype), double> {
  using Base =             Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(&alloc_Mytype), decltype(&free_Mytype), double>;
public:
  MytypeRAII2(double x) : Base{&alloc_Mytype, &free_Mytype, x}
  {}

};
#else
class MytypeRAII2 : public Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(alloc_Mytype), decltype(free_Mytype), double> {
  using Base =             Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(alloc_Mytype), decltype(free_Mytype), double>;
public:
  MytypeRAII2(double x) : Base{alloc_Mytype, free_Mytype, x}
  {}

};
#endif




// delayed Ra (delayed resource aquisition)

template <typename T, typename F_Free>
class Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper : protected std::unique_ptr<T, decltype(Deleter<T, F_Free>())> {
private:
  T *ptr;
  using Deleter1 = Deleter<T, F_Free>;
  using BaseUniquePtr = std::unique_ptr<T, decltype(Deleter1())>;
public:
  Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper(const F_Free& f2, T *p = nullptr)
    : BaseUniquePtr{p, Deleter1(f2)}, ptr{p}
  {
  }

  ~Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper()
  {
    check_allocation();
  }

  T *&get() { /* Override the normal get, to return a reference here!
                 This allows us to take the address of the return-value: &(obj.get()) */
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

      BaseUniquePtr::reset(ptr); /* here we now delete that previous copy (if not nullptr) held by unique_ptr, 
                                    and then give unique_ptr a copy of the current new value of ptr
                                 */
#ifndef NDEBUG
      std::cout << "(ptr changed)" << std::endl;
#endif
    }
  }

  T *release()
  {
    check_allocation();
    ptr = nullptr;
    return BaseUniquePtr::release();
  }

  void reset(T *p = nullptr)
  {
    check_allocation();
    BaseUniquePtr::reset(p);
  }

  void swap(BaseUniquePtr& other)
  {
    check_allocation();
    BaseUniquePtr::swap(other);
  }


  typename std::add_lvalue_reference<T>::type operator*() const
  {
    const_cast<Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper *>(this)->check_allocation();
    return *ptr;
    //return BaseUniquePtr::operator*();
  }

  T* operator->() const
  {
    const_cast<Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper *>(this)->check_allocation();
    return ptr;
    //return BaseUniquePtr::operator->();
  }

};

#ifdef FUNC_COPY
class MytypeRAII : public Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(&free_Mytype)> {
  using Base =            Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(&free_Mytype)>;
public:
  MytypeRAII(Mytype *p = nullptr) : Base{&free_Mytype, p}
  {}
};
#else
class MytypeRAII : public Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(free_Mytype)> {
  using Base =            Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(free_Mytype)>;
public:
  MytypeRAII(Mytype *p = nullptr) : Base{free_Mytype, p}
  {}
};
#endif


int main()
{
  {
    //typical usage
    ///////////////
    Mytype *mt;
    alloc_Mytype(&mt, 0.0);
    // ... // std::cout << mt->i << std::endl;
    free_Mytype(mt);   // danger: this can be forgotten!!
  }

  std::cout << '\n' << std::endl;
  
  {
    // RAII-approach
    MytypeRAII mt2;
    alloc_Mytype(&(mt2.get()), 0.0);
    // ... // std::cout << mt2->i << std::endl;
  }

  std::cout << '\n' << std::endl;
  
  {
    // RAII-approach
    MytypeRAII mt3;
    alloc_Mytype(&(mt3.get()), 0.0);
    // ... // std::cout << mt3->i << std::endl;
    alloc_Mytype(&(mt3.get()), 0.0); // yes doing this again is safe!
  }

  std::cout << '\n' << std::endl;
    
  {
    // RAII2-approach
    MytypeRAII2 mt4(0.0);
    // ... // std::cout << mt4->i << std::endl;

  }
  return 0;
}
