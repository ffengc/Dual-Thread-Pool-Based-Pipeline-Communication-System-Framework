

# 复用线程池对象

- **[简体中文](./reuse-the-threadpool-CN.md)**
- **[English](./reuse-the-threadpool.md)**

代码见 threadControl.hpp。

本对象本质就是一个线程池，首先，有非常多的优化方案，这个我在readme中也有提到。

## 项目可以优化的方向

我欢迎各位对我本项目进行持续的优化，如果要对本项目进行优化，大家可以联系我的邮箱（我的主页上有）之后，fork本仓库进行优化，如果我认为这个优化是ok的，我会进行merge操作的！

- connector线程目前只有一个，可以进行优化，connector线程通过参数进行设定。
- 可以优化使得connector线程结合多路转接技术，具体可以见我另一个仓库，[Multiplexing-high-performance-IO-server](https://github.com/Yufccode/Multiplexing-high-performance-IO-server)，同时监听多个文件描述符，这个文件描述符不止来自于管道，还可以来自于网络，来自于硬件等等，因此本项目是可以持续优化的！
- 优化线程池支持的数据类型，目前只支持string类型，因为目前来说，我还没有编写序列化，反序列化的代码，因此只支持string/char*类型。如果本项目要扩展到网络服务器上，或者其他地方，都需要编写反序列化和序列化的代码，维护一个Task类，Task类可以序列化反序列化。同时Task类可以重载operator()方法，表示Task任务的执行和调用。

## 这个线程池在干什么

**一句话总结：线程池维护一个缓存，缓存里面是数据/任务，worker线程和connector线程会基于这个缓存做不同的事情。**

在我的项目中，server端的worker就是生成数据，push到缓存中，connector从缓存中取数据！当然，worker和connector要做的事情是可以改的，是通过传参决定的！

## 如何复用

看这一部分之前还是先要看下[项目可以优化的方向](#项目可以优化的方向)，更容易理解复用的方法和原理。

调用threadControl对象。

```cpp
thread_control<std::string>* tc = new thread_control<std::string>(worker, connector, 3, fd, lambda_arg);
tc->run();
```

首先可以见到我编写的代码中server.cc，会传递一个worker和connector函数指针（当然你也可以修改代码封装成C++的函数对象），这个是这个线程池必须的参数，这两个参数表示worker和connector线程分别要做的事情，至于后面的参数，这个3表示是worker线程的数量（connector线程的数量现在是写死的，就是一个，你当然可以在这基础上优化），fd表示我的这个项目中管道文件的文件描述符（不是必须参数），lambda\_arg表示我这个项目中所需要的lambda参数（不是必须参数）。

你可以复用这个threadControl.hpp文件，让worker和connector做不同的事情，这就是复用的方法！