#include <iostream>
#include "uniq_ptr_raii_func_wrapper.h"
#include "uniq_ptr_raii_delayed_ra_func_wrapper.h"

#include "mytype.h"   // old C-interface



#ifdef FUNC_COPY
class MytypeRAII : public Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(&alloc_Mytype), decltype(&free_Mytype), double> {
  using Base =            Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(&alloc_Mytype), decltype(&free_Mytype), double>;
public:
  MytypeRAII(double x) : Base{&alloc_Mytype, &free_Mytype, x}
  {}

};
#else
class MytypeRAII : public Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(alloc_Mytype), decltype(free_Mytype), double> {
  using Base =            Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(alloc_Mytype), decltype(free_Mytype), double>;
public:
  MytypeRAII(double x) : Base{alloc_Mytype, free_Mytype, x}
  {}

};
#endif




#ifdef FUNC_COPY
class MytypeRAII_delayed : public Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(&free_Mytype)> {
  using Base =                    Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(&free_Mytype)>;
public:
  MytypeRAII_delayed(Mytype *p = nullptr) : Base{&free_Mytype, p}
  {}
};
#else
class MytypeRAII_delayed : public Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(free_Mytype)> {
  using Base =                    Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(free_Mytype)>;
public:
  MytypeRAII_delayed(Mytype *p = nullptr) : Base{free_Mytype, p}
  {}
};
#endif


int main()
{
  {
    //typical C usage (dangerous: one can forget free_Mytype)
    ///////////////
    Mytype *mt;
    alloc_Mytype(&mt, 0.0);
    // ... // std::cout << mt->i << std::endl;
    free_Mytype(mt);   // DANGER: THIS CAN BE FORGOTTEN!!
  }

  std::cout << '\n' << std::endl;
  
  {
    // RAII-approach     // traditional RAII
    MytypeRAII mt4(0.0);
    // ... // std::cout << mt4->i << std::endl;
    mt4.reset(3.3);      // custom reset function which frees prev pointer and invokes the allocate function again!
#ifdef FUNC_COPY
    mt4 = std::move(MytypeRAII(1.1)); // invoke allocate on temporary, then free current contents of m4, then move temporary
    MytypeRAII mt5{std::move(mt4)};   // no new allocation, just move
#endif
  }

  std::cout << '\n' << std::endl;
  
  {
    // RAII with delayed RA (delayed resource aquisition)
    MytypeRAII_delayed mt2;
    alloc_Mytype(&(mt2.get()), 0.0); // RA now
    // ... // std::cout << mt2->i << std::endl;
  }

  std::cout << '\n' << std::endl;
  
  {
    // RAII with delayed RA (delayed resource aquisition)
    MytypeRAII_delayed mt3;
    alloc_Mytype(&(mt3.get()), 0.0); // RA now
    // ... // std::cout << mt3->i << std::endl;
    alloc_Mytype(&(mt3.get()), 0.0); // yes doing this again is safe!
  }

  return 0;
}
