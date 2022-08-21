

|std::string  |  stx::String |  
|----------------------------------------------|-------------------------------------------------------|
| can be null-terminated | always null-terminated|
| char elements are mutable | char elements are not mutable since they can live on flash (static storage duration)
| requires allocation to store static storage strings | requires zero allocation to store static storage strings|
| templated | non-templated |
| implicitly handles constructor arguments | makes use of helper functions e.g. ```stx::string::make_static("Hello, World")```|


String optimizes especially for the case where the strings reside in your binary and you wouldn't need to allocate memory and copy them, as done in std::string. Static storage strings are read-only.
