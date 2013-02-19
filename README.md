libmoost
========

*libmoost* is a collection of C++ utility libraries, including:

* algorithms (set intersection, hashing, variable length encoding, ...)
* abstractions for compiler specific features
* configuration
* container data structures (e.g. LRU cache, memory mapped files)
* message digests
* smart pointers (in addition to boost smart pointers)
* I/O helpers (e.g. async writer, file change watcher)
* key-value store client wrappers for kyoto tycoon, bdb, etc.
* logging
* template metaprogramming
* a stomp message queue client
* object-oriented shared object loading
* service framework (helpers for daemonisation, remote shell access, monitoring, etc.)
* progress bars
* postgres pgq abstraction
* signal handling
* string functions
* unit test support
* threading
* transaction handling
* timers
* xml parser
* complex data structure stringification
* benchmarking
* C++ name demangling
* and more!

Installation
------------

*libmoost* requires a recent C++ compiler (g++, clang++) and boost-1.42 or (ideally) higher. It uses mirbuild as the build system. Once the dependencies are satisfied, you can build and test *libmoost* with

  ./build.py test

and install it using:

  sudo ./build.py install
