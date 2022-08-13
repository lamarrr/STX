

std::vector<T>                                                                          stx::Vec<T>, stx::FixedVec<T>

uses exceptions to report errors                                                        uses monadic result type to report errors
constructors are allowed to throw exceptions (which could significantly slow down your app)       doesn't support throwing constructors  
cannot use realloc for trivial relocation of its trivially-relocatable elements         uses realloc for trivial relocation of its trivially relocatable element, making it faster
provides guarantees for throwing constructors                                           doesn't provide guarantees for throwing constructors


uses result types and lets the user decide how to handle them