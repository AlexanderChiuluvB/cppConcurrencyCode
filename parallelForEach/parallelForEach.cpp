//
// Created by alex on 2020/9/10.
//

#include <vector>
#include <thread>
#include <future>
#include <algorithm>
#include <list>
#include <iostream>

class threadsGuard
{
private:
    std::vector<std::thread> & threads;
public:
    explicit threadsGuard(std::vector<std::thread> & t) : threads(t) {}
    ~threadsGuard()
    {
        for(auto & t : threads)
        {
            if(t.joinable())
                t.join();
        }
    }
};

template<typename Iterator, typename Func>
void parallel_for_each(Iterator begin, Iterator end, Func f)
{
    auto threadNum = std::thread::hardware_concurrency();
    auto blockLength = std::distance(begin, end);
    auto blockSize = blockLength / threadNum;
    std::vector<std::thread> threadLists(threadNum-1);
    std::vector<std::future<void>> futureLists(threadNum -1);
    Iterator blockBegin = begin;
    threadsGuard tg(threadLists);
    for(auto i = 0; i < threadNum-1; i++)
    {
        Iterator blockEnd = blockBegin;
        std::advance(blockEnd, blockSize);
        std::packaged_task<void(void)> pt( [=] { std::for_each(blockBegin, blockEnd, f);});
        futureLists[i] = pt.get_future();
        threadLists[i] = std::thread(std::move(pt));
        blockBegin = blockEnd;
    }
    std::for_each(blockBegin, end, f);
    for(auto & t : threadLists)
    {
        t.join();
    }
    for(auto & ft: futureLists)
    {
        ft.get();
    }
}

template<typename Iterator, typename Func>
void parallel_for_each_async(Iterator begin, Iterator end, Func f)
{
    auto threadNum = std::thread::hardware_concurrency();
    auto blockLen = std::distance(begin, end);
    if(blockLen < threadNum)
    {
        std::for_each(begin, end, f);
    }
    else{
        Iterator mid = begin;
        std::advance(mid, blockLen / 2);
        std::future<void> ft = std::async(&parallel_for_each_async<Iterator, Func>, begin, mid, f);
        parallel_for_each_async(mid,end, f);
        ft.get();
    }
}

int main() {

    std::list<int> lst{1,4,9,11};

    std::for_each(lst.begin(), lst.end(), [&](auto & n){n++;});

    auto print = [] (const auto & n) { std::cout<< n << " ";};

    std::for_each(lst.begin(), lst.end(), print);

    parallel_for_each(lst.begin(), lst.end(), print);

    parallel_for_each_async(lst.begin(), lst.end(), print);

    return 0;
}

