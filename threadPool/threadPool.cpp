//
// Created by alex on 2020/9/11.
//

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <iostream>

class threadPool{
public:
    threadPool(size_t);
    template<typename F, typename ... Args>
    auto enqueue(F && f, Args && ... args) -> std::future<typename std::result_of<F(Args...)>::type>;
    ~threadPool();
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    //synchronization
    std::mutex queue_mutex;
    std::condition_variable cv;
    std::atomic<bool> stop;
};

template<typename F, typename... Args>
auto threadPool::enqueue(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type> {

    using returnType = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<returnType()> >(
            [Func = std::forward<F>(f)] { return Func(); }
            );

    std::future<returnType> result = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(stop)
            throw std::runtime_error("threadPool already stopped");
        tasks.emplace([task](){(*task)();});
    }
    cv.notify_one();
    return result;
}

inline threadPool::threadPool(size_t threads) : stop(false) {

    for(int i = 0; i < threads; i++)
    {
        workers.emplace_back(
                [this]
                {
                    for(;;)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);
                            this->cv.wait(lock, [this] {return this->stop || !this->tasks.empty();});
                            if(this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                }
                );
    }



}

inline threadPool::~threadPool() {

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    cv.notify_all();
    for(auto &t : workers)
        t.join();
}

int main()
{
    threadPool pool(4);
    std::vector<std::future<int>> results;
    for(int i = 0; i < 100; i++)
    {
        results.emplace_back(pool.enqueue(
                [i]
                {
                    return i*i;
                }
                ));
    }

    for (auto & result : results)
        std::cout << result.get() << ' ';
    return 0;
}



