
cacheline bouncing and efficiency guarantees
no exceptions whatsover, meaning we can go insane on lockfree
await is cool but suspension points are somewhat vague and it can be difficult to reason about
the costs of each program depends on program structure and isn't introspectible
Functions that await other functions must be async
not decomposable
you have to rewrite interfaces
reduce cache misses and memory hopping, improve cache locality
custom allocators and embedded systems support
No locks whatsoever
little to no inter-thread communication
introspectable
NO CALLLLLBACCCKKSSS OR COMPLETION HANNNDDLLERRRSSSS
FAST, EXTREMELY FAST

lock-free: the CPU isn't doing any useful work when performing blocking locks
Lock-free or go home

Support for multi-pronged executorss

Callback process:

load address of function pointer.
load address of data for function pointer to operate on.
execute the function of the pointer on the data it points to.
load address of the promise
notify of beginning modification so other threads do not write to the storage twice
store the computation result in the promise
notify of completion
repeat

all performed in one cache line, little to no bouncing depending on what code the function pointer executes
all compactly packed into the same memory address



uses one allocator.


CAVEAT: no support for exceptions. can't panic, abort, or exit on the main thread.


lean, introspectible, no callbacks. Extremely simple (in runtime and space)

performance and usability is key. generalization is only a bonus



No generalization, focused on functionality, predicatability (time and space), and extensibility with little to no cost


***********DETERMINISM************
efficiency: cache-line reuse tightly packed guarantee

walkthrough of C++ 20's coroutines and asyncrhony


Plexers/Forwarders


