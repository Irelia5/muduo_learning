# 生产者消费者模式
[TOC]

生产者消费者模式的由来：在操作系统中，进程之间是并发执行的，不同的进程间存在不同的制约关系。当进程之间共享资源，进程间的协同就至关重要。多线程同时占有资源可能造成冲突，因此规定同步和互斥对进程进行制约，生产者消费者模式就是一个经典的例子。

生产者和消费者有一块共享空间，由生产者向其中放入产品，消费者从其中取出产品。然而这样简单的行为却会引起冲突，想象一下，厨师还没做出菜，然而盘子却被顾客端走了；厨师往盘子里放了一半菜，盘子又被顾客端走了。于是我们做出规定：
* 厨师先放，顾客再端
* 盘子一次只能一个人用（厨师放，顾客端）

这个例子中，厨师和顾客视为不同的进程，盘子是临界资源；厨师和顾客之间先后是同步关系；厨师和顾客使用盘子是互斥关系。

## 临界资源
多个进程可以共享系统中的各种资源，但其中许多资源一次只能被一个进程所使用，这样的资源称为临界资源。
对于临界资源的访问，必须互斥进行。
每个进程中，访问临界资源的那段代码叫临界区。

## 同步
同步称直接制约关系，进程间协调工作次序，进行合作。

## 互斥
互斥称间接制约关系，当一个进程进入临界区使用临界资源，另一个进程必须等待，当占用临界资源的进程退出临界区，另一个进程才能去访问临界资源。

## 信号量
信号量可用来解决同步互斥问题，只能被两个标准原语wait和signal访问，也成为P操作和V操作。
原语是指完成某种功能且不被分割不被中断执行的操作序列，有时也称为原子操作，通常可用硬件来实现完成某种功能的不可分割执行特性。

## 分析手法
遇到这样的问题，首先找出进程数，并且分析出他们之间的同步互斥关系。
确定同步互斥关系之后设定信号量和初值。
完成PV操作。

## 伪码
### C
C主要使用`pthread_t`来完成，包括创建线程，`join`线程，销毁线程
线程锁，线程条件变量等。
#### 信号量
使用信号量pv操作实现生产者消费者——分析同步互斥关系:
生产者和消费者是同步关系，两者均互斥使用空间。
同步变量：
> full = 0    
empty = N

互斥变量；
>mutex

```cpp
void preducer
{
    p(empty);
    mutex.lock();
    //add
    mutex.unlock();
    v(full);
}

void consumer
{
    p(full);
    mutex.lock();
    //reduce
    mutex.unlock();
    v(empty);
}
```
#### 条件变量
互斥变量 
>mutex

条件变量 
>cond

```cpp
void preduce 
{
    mutex.lock();
    while (full == N) {
        cond.wait(&mutex);
    }
    //add
    mutex.unlock();
}
void consumer 
{
    mutex.lock();
    while (empty == N) {
        cond.wait(&mutex);
    }
    //reduce
    mutex.unlock();
}
```
### CPP
C++11多线程有了一定更新，包括
`std::thread`
`std::mutex`
`std::condition_variable`
`std::future`
`std::atomic`

对于条件变量的使用，有两种方式，一种是类似C的
```cpp
while (...) {   //虚假唤醒
    cond.wait(mutex);
}
```
一种是
当函数返回false阻塞解锁，被唤醒且返回true解除阻塞上锁，和之前的while等价。
```cpp
cond.wait(mutex, [] { return !(...); });
```


```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex g_mutex;
std::condition_variable g_producer, g_consumer;
std::queue<int> g_queue;
const int g_size = 20;

void producer(int id)
{
    while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        std::unique_lock<std::mutex> lock(g_mutex);
        g_producer.wait(lock, [] { return g_queue.size() != g_size; });
        std::cout << "Producer " << std::this_thread::get_id() << std::endl;
        g_queue.push(id);
        std::cout << g_queue.size() << std::endl;
        g_consumer.notify_all();
    }
}

void consumer()
{
    while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        std::unique_lock<std::mutex> lock(g_mutex);
        g_consumer.wait(lock, [] { return g_queue.size() != 0; });
        std::cout << "Consumer " << std::this_thread::get_id() << std::endl;
        g_queue.pop();
        std::cout << g_queue.size() << std::endl;
        g_producer.notify_all();
    }

}

int main()
{
    std::thread producers[2];
    std::thread consumers[2];
    for (int i = 0; i < 2; i++) {
        producers[i] = std::thread(producer, i + 1);
        consumers[i] = std::thread(consumer);
    }
    for (int i = 0; i < 2; i++) {
        producers[i].join();
        consumers[i].join();
    }
    return 0;
}
```