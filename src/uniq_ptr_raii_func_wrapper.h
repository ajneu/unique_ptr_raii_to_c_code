#ifndef UNIQ_PTR_RAII_FUNC_WRAPPER_H
#define UNIQ_PTR_RAII_FUNC_WRAPPER_H

#include <memory>

#include "deleter.h"


template <typename F_Alloc>
struct AllocFuncStorage {
 public:
  AllocFuncStorage (const F_Alloc &f_alloc)
    : alloc_func{f_alloc}
  {}

#ifdef FUNC_COPY
  AllocFuncStorage (AllocFuncStorage &&allo)
  : alloc_func{std::move(allo.alloc_func)}
  {}

  AllocFuncStorage &operator=(AllocFuncStorage &&allo) 
  {
    alloc_func = std::move(allo.alloc_func);
  }
#endif
    
#ifdef FUNC_COPY
  F_Alloc  alloc_func;
#else
  const F_Alloc &alloc_func;
#endif

};


template <typename T, typename F_Alloc, typename F_Free, typename... Params>
class Uniq_Ptr_Raii_Func_Wrapper : protected AllocFuncStorage<F_Alloc>, public std::unique_ptr<T, decltype(Deleter<T, F_Free>())> {
  using Base = std::unique_ptr<T, decltype(Deleter<T, F_Free>())>;
public:
  
  Uniq_Ptr_Raii_Func_Wrapper(const F_Alloc &f_alloc, const F_Free &f2, Params... args)
    : AllocFuncStorage<F_Alloc>{f_alloc}, Base{invoke_alloc(args...), Deleter<T, F_Free>(f2)}
  {
  }

  Uniq_Ptr_Raii_Func_Wrapper(Uniq_Ptr_Raii_Func_Wrapper &&un)
    : AllocFuncStorage<F_Alloc>{std::move(un)}, Base::unique_ptr{std::move(un)}
  {
  }

  Uniq_Ptr_Raii_Func_Wrapper &operator=(Uniq_Ptr_Raii_Func_Wrapper &&un)
  {
    // static_cast<AllocFuncStorage<F_Alloc> &>(*this) = std::move(un); // static_cast<AllocFuncStorage<F_Alloc> &&>(un);
    AllocFuncStorage<F_Alloc>::operator=(std::move(un));

    // static_cast<Base &>(*this) = std::move(un); // static_cast<Base &&>(un);
    Base::operator=(std::move(un));
  }
  
  
  ~Uniq_Ptr_Raii_Func_Wrapper()
  {
  }
  void reset(Params... args)
  {
    Base::reset(invoke_alloc(args...));
  }
private:
  T* invoke_alloc(Params... args)
  {
    T *ptr;
    AllocFuncStorage<F_Alloc>::alloc_func(&ptr, args...);
    return ptr;
  }

};

#endif
