资源的一个特点是：使用完后必须还给系统。如果不归还糟糕的事情就会发生。
C++程序常见的使用资源有：

* 动态分配内存
* 文件描述符(file descriptors)
* 互斥锁(mutex locks)
* 图形界面的字型、笔刷
* 数据库连接
* 网络sockets

比如动态分配的内存，如果不归还，会导致内存泄漏。

这里以一个投资类为例，工厂模式也是我们非常常用的一种设计模式，会很直接的遇到资源管理的问题。

```cpp
class Investment { ... };           //投资类型 -->继承体系中的root class

Investment* CreateInvestment();     //工厂函数，返回指针

void f()
{
    Investment *pInv = CreateInvestment();
    ...
    delete pInv;
}
```
看起来妥当，然而存在一些潜在的问题：
* 将资源的释放交给客户，客户可能并不知情；
* 即使客户知道需要手动释放，也可能因为控制流的过早结束或发生异常而无法触碰到`delete`语句。比如在f函数中的...中提前`return`或抛出异常，那这个指针将永远丢失，导致内存泄漏；
* 即使你足够小心，其他人维护代码添加`return`语句却忘了添加`delete`。

是否有一种方式，能自动回收资源，而不是煞费苦心考虑`delete`的位置，考虑每一个跳出位置和次数？
我们知道栈对象作用域结束后调用析构函数自动回收，资源想要这样应当如何呢？
答案正是将资源放入管理对象中，析构函数负责资源回收，以实现自动回收资源。
两个关键点：

* 获得资源后立刻放进管理对象
* 管理对象运用析构函数确保资源被释放

这样的技术称为**RAII**(Resource Acquisition Is Initialization) ——资源获取时初始化
STL提供了智能指针似乎正是为RAII而设计

```cpp
void f()
{
    std::shared_ptr<Investment> pInv(CreateInvestment());
    std::unique_ptr<Investment> pInv2(CreateInvestment());
}
```
其中`std::shared_ptr`是引用计数型智能指针，持续追踪对象对象指向某资源，当无人指向它时自动删除该资源；
`std::unique_pt`r是独占型智能指针，仅有该指针唯一指向某资源，不可被复制。

要注意的是：智能指针默认析构函数做`delete`而不是`delete[]`，因此对于数组不能进行这样的动作。对于其他类型的资源，比如文件描述符等，定制`shared_ptr`的删除器，使析构函数调用该删除器完成自动释放。
```cpp
T* delete_func()
{
    //(*T).close();
    delete T;
}
std::shared_ptr<T> pInv(CreateT(), delete_func);    //定制删除器
```