#ifndef UNIQ_PTR_RAII_FUNC_WRAPPER_H
#define UNIQ_PTR_RAII_FUNC_WRAPPER_H

#include <memory>

#include "deleter.h"

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

#endif
