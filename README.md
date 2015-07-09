# The problem: Inconveniences of C-code interfaces

Some C interfaces look like this:

```
mytype *x;
bla_init(&x); // allocate storage on the heap and set the address of that allocated storage as the value of pointer x
bla_free(x);  // use the value of pointer x, to locate the storage that is now freed
```

Here's a real-world example: [libgit2](https://libgit2.github.com/docs/guides/101-samples/#best_practices_freeing) ([also](https://libgit2.github.com/libgit2/#HEAD/search/_free))

The problem: you must always remember to call `bla_free(x);`

# A possible solution

```
{
  // RAII-approach
  Unique_ptr_Mytype x2;
  bla_init(&(x2.get()));
  // use x2
  
  // x2 goes out of scope here. automatically call bla_free!
}

```
The thing about *this* C interface... is that you cannot use the typical `std::unique_ptr` in a convenient manner.
The reason is that the allocated storage is not returned from a function-call  
```
// if it were returned from a function-call, it would be nice: 
std::unique_ptr<mytype, decltype(Deleter_Mytype())> x2(static_cast<mytype *>(bla_init()), Deleter_Mytype());
```
... but instead returned with the argument-parameter,
as an lvalue with applied address-of operator (e.g. `&x`).

So the code in this repo shows a possible way of handling this with C++.
If you know of alternatives, or (particularly) if there is a standard C++ construct for this already (that I've missed), please let me know.
