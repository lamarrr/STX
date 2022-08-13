

std::future<T>              stx::Future<T>

supports exceptions         no support for exceptions, use result types.
does not support, cancelation, suspension, nor resumption       support cancelation,suspension,and resumption
only reports the value or exception being set.     reports intermediary states in addition to the value being finally set. i.e. Scheduled, Executing, Completing, Resuming














https://devblogs.microsoft.com/oldnewthing/20210707-00/?p=105417


You get little to no benefit in making small non-blocking tasks async
don't use async as state machines, they are heavy and tasking.


ROBUST ERROR HANDLING, TRANSMISSION AND MANAGEMENT

What to make Async