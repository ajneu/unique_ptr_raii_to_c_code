#ifndef DELETER_H
#define DELETER_H

// OPTION: one of the following 2 lines must be commented
#define FUNC_COPY
//#undef  FUNC_COPY


template <typename T, typename F_Free> // F_Free is a function that has *T as parameter, e.g. TRet func(*T)
class Deleter {
public:
  Deleter() {}
  Deleter(const F_Free& f) : free_func{f} {}

#ifdef FUNC_COPY
  Deleter(Deleter<T, F_Free> &&del) : free_func{std::move(del.free_func)} {}

  Deleter &operator=(Deleter<T, F_Free> &&del) {
    free_func = std::move(del.free_func);
  }
#endif
  
  void operator()(T *p) {
    std::cout << "deleter for Mytype..." << std::endl;
    free_func(p);
  }
private:
#ifdef FUNC_COPY
  F_Free free_func;
#else
  const F_Free &free_func;
#endif
};


#endif
