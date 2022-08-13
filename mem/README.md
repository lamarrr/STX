




# you don't pay for function indirection if the compiler can deduce the type of the allocator
# no static/default allocator


# problems with implicit memory allocations
cache thrashing
makes a syscall
fallible and can crash program if failed
you have no idea when ands where it happens
takes time