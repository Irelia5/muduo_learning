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