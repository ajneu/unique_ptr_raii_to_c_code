# The problem: Inconveniences of C-code interfaces

Some C interfaces look like this. Note `alloc_Mytype()` and `free_Mytype()`:

```c
double arg = 0.0;      // Special arg to control something during allocation in alloc_Mytype() function coming up
                       // This is just to show a non-trivial alloc function below 
                       // (see https://libgit2.github.com/libgit2/#HEAD/group/repository/git_repository_init 
                       //  for a real-world example alloc-function, that takes args)
                       
Mytype *x;
alloc_Mytype(&x, arg); // allocate storage on the heap and set the address of that allocated storage as the value of pointer x
free_Mytype(x);        // use the value of pointer x, to locate the storage that is then freed
```

Here's a real-world example: [libgit2](https://libgit2.github.com/docs/guides/101-samples/#best_practices_freeing) ([also](https://libgit2.github.com/libgit2/#HEAD/search/_free))

The problem: you must always remember to call `free_Mytype(x);`. If you forget, you have yourself merry little memory leak.


The thing about *this* C interface... is that you cannot use the typical `std::unique_ptr` in a convenient manner.
The reason is that the allocated storage is not returned from a function-call  
```cpp
// if it were returned from a function-call, it would be nice: 
std::unique_ptr<mytype, decltype(Deleter_Mytype())> x2(static_cast<mytype *>(alloc_Mytype(arg)), Deleter_Mytype());
```
... but instead returned with the argument-parameter,
as an lvalue with applied address-of operator (e.g. `&x`).

So the code in this repo shows a possible way of handling this with C++.
If you know of alternatives, or (particularly) if there is a standard C++ construct for this already (that I've missed), please let me know.

# A possible solution

## Preferred solution: RAII

We use a wrapper-class, that calls `alloc_Mytype()` during construction and `free_Mytype()` during destruction (e.g. when the wrapper-class goes out of scope).

```cpp
// use Uniq_Ptr_Raii_Func_Wrapper to create a wrapper-class MytypeRAII that 
// * handles allocation during construction via   alloc_Mytype(Mytype**, double)
// * handles freeing at destruction         via   free_Mytype (Mytype*)
class MytypeRAII : public Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(&alloc_Mytype), decltype(&free_Mytype), double> {
  using Base =            Uniq_Ptr_Raii_Func_Wrapper<Mytype, decltype(&alloc_Mytype), decltype(&free_Mytype), double>;
public:
  MytypeRAII(double x) : Base{&alloc_Mytype, &free_Mytype, x}
  {}
};

int main()
{
  // ...
  {
     double arg = 0.0;   // control something during allocation in the alloc_Mytype function coming up
  
     // RAII-approach
     MytypeRAII x2(arg); // call alloc_Mytype() during the constructor
     
     // use x2, e.g. the allocated pointer can be gotten via x2.get()
     // ...
     
     
     // x2 goes out of scope here. automatically calls free_Mytype!
  }
  // ...
}

```

## Non-preferred method: delayed-but-tracked RA (resource aquisition)

Here we have a wrapper-class that just handles deletion (during destruction, e.g. when the object goes out of scope).
It exposes a reference to a pointer (via member `get()`) that can be fed into allocation functions. 

```cpp
class MytypeRAII_delayed : public Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(&free_Mytype)> {
  using Base =                    Uniq_Ptr_Raii_delayed_Ra_Func_Wrapper<Mytype, decltype(&free_Mytype)>;
public:
  MytypeRAII_delayed(Mytype *p = nullptr) : Base{&free_Mytype, p}
  {}
};

int main()
{
  // ...
  {
     // delayed-but-tracked RA (Resource Aquisition)
     MytypeRAII_delayed mt3;
     alloc_Mytype(&(mt3.get()), 0.0); // RA now
                                      // get() is special here: it returns a reference(!) to a pointer
                                      // (Every interaction with mt3 will now check this pointer and track any changes)
     
     // use mt3
     // ...
     
     // mt3 goes out of scope here. automatically calls free_Mytype!
  }
  // ...
}
```

Note: As long as all interaction is via the wrapper-class, the wrapper-class will notice if the exposed pointer is changed (deleting the copy that is held by the base unique_ptr)
```
  {
     // delayed-but-tracked RA (Resource Aquisition)
     MytypeRAII_delayed mt3;
     alloc_Mytype(&(mt3.get()), 0.0); 
     
     // use mt3
     // ...
     
     alloc_Mytype(&(mt3.get()), 0.1); // Yes doing this again is safe! 
                                      // The pointer-change will be noticed. The previously allocated pointer is deleted appropriately
     
     // mt3 goes out of scope here. automatically calls free_Mytype!
  }
  // ...
```
