
 
custom allocator support
trivial
no lifetime
no null
no exceptions (i.e. bad function call, fn is always valid and non-null)


std::function<Return(Args...)>                                                                            stx::Fn<Return(Args...)>

allocates memory and dictates the lifetime of the functor                                                only references already allocated functor. only constraint is that it should                                                                                                   not be called after the referenced functor/function pointer is destroyed
can not be called directly from a C interface that receives a callback                                    can be called directly from a C interface that receives a callback [void* data, int(*callback)(void* data, int)], in other words it is equal to the callback(void * arg, ...) callback paradigm found in C.
acts as a container of the functor                                                                        acts as a view over functors                  1   
has implicit deep copy constructors                                                                       has a copy constructor that only copies the view, not the data (i.e. struct or lambda data)
can contain a null function which is checked and once it does                                            Fn is always non-null, so no checks are made, it thus has no branching
contain a null funcyion, an exception it throws an exception (std::bad_function_call?)

#todo(lamarrr): finishcheckedchecked