

# Reuse thread pool objects

- **[简体中文](./reuse-the-threadpool-CN.md)**
- **[English](./reuse-the-threadpool.md)**

See threadControl.hpp for the code.

This object is essentially a thread pool. First of all, there are many optimization solutions, which I also mentioned in the readme.

## Directions in which the project can be optimized

I welcome everyone to continue optimizing my project. If you want to optimize this project, you can contact my email address (available on my homepage) and then fork this warehouse for optimization. If I think this optimization is OK, I The merge operation will be performed!

- There is currently only one connector thread, which can be optimized. The connector thread is set through parameters.
- The connector thread can be optimized to combine with multiplexing technology. For details, please see my other warehouse, [Multiplexing-high-performance-IO-server](https://github.com/Yufccode/Multiplexing-high-performance-IO -server), monitor multiple file descriptors at the same time. This file descriptor not only comes from pipes, but also from the network, from hardware, etc., so this project can be continuously optimized!
- Optimize the data types supported by the thread pool. Currently, only string types are supported. Because at present, I have not written serialization and deserialization code, so only string/char* types are supported. If this project is to be expanded to a network server or other places, it is necessary to write deserialization and serialization code and maintain a Task class, which can be serialized and deserialized. At the same time, the Task class can overload the operator() method to represent the execution and invocation of the Task task.

## What is this thread pool doing?

**Summary in one sentence: The thread pool maintains a cache, which contains data/tasks. The worker thread and the connector thread will do different things based on this cache. **

In my project, the worker on the server side generates data, pushes it to the cache, and the connector fetches data from the cache! Of course, what workers and connectors want to do can be changed and is determined by passing parameters!

## 如何复用

Before reading this part, you should first look at [Directions in which the project can be optimized] (#Directions in which the project can be optimized) to make it easier to understand the methods and principles of reuse.

Call the threadControl object.

```cpp
thread_control<std::string>* tc = new thread_control<std::string>(worker, connector, 3, fd, lambda_arg);
tc->run();
```

First of all, you can see that server.cc in the code I wrote will pass a worker and connector function pointer (of course you can also modify the code to encapsulate it into a C++ function object). This is a necessary parameter for this thread pool. These two parameters represent What the worker and connector threads have to do respectively. As for the following parameters, this 3 represents the number of worker threads (the number of connector threads is now hard-coded, it is just one. Of course you can optimize on this basis), fd means I The file descriptor of the pipeline file in this project (not a required parameter), lambda\_arg represents the lambda parameter required in my project (not a required parameter).

You can reuse this threadControl.hpp file and let workers and connectors do different things. This is the reuse method!