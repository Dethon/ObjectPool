# Object Pool
A header only library implementing a templated contiguous object-pool.

## Objective
This class provides an easy way to optimize repetitive object creation and destruction while also preventing memory fragmentation (which is usual in these kind of scenarios).

## Description
The `ObjecPool` class preallocates a number of objects so they can be reused within a loop without incurring the cost of allocation and deallocation. The pooled type can implement a `reset()` method to reinitialize the object before reusing it.
It also leverages `std::vector` to ensure everything stays contiguous in memory, making it as cache friendly as possible.
