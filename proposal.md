# C is Safe by Design

***DISCLAIMER***: Your criticisms to this article is invalid till you write a constructive one to prove otherwise. Neither am I saying the "safety" issues that plague the industry is invalid. Also, Twitter is to short to convery thoughts so I thought I'd make a repository.


I believe it's an inadequate understanding of the Systems that makes people say things like C is "unsafe", "don't use it", or the non-constructive criticisms to projects and hardware vendors writing useful software, like "have you considered writing this in rust?"


C is safe by design. The problem is poorly validated assumptions and contracts not clearly expressed nor implementable by the type system. and no, Rust only patches this problem by marking code as "safe" or "unsafe" which still doesn't solve the abstraction and constract problem.
"safety" is contextual, and the type system of Rust or any language doesn't solve this yet, and neither can you share types across ABIs "safely".


An entangled Abstraction Mess that is difficult to explain without looking like you are insane.


Safety By Abstraction.

Why an Operating System?

I need:

# User
The user is the lowest level of abstraction in this design, and it is what the abstractions are usually centered around.

I as a user needs:

- Something to manage my hardware
- Something to run programs whilst managing my hardware effectively without wasting it (I mean, I bought my hardware)
- [[limitation:1]] I am no scientist, only human, I don't want to be manipulating electron states myself.

What we need is what we call the "Operating System"


Needs from the Operating System:
- run as smooth as possible, nobody likes:
        - Electron Apps swallowing my hardware resources (memory whilst I'm doing nothing, on my poor 4GB of RAM)
        - Games running at 30FPS
- I don't want to use electrons or bang electrons on my computer, I know nothing about that

[contract::1] I need to delegate responsibilities to someone we call an OS programmer that will effectively manage my hardware resources.
[[contract::1::implication::1]] We also have a group of users called "companies" that will use this operating system. I as an
individual user might not care about speed, but "companies" as a user will always go for the fastest hardware for my specific use-cases. so my operating system better be as fast as possible. [[contract::1::implication::1::citing]] example of this can be seen in Linux and Windows. that is why Linux is deemed faster than Windows and more popularly used for servers since it is often considered to consume less resources than the Windows counterpart. Linux can scale even to embedded systems and SBCs, i.e. Raspberry Pi 4, Raspberry Pi Micro, BeagleBone, etc.
[[contract::1::implication::1]] Why do "companies" do this? handling millions of devices at scale, i.e. servers is no small talk. you'd pay a lot of money to maintain them. If it takes Windows a second less than Linux to execute a program, I as an individual user might not care. but at scale of a million devices, that is 1 million x cost of electricitiy per second






Before you go bezerk, Let's build an hypothetical compiler and operating system from ground up using a "safe" language to see what I mean, it could be Python, Rust, Swift, Whatever.




 Modern OS's has one critical thing to function:
  - TLB caches
 
 And TLB caches are **abstractions** over Storage Devices:
 (sorted in speed order)
 
 - RAM[1]:
  ***[1:1]*** **RAM** is an abstraction over ***electron states*** and manipulation of the appropriate electron states will give you the desired effects on the RAM device, i.e. storing data. manipulation of these electron states is performed via another abstraction called **hardware registers**
  
 
 ## Why RAM?
  RAM is faster than 
  
  
  ## Delegation of Responsibility
  
 ***[1:2]*** Of course as an OS programmer, I'm not a scientist and neither do I care about electron states, and neither do I have the expertise to do that. I'll delegate that responsibility to *RAM* vendors. I just need an **abstract** concept of somewhere to store bytes and very fast.
 
 => Contract Signed With Me: Hardware Vendors <===> Operating System Authors
 
 # RAM Design
 
 
 ## Uses
 To store bytes temporarily. Is this "safe" Yes, by design.
 
 
 ## Sourcing
 RAMs are typically provided by hardware vendors, you buy it from one.
 
 ## Unifying
 Why not a single RAM type? Well, 
 - Hardware vendors provide what we call drivers to different operating systems to manipulate these registers.
  Implications: RAM drivers need
 - Hardware vendors also 
 
 
 # RAM Design Implications from the ... abstraction
 
 ***[1:2]*** once electricity to the **RAM** is detached, the "electron states" reset and you can't recover the electron's state anymore.
 
 
 
 - Hard Disks:
 - Flash Drives:




Designing our Hypothetical TLB cache
TLB caches exists to map processes' "memory addresses" to RAM or 





