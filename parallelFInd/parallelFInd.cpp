//
// Created by alex on 2020/9/10.
//

#include <vector>
#include <thread>
#include <future>
#include <iostream>
#include <algorithm>

class threadsGuard
{
    std::vector<std::thread> & threads;
public:
    explicit threadsGuard(std::vector<std::thread> & t) : threads(t) {}
    ~threadsGuard()
    {
        for(auto & t : threads)
        {
            if (t.joinable())
                t.join();
        }
    }
};

template<typename Iterator, typename T>
Iterator parallelFind(Iterator begin, Iterator end, T match)
{
    struct findElement{
        void operator() (Iterator begin, Iterator end, T match, std::promise<Iterator> *promise, std::atomic<bool>* atomicBool)
        {
            try{

                for(;begin!=end && !atomicBool->load(); begin++)
                {
                    if (*begin == match)
                    {
                        promise->set_value(begin);
                        atomicBool->store(true);
                    }
                }

            } catch (...) {
                try{
                    promise->set_exception(std::current_exception());
                    atomicBool->store(true);
                } catch (...) {
                }
            }

        }
    };

    auto threadNum = std::thread::hardware_concurrency();
    auto blockLen = std::distance(begin, end);
    auto blockSize = blockLen / threadNum;
    std::vector<std::thread> threads(threadNum-1);
    std::promise<Iterator> res;
    std::atomic<bool> atomicBool(false);
    {
        threadsGuard tg(threads);
        Iterator blockBegin = begin;
        for(auto i = 0; i < threadNum -1 ;i++)
        {
            Iterator blockEnd = blockBegin;
            std::advance(blockEnd, blockSize);
            threads[i] = std::thread(findElement{}, blockBegin, blockEnd, match, &res, &atomicBool);
            blockBegin = blockEnd;
        }
        findElement()(blockBegin, end, match, &res, &atomicBool);
    }

    if(!atomicBool.load())
    {
        return end;
    }
    return res.get_future().get();
}

template<typename Iterator, typename T>
Iterator parallelFindAsyncImpl(Iterator begin, Iterator end, T match, std::atomic<bool>& atomicBool)
{
    try {
        auto blockLen = std::distance(begin, end);
        auto threadNum = std::thread::hardware_concurrency();
        if (blockLen < threadNum)
        {
            for(;begin!=end && !atomicBool.load();begin++)
            {
                if(match == *begin)
                {
                    atomicBool=true;
                    return begin;
                }
            }
            return end;
        }
        else{
            auto mid = begin + blockLen/2;
            std::future<Iterator> ft = std::async(&parallelFindAsyncImpl<Iterator, T>, begin, mid, match, std::ref(atomicBool));
            const auto directResult = parallelFindAsyncImpl(mid, end, match, atomicBool);
            return directResult == end ? ft.get() : directResult;
        }
    } catch (...) {
        atomicBool=true;
        throw;
    }
}

template<typename Iterator, typename T>
Iterator parallelFindAsync(Iterator begin, Iterator end, T match)
{
    std::atomic<bool> atomicBool(false);
    return parallelFindAsyncImpl(begin, end, match, atomicBool);
}

int main()
{
    int n1 = 3;

    std::vector<int> v{0, 1, 2, 3, 4};

    auto result1 = std::find(std::begin(v), std::end(v), n1);
    auto result2 = parallelFindAsync(v.begin(), v.end(), n1);
    auto result3 = parallelFind(v.begin(), v.end(), n1);

    if (result1 != std::end(v)) {
        std::cout << "v contains: " << n1 << '\n';
    } else {
        std::cout << "v does not contain: " << n1 << '\n';
    }


    if (result2 != std::end(v)) {
        std::cout << "v contains: " << n1 << '\n';
    } else {
        std::cout << "v does not contain: " << n1 << '\n';
    }


    if (result3 != std::end(v)) {
        std::cout << "v contains: " << n1 << '\n';
    } else {
        std::cout << "v does not contain: " << n1 << '\n';
    }

    return 0;
}
