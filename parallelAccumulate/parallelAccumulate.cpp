//
// Created by alex on 2020/9/10.
//

#include <thread>
#include <vector>
#include <numeric>
#include <future>
#include <iostream>

class threadsGuard
{
private:
    std::vector<std::thread>& threads;
public:
    explicit threadsGuard(std::vector<std::thread> & t) : threads(t) {}
    ~threadsGuard()
    {
        for (auto & thread : threads)
        {
            if (thread.joinable())
                thread.join();
        }
    }
};

template<typename Iterator, typename T>
struct parallel_accumulate_block
{
    T operator()(Iterator begin, Iterator end)
    {
        std::accumulate(begin, end, T{});
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator begin, Iterator end, T init)
{
    const unsigned long threadNum = std::thread::hardware_concurrency();
    const unsigned long blockLength = std::distance(begin, end);
    const unsigned long blockSize =  blockLength / threadNum;

    std::vector<std::future<T>> futureList(threadNum - 1);
    std::vector<std::thread> threadList(threadNum - 1);
    threadsGuard tg(threadList);
    Iterator blockBegin = begin;
    for(unsigned long i = 0; i < threadNum - 1; i++)
    {
        Iterator blockEnd = blockBegin;
        std::advance(blockEnd, blockSize);
        std::packaged_task<T(Iterator, Iterator)> pt(parallel_accumulate_block<Iterator, T>{});
        futureList[i] = pt.get_future();
        threadList[i] = std::thread(std::move(pt), blockBegin, blockEnd);
        blockBegin = blockEnd;
    }

    T lastRes = parallel_accumulate_block<Iterator, T>{}(blockBegin, end);
    for(auto & thread : threadList)
    {
        thread.join();
    }

    T res = init;
    for(auto & future : futureList)
    {
        res += future.get();
    }

    res += lastRes;
    return res;
}

template<typename Iterator, typename T>
T parallel_accumulate_async(Iterator begin, Iterator end, T init)
{
    const unsigned long threadNum = std::thread::hardware_concurrency();
    const unsigned long blockLen = std::distance(begin, end);
    if (blockLen < threadNum)
    {
        return std::accumulate(begin, end, init);
    } else
    {
        Iterator mid = begin;
        std::advance(mid, blockLen/2);
        std::future<T> ft = std::async(&parallel_accumulate_async<Iterator, T>, begin, mid, init);
        T secondHalf = parallel_accumulate_async(mid, end, T{});
        return ft.get() + secondHalf;
    }
}

int main()
{
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    int sum1 = std::accumulate(v.begin(), v.end(), 0);

    int sum2 = parallel_accumulate(v.begin(), v.end(), 0);

    int sum3 = parallel_accumulate_async(v.begin(), v.end(), 0);

    std::cout<< sum1 << " " << sum2 << " " << sum3 << std::endl;

    return 0;
}