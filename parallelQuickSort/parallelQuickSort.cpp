//
// Created by alex on 2020/9/10.
//
#include <list>
#include <algorithm>
#include <iostream>
#include <future>

template <typename T>
std::list<T> parallel_quick_sort(std::list<T> lst)
{
    if (lst.empty()) return lst;
    std::list<T> res;
    res.splice(res.begin(), lst, lst.begin());
    const auto & firstVal = *res.begin();
    auto it = std::partition(lst.begin(), lst.end(), [&] (const auto & x) { return x < firstVal;});
    std::list<T> low;
    low.splice(low.end(), lst, lst.begin(), it);
    std::future<std::list<T>> l = std::async(&parallel_quick_sort<T>, std::move(low));
    auto high(parallel_quick_sort<T>(std::move(lst)));
    res.splice(res.begin(), l.get());
    res.splice(res.end(), high);
    return res;
}

int main(){

    std::list<int> lst {1,4,3,2,5,1,2,5,6,7,3};
    auto res = parallel_quick_sort(lst);
    for(auto & x : res)
        std::cout<< x << std::endl;

    return 0;
}
