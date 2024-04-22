
//============================================================================
// Name        : client.cc
// Author      : 俞沣城
// Date        : 2024.4.10
// Description : 本项目中client进程client.cc编写
//                  client.cc中需要提供tc对象运行时的worker方法和connector方法
//============================================================================

#include "../Utils/comm.hpp"
#include "../Utils/threadControl.hpp"

void WriteToIpc(int fd, std::string msg) {
    // 2. ipc过程
    // 把数据写到管道中去
    std::string buffer = msg;
    write(fd, buffer.c_str(), buffer.size());
}

void* worker(void* args) {
    __thread_data* td = (__thread_data*)args;
    thread_control<std::string>* tc = (thread_control<std::string>*)td->__args;
    // 在这里构造Task
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> dist(tc->get_lambda()); // 这里用命令行传递过来的参数
    while (true) {
        double interval = dist(gen); // 生成符合负指数分布的随机数
        unsigned int sleepTime = static_cast<unsigned int>(std::floor(interval)); // 负指数
        sleep(sleepTime);
        // sleep(1);
        std::string msg = "hi I'm client, from thread: " + td->__name + "\n\tmy pid: " + std::to_string(getpid()) + "\n";
        std::cout << "client generate a mesg: " << msg << std::endl;
        lockGuard lockguard(tc->get_mutex());
        while (tc->is_full())
            tc->wait_full_cond();
        tc->get_queue().push(msg);
        pthread_cond_signal(tc->get_empty_cond()); // 唤醒connection线程，让他去取数据放到pipe里面
    }
    return nullptr;
}

void* connector(void* args) // 线程去调用，参数一定是void* args，如果不加statics，就多了个this
{
    // routine 本质就是消费过程
    __thread_data* td = (__thread_data*)args;
    thread_control<std::string>* tc = (thread_control<std::string>*)td->__args;
    // 通过这种方式，就可以让static方法调用类内的非staic属性了！
    size_t msg_number = 0;
    while (true) {
        // 1. lock
        // 2. while看看条件变量是否符合（队列中是否有任务）如果不符合，阻塞！
        // 3. 拿到任务
        // 4. unlock
        // 5. 处理任务: 如果是生产者->任务就是把东西放到20个slot的缓冲区里面
        //              如果是消费者->任务就是把东西从20个slot把东西拿出来并执行
        std::string msg;
        {
            // 在这个代码块中，线程是安全的
            // 这里用个代码块，lockGuard直接就帮我们解锁了
            lockGuard lockguard(tc->get_mutex());
            while (tc->is_empty()) {
                tc->wait_empty_cond();
            }
            // 读取任务
            msg = tc->get_task(); // 任务队列是共享的
            pthread_cond_signal(tc->get_full_cond());
            // 解锁了
        }
        WriteToIpc(tc->get_fd(), msg); // 把东西写到ipc中
        msg_number++;
        if (msg_number == MESG_NUMBER) {
            logMessage(NORMAL, "client quit\n");
            tc->__quit_signal = true;
        }
    }
    return nullptr;
}

void Usage() {
    std::cerr << "Usage: " << std::endl
              << "\t./client lambda" << std::endl;
}

int main(int argc, char** argv) {
    // 0. 计算运行时间
    auto start = std::chrono::high_resolution_clock::now();
    if (argc != 2) {
        Usage();
        exit(1);
    }
    // 0. 提取命令行参数
    double lambda_arg = std::stod(argv[1]);
    // 1. 获取管道文件
    int fd = open(ipcPath.c_str(), O_WRONLY); // 按照写的方式打开
    assert(fd >= 0);
    // 2. 构造并运行tc对象
    thread_control<std::string>* tc = new thread_control<std::string>(worker, connector, 3, fd, lambda_arg);
    tc->run();
    // 3. 关闭管道文件
    close(fd);
    // 4. 计算运行时间
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    sleep((unsigned int)WAIT_IO_DONE); // 统一睡眠相同时间，等待所有io执行完，再执行下面的io，避免打印的时候重叠了
    std::cout << "\t[client run time: ]" << elapsed.count() << "(s)" << std::endl;
    return 0;
}