std::span<T>                    stx::Span<T>

not available till C++20        available from C++17
has no helper methods           has helper methods .find(...), .transform(...), .fill(...), etc. which all abstract the algorithms library.
is intended as a separate class in the std.         it is intended as a static interface that linear access containers provide. i.e. stx::String, stx::Vec<T>
