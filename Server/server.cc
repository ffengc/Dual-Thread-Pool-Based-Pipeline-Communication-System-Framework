
//============================================================================
// Name        : Fengcheng Yu
// Author      : 俞沣城
// Date        : 2024.4.10
// Description : 本项目中server进程server.cc编写
//                  server.cc中需要提供tc对象运行时的worker方法和connector方法
//============================================================================

#include "../Utils/comm.hpp"
#include "../Utils/threadControl.hpp"

std::string ReadFromIpc(int fd) {
    while (1) {
        // 3. 开始通信
        // 读文件
        char buffer[SIZE];
        // while (true)
        // {
        memset(buffer, '\0', sizeof(buffer)); // 先把读取的缓冲区设置为0
        ssize_t s = read(fd, buffer, sizeof(buffer) - 1);
        // 最好不要让缓冲区写满，因为没有\0
        if (s > 0) {
            return std::string(buffer);
        } else if (s == 0) {
            logMessage(NORMAL, "client quit, I quit too\n");
            return std::string("quit");
        }
    }
}

// 这里的worker负责在slot里面找msg，然后打印出来
void* worker(void* args) {
    __thread_data* td = (__thread_data*)args;
    thread_control<std::string>* tc = (thread_control<std::string>*)td->__args;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::exponential_distribution<> dist(tc->get_lambda());
    while (true) {
        double interval = dist(gen); // 生成符合负指数分布的随机数
        unsigned int sleepTime = static_cast<unsigned int>(std::floor(interval)); // 负指数
        sleep(sleepTime);
        // 去缓冲区里面获取数据并打印
        std::string msg;
        {
            // 在这个代码块中，线程是安全的
            // 这里用个代码块，lockGuard直接就帮我们解锁了
            lockGuard lockguard(tc->get_mutex());
            while (tc->is_empty())
                tc->wait_empty_cond();
            // 读取任务
            logMessage(DEBUG, "queue size: %d\n", tc->get_queue().size()); // 打印一下队列的数据个数
            msg = tc->get_task(); // 任务队列是共享的
            pthread_cond_signal(tc->get_full_cond()); // 唤醒connector去继续到pipe里面拿数据，因为我刚拿走一个，空出来位置了
            if ((msg == "quit" || tc->__quit_signal == true) && tc->get_queue().size() == 0) // 起码要把队列的数据打印完
            {
                tc->__quit_signal = true; // 控制退出
                break;
            }
            std::cout << "client# " << msg << std::endl; // 打印这个消息
        }
    }
    return nullptr;
}

// 这里的connector负责在管道中读取数据
void* connector(void* args) {
    // routine 本质就是消费过程
    __thread_data* td = (__thread_data*)args;
    thread_control<std::string>* tc = (thread_control<std::string>*)td->__args;
    while (true) {
        std::string msg = ReadFromIpc(tc->get_fd());
        // 把这个msg写到缓冲区里面去
        lockGuard lockguard(tc->get_mutex());
        if (msg == "quit" || tc->__quit_signal == true) {
            // 不要再push到缓冲区中了
            // 但是也不要继续循环了
            logMessage(NORMAL, "server connector quit\n");
            // 但是需要最后push一次，把quit给push进去，告诉worker要结束了
            tc->get_queue().push(msg);
            pthread_cond_signal(tc->get_empty_cond()); // 唤醒worker线程，让他去取数据并打印
            break;
        }
        while (tc->is_full())
            tc->wait_full_cond();
        tc->get_queue().push(msg);
        pthread_cond_signal(tc->get_empty_cond()); // 唤醒worker线程，让他去取数据并打印
    }
    return nullptr;
}

void Usage() {
    std::cerr << "Usage: " << std::endl
              << "\t./server lambda" << std::endl;
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
    // 1. 创建管道文件
    if (mkfifo(ipcPath.c_str(), MODE) < 0) {
        // 创建管道文件失败
        perror("mkfifo");
        exit(-1);
    }
    // 2. 正常的文件操作(只读的方式打开管道文件)
    int fd = open(ipcPath.c_str(), O_RDONLY);
    assert(fd >= 0); // 文件打开失败
    // 3. 管道文件fd传给tc对象
    thread_control<std::string>* tc = new thread_control<std::string>(worker, connector, 3, fd, lambda_arg);
    tc->run();
    // 4. 关闭管道文件
    close(fd);
    // 5. 删除管道文件
    unlink("../Utils/fifo.ipc");
    // 6. 计算时间
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    sleep((unsigned int)WAIT_IO_DONE); // 统一睡眠，等待所有io执行完，再执行下面的io，避免打印的时候重叠了
    std::cout << "\t[server run time: ]" << elapsed.count() << "(s)" << std::endl;
    return 0;
}