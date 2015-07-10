#ifndef UNIQ_PTR_RAII_DELAYED_RA_FUNC_WRAPPER_H
#define UNIQ_PTR_RAII_DELAYED_RA_FUNC_WRAPPER_H

// delayed Ra (delayed resource aquisition)



#include <memory>

#include "deleter.h"




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

#endif
