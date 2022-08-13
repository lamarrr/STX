# Resource Management Library



//
// I often find myself needing to group resources and avoiding needless
// allocations when I'm certain the resource will both be valid.
//
//
// I need something like: shared<Resource>
//
// i.e. a curl http client:
//
// struct Resource{
//  Parent * parent; // parent handle
//  Child * child; // child handle
// };
//
// Requirement: child must be deleted before parent.
//
// I'd still have an allocation for the `Resource` struct or the individual
// objects no matter what approach I take with `shared_ptr`. (aliasing
// constructors, custom deleters, etc ).
//
//
// Approach one: two shared_ptrs
//
// struct Resource{
//  shared_ptr<Parent> parent{create_parent(), make_parent_deleter()};
//  shared_ptr<Child> child{parent->create_child(), make_child_deleter()};
// };
//
// Problem with approach: two control block allocations, and we have to make
// sure the child is deleted before the parent, therefore we need to enforce
// that the child is placed after parent. and we'd still need to make sure the
// refcount of the child does not exceed that of the parent.
//
// Approach two: aliasing constructors? No way!
//
// struct Resource{
// shared_ptr<Parent> parent{create_parent(),  make_parent_deleter() };
// shared_ptr<Child> child{parent, create_child(), child_deleter() ???????? };
// };
//
// Problem with approach: two control blocks will still be needed due to
// the new deleter, shared_ptr does not support this pattern
//
//
//
//
//
//
//
//
//
//
//
//
//
// // one control block, one
// allocates a control block struct
//
//
// CurlEasyHandle{
//  CURL * easy;
//  shared_ptr<CURLM> multi;
// };
//
// shared_ptr<CurlEasyHandle> easy { new CurlEasyHandle{curl_easy_init(),
//              multi}, CurlEasyHandle::Deleter{}  };  // allocates memory for
//              the handle
// struct and allocates another control block + intentional ref-count
//
//
//
//
//
//
//
//
//
// I can't reasonably do this using `shared_ptr` since I'd have two allocations
// for two control blocks plus one ref-count plus one handle struct allocation
// .i.e.
//
// Intuitively, all I really need here is just one control block and one deleter
// along with the `ClientHandle` struct. that's not really possible with
// shared_ptr as it assumes the resource will always be a pointer just as its
// name implies.
//
// A number of APIs also use integers or structs to represent resource handles.
// i.e. Vulkan's Queue Families, Vulkan's Descriptor sets, OpenGL's horrible
// integer handles and many others.
//
//
//
//
//
//
//
//
//
//
// child can be created from parent at any point in time and it is not created
// along with the parent. and the child needs to be deleted before the parent is
// deleted. we'd somehow need to refcount the parent along with the child.
//
//
//
//
//
//
//
//
//


Standard library is too general where it need not be.

Sharing
mostly used for sharing resources where the cost of sharing is lower than the cost of re-creating them. also when the same object is not guaranteed to be gotten once lost.


# you'd typically need to allocate memory for the device handle and all other of its properties and attributes


# "a workaround it isn't hard to implement" is not an excuse  for poor abstractions

shared_ptr makes lifetime event-oriented???

# resource sharing has nothing to do with allocation or memory, they are two independent and intertwined concepts

OOP gives the illusion that we are operating on objects, meanwhile we are only operating on a virtual representation of them / these concepts, they are represented as bytes and operations to be performed on them.

Objects are often bytes


# if you think about it, the pointer isn't the resource, it is the device. and what we are trying to do is to keep the device alive until we don't need it any more.



asynchrony/concurrency is a function of operations not of values nor of objects

