
- Consider making constructor copyable
- or add map method

map
mep_err
copy
copy_err


stx_option_v3
stx_option_shared_v3


- remove all uses of std::invoke
- mandate that all uses of std::move be reachable by the compiler
- must not move from generic containers.
- must not move from generic types.
- templates must be limited.
- must not define operator overloads for the slightest ambiguity
- need a way to check binary compatibility of objects
- the present standard library as it is must go
- lifetime annotations
- memory typing annotations and requirements
- [[required_pointer_attributes(type = writable, lifetime = static, can_alias = false)]] void *




- re-add panic backtracing


  // static constexpr uint64_t ABI_TAG = STX_ABI_VERSION;
  //
  // we just need to be selective about what types we want to support across
  // ABIs
  //
  // complex objects usually don't cross ABIs.
  //
  //
  // Allocators and complex objects shouldn't decide the behavior of objects.
  //
  // enum class AbiTag
  //
  //
  // template<typename Output, AbiTag tag>
  // abi_cast( stx::Span<Output>  ) {
  // }
  //
  // define own abi cast by accessing internal members. must be vetted.
  //
  //
  



#
#
#
#
# Concepts on Steroids
#
#
#
# Fearless Low-Level Programming With Tag Attributes and User-Guided Compiler Reachability
#
#
# this will require that the compiler be able to perform full analysis wherever it is used
# but it can't be used with old code, instead, the compiler will tell you it is ambigous.
# must be enforced wherever it is used.
# the actual problem with C code is not being able to validate these assumptions and tell when these assumptions are violated.
#
# we introduced classes, but they don't work in practice to avoid these limitations.
#
#
# some of these constructs aren't necessary in themselves reasonable, but they are assumptions about hardware that necessisates some of these optimizations.
# pessimizing them and deeming them "safe" or "unsafe" is not helpful or useful to anyone.
#
# more and more of these optimizations will come with more new hardware.
#
#
# should work with concepts
#
#
# these are higher order types, even higher order to concepts
#
#
#
# when casted to untyped pointers it can't help you
#
#
#
# all attributes must be filled in
# - compiler flag to check and validate these assumptions
#
# pointer aliases need to be a thing and be programmable and not collide with normal C++ statements.
#
# attribute typed aliases
#
# i.e. using p = [[nodiscard]] int;
# using writable_memory = [[nodiscard]]
#
- [[required_pointer_attributes(readable | writable | readonly = true, uninitialized | initialized = true, object_lifetime = static | any, memory_lifetime = static | any,  can_alias = true | false)]] void *
# points_to = int
# uninitialized = true
#
#
# Opt In
#
#
#
# Compiler Lookup
#
#
# they don't always have to be typed. the compiler only has to raise a warning or error when it can't reach the tags.
#
#
#
# defualt type tags
#
#
#
#
#
#
# builtins
#
#
# values added to tags must be deducible at compile-time or forced onto them
#
# they need not be constexpr, they just need to be reachable
#
# // composability is needed
#
#
# namespace std{
#
# // can be made builtin whatever
#
#
#
# tag_alias static_ref(T);
# tag_alias static_readable_ref(T);
# tag_alias static_writable_ref(T);
#
#
# tag_alias static_string __builtin*;
#
#
#
#
#
# # aliases need conversion rules
# # aliases clearly need to be programmable at compile time
# # we need to be able to refer to other tags
# # tags must not be ambigous??? as in the case of memory
# # tags need to be re-usable
#
#
#
#
#
#  # tag_alias operate on specific type
#  # tag_alias operate on generic type
#  # any logic needs to be handled by the type system, SFINAE, concepts, etc.
#
#
#
# tagged pointers
# zero page protection on OS (but not embedded devices)
#
# ===========> similar ideas. clang's type annotations
#
#
#
#
# # unspecified because we can't require any ones that register to it be valid, sometimes there will be too much information to possibly type. and sometimes you just need a specific readable memry type, and the compiler would fill in the rest of the info and requirements.
# 
#
#
#
# ================== description of alias
# ================== error messages upon violation
# ================== formatting???????? tooo much????
# 
#
# [[expects()]] - hard requirement that stops compilation
# [[requires()]] - soft requirement that stops compilation
#
#
#
#
#
#
# ========= arguments must not repeat but can be added in any order
# ==========> only evaluatable in [[expects()]] or [[recommends()]] blocks
# ========> unspecified needs to be a default value and any tag using it 
# ==========> must not work with macros or templates???
#
# =====> builtin types: lifetime = static, unspecified
# =====>      initialized
# -------> tags need context specific meaning
# 
#
# super-types: type,
#
#
#
# =====> must be a specific type or a generic type
#
#
#
# throwing whilst holding a lock
#
# 
# tag memory (
# initialized = default: "unspecified" | bool,
# initialized_with = default: "unspecified" | typename,
# lifetime = default: "unspecified" | "static" | "any",  
# readable = default: "unspecified" | bool,
# writable = default: "unspecified" | bool,
# size_bytes = default: "unspecified" | size_t,
# alignment = default: "unspecified" | size_t,
# of_type = default: "unspecified" | typename,
# )
# expectations
# {
#
#  // compiler must reach the expressions
#  requires(alignment > 0, "...");
#  requires(size_bytes > 0, "...");
#  requires(!is_void<type>, "...");
#
#
#  // compiler should raise a warning that it can't reach the expression, so it can't tell
#  recommends(alignment > 0, "...");
#  recommends(size_bytes > 0, "...");
#
#  requires(of_type.has_tag(__builtin_tag__memory));
# 
#
#  alias __builtin_tag__memory(.initialized = initialized, .initialized_with = initialized_with);
#  alias ...
#  alias ...
# }
#
# conversion_rules
#
# {
#
#
#
#
#
# }
# 
#
#
#
# // must be constructed here if multiple of them are needed
#
#
# void fn( [[expects(memory, void)]] void * fn    );
#
#
#
#
# // must never be re-defined
# // must be defined in the same translation unit it was given
# define_tag memory = __builtin_tag__memory(initialized = memory::initialized, ...);
#
#
#
# define_tag nothrow = __builtin_tag__nothrow();
#
#
# declare_tag throws(will_throw =  "unspecified" | expression;
#
# define_tag throws = __builtin_tag__throws();
# 
#
# declare_tag a ("unspecified" | constexpr_type, lifetime: "static" | any,     )
#
#
#
#
# tag_alias nothrow  builtin*;
# tag_alias throws builtin*;
# tag_alias 
#
#
#
# TODO(lamarr) tag alias conversion rules
# // unique_ptr requires it to not be moved-from
#
#
# void* without this tag is compatible if the compiler can automatically detect it,
#  
#
#
# thread_safety
#
# // const is a tag that can be used to check
# // runs on thread must require functions 
# // state safety
#
#
#
# tag_alias runs_on_thread ...;
#
# tag_alias lock; 
# tag_alias holds_lock;
# tag_alias releases_lock;
#
#
#
# // bit banging, bit patterns, max shifts, invalid patterns, etc.
#
# };
#
#
#
#
# ADVANTAGE: for all places where these are used, the compiler will always guide you.
#
#
# namespace my{
#
#
#
# // they will and can be expanded to booleans just like concepts
# // with if-constexpr the compiler can always reach them
#
# declare_tag bit_assignable;
# declare_tag bit_constructible;
# declare_tag bit_movable;
#
#
# declare_tag v1_abi_tag;
#
# declare ABI compatibility rules for 2 versions
#
#
#
#
# [[add_tag()]], [[remove_tag()]] - shall not be used in templates, they need to be reachable, templates will hide them
#
# [[add_tag()]] - must be on type declarations only.
#
#
#
#
# add_tag(struct Y, bit_assignable, bit_constructible, bit_movable) {
#
#
#
#  add_tag(...)
#
#
#
# };
#
#
#
# template<typename T>
# using bitable_t = add_tag(T, bit_assignable, bit_constructible); // same type, but now with compiler guidance
#
#
# template<typename T, get_tags(T)>
# concept relocatable = [[T]]::has_tag(bit_assignable) && [[T]]::has_tag(bit_constructible);
#
#
#
#
# =========get_tag()
#
# template<typename U, typename Y>
# concept abi_compatible =  has_tag(U, my::v1_abi_tag) && has_tag(Y, my::v1_abi_tag) && type_is_same(U, Y);
# belongs_to_namespace(std)
#
# template<typename U>
# std2::string& as_v2_string(  U & v1_string){
#
#
# }
#
#
# template<typename T>
# void set(T)  expects(required, relocatable<T>) {
#    
# }
#
#
## template<typename T>
# void set(T)  expects(recommended, relocatable<T>) {
#    
# }
#
#
#
#
# };
#
#
# [[expects(required, expectation)]]
# [[expects(recommended, expectation)]]
#
#
# [[expects(  )]]
#
#
#
#
# template<[[required_tag(...)]]  typename T>
# [[add_tag(bit_assignable)]] BitSet{
#
# };
#
#
#
#
#
#
#
# SOLVES ABI, almost
#
#
# version must be linearly increasing
# functions that accept other versions must programattically check using tag programming
#
#
#
# tag(abi_version = 3495)
#
#
#
# converting objects across abis
#
#
# vendors will have to provide abi convertors and comparators to programatically declare these assumptions.
#
# std2::span<int>& to_v2_abi(std::span<int> & a){
# // i.e. if data layout of span didn't change
#   return reinterpret_cast<  std2::span<int> &  >(a);
# }
#
#
# higher-level tags and tagged based programming
#
#
#
# template<[[expect(has_any_tag() )]]  typename T>
# void same_address(std2::span<int>, std::span  )
#
#
# tag programming using integral types, must have overflow checks
#
#
#
#
#
# - [[recommended_tags(...)]] - raise warning if any of the required tags (requirements) aren't met, i.e. the compiler can't reach them.
# - [[required_tags(...)]] - abort compilation if any of the required tags (requirements) aren't met, i.e. the compiler can't reach them.
# - [[required_pointer_tags(...)]], - must all be filled, pointers are extremely ambiguous.
# - [[recommended_pointer_tags(...)]]
# - [[required_object_tags(...)]], [[recommended_object_tags(...)]] - must all be filled with compiler defined values, i.e. requirements from the C++ object model
# - [[required_memory_tags(...)]] - must all be filled with compiler defined values, i.e.
# - [[add_tag(...)]]
# - [[remove_tag()]]
# - 
#
#
#
# this helps isolate unintended behaviour or non-validated assumptions both in low-level code and high-level code.
#
#
# - [[tag(...)]] - add tags, the compiler should be able to list all places where this occured
# - [[untag(...)]] - remove tags, the compiler should be able to list all places where this occured
# - [[expect_tag(...)]] used anywhere
#       - recommended: compiler should raise a warning if it can't tell. syntex: [[tag]]
#       - required: compiler should abort compilation if it can't tell.
# - if constexpr([[has_any_tag(...)]] value){   }
# - has_any_tag || has_any_tag
# - has_all_tag || has_all_tag
#
#
# tags for C++ object-based operations (the compiler knows): 
# - abi_version
# - constructed
# - not_moved
# - not_copied
# - not_move_constructed_from
# - not_move_constructed_to
# - not_move_assigned_from
# - not_move_assigned_to
# - not_default_constructed
# - copied
# - copied_from
# - moved
# - moved_from
# - uninitialized - i.e. has no default constructor and its objects are integral types
# - byte_assignable
# - byte_readable - valid for all C++ objects
# - trivially_relocatable
# - copy_constructible
# - copy_assignable
# - is_integral
# - is_trivial
# - must_call(function)
# - belongs_to_namespace(T, std::) 
# - returned_from(std2::allocate)
# - must_return_to(std2::deallocate)
#
#
#
#
# tags for C++ functions:
# - abi_version
#
#
# tags for memory-based operations:
# - initialized
# - initialized_with
# - bit_assignable
# - readable
# - writable
# - 
#
# tags for
#
#
# tags for aliasing
#
#
# tags for concepts:
#
# tags for constexpr bool
#
# tagged aliases:
#
# using valid_memory = [[recommended_pointer_tags(...)]] void*;
# using object_placed_memory = [[recommended_pointer_tags(...)]] void *;
#
# delegating responsibilities and state maintenance to types isn't enough. there will always be assumptions and validating them using the type system will lead to unnecessary code duplication
#
#
#
# - functions can add tags and be deemed safe, this makes it easy to spot and isolate functions that don't know what they are doing
# - compiler utility: list all functions adding tags of a type
# - far better than unsafe
#
#
# static_assert and templates aren't enough, they eventually lead to code complexity when overused and you end up resorting to C code
#
# tag aliases and adding tags:
#
#
#
# adding tags
#
# tag(void*, /* pointer-only tags */ ) allocate()
#
# // removing tags
#
#
# untag(void*, /* pointer-only tags ....  */) deallocate()
#
#
# template<typename T>
# detag valid_object T;
#
#
# compiler defined tags:
#
#
# lifetime:
#
# - 
# - memory_lifetime - static | any
#
# example:
#
#
# we use the type system to disambiguate these, but it is not enough.
# we need the compiler. we can instruct the compiler to do these for us.
#
#
# TODO(lamarrr): create an higher-order type
# 
# compilers should bd able to list all places where tags were removed
#
#
# typing and enforcing requirements in C++ presently requires storage and template metaprogramming ninjutsu, we need a way to enforce requirements without 
#
#
# "don't use pointers" is not a valid criticism or anywhere useful to anyone. 
#
#
# how about:
#
# - embedded systems folks that have to 
# - operating systems folks who explicitly manage memory in bytes and often make assumptions to optimize memory management.
# - game and browser developers: who continuosly develop new ways to manage memory and optimize their applications and reap the last bit of performance from their hardware
#
# - compilers already check these assumptions for us but they aren't enforced enough.
#
#
# these are often the mainstream systems that are subject to vulnerabilities
#
#
# operating systems provide great optimizations defined by the POSIX API, but they'll bite you if any of their requirements aren't met
#
#
#
# namespace std2{
# namespace posix{
#    
#
#
#
#
# // need static tag declaration method
# // the c++ standard also needs tags that it adds and detects. i.e. objects must not be initialized on invalid memory
# // need a way to force tags on methods, i.e. static signatures
#
#
# //
#
# using allocated_memory = [[tags(aliases = false, writable = true, uninitialized = true, lifetime = runtime, returned_from(std2::posix::allocate)     )]]
#
#
#
#
# std2::optional<allocated_memory> allocate(size_t size)
# void deallocate(allocated_memory memory);
#
# }
# }
#
#
#
#
#
# # Undefined Behavior is beautiful, but we need to isolate them and validate our assumptions.
#
# they aren't inherently bad but they can help reap a lot of optimizations from code often deemed # "unsafe"
#
#
#
#
#
#
#
#
#
#
#
#
#
- [[pointer_attributes(readonly, writable, lifetime)]]



# Span methods

map_to
any_of
all_of
none_of
apply
reverse
find
find_if
partition // remove_if // filter
sort
max
min
max_with
min_with


 

struct any_memory_handle {
  void* handle = nullptr;
};

struct readonly_memory_handle {
  void const* handle = nullptr;
};

struct writable_memory_handle {
  void const* handle = nullptr;
};

struct readonly_memory {};

struct writable_memory {
  void* handle;
};

struct any_memory {};

struct static_string {
  readonly_memory memory;
  static_string(char const* handl) static_string()
};
