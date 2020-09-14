//
// Created by alex on 2020/9/14.
//
#include <mutex>
#include <memory>

template <typename T>
class threadSafeList
{
    struct node
    {
        std::mutex m;
        std::shared_ptr <T> data;
        std::unique_ptr<node> next;
        node() : next() {}
        explicit node(const T &x) : data(std::make_shared<T>(x)){}
    };
    node head;

public:

    threadSafeList()= default;
    ~threadSafeList() {remove_if([](const node& ) {return true;});}
    threadSafeList(const threadSafeList&) = delete;
    threadSafeList &operator=(const threadSafeList &) = delete;
    void push_front(const T & x)
    {
        std::unique_ptr<node> newNode(new node(x));
        std::lock_guard<std::mutex> l(head.m);
        newNode->next = std::move(head->next);
        head->next = std::move(newNode);
    }
    template<typename F>
    void for_each(F f)
    {
        node * cur = &head;
        std::unique_lock<std::mutex> l(head.m);
        while(node * const next = cur->next.get())
        {
            std::unique_lock<std::mutex> nextLock(next->m);
            //unlock pre node
            l.unlock();
            f(*next->data);
            cur = next;
            //hand over mutex
            l = std::move(nextLock);
        }
    }

    template<typename F>
    void remove_if(F f)
    {
        node* cur = &head;
        std::unique_lock<std::mutex> l(head.m);
        while(node * const nextNode = cur->next.get())
        {
            std::unique_lock<std::mutex> nextLock(nextNode->m);
            if(f(*nextNode->data)){
                cur->next = std::move(nextNode->next);
                nextLock.unlock();
            }
            else{
                l.unlock();
                cur = nextNode;
                l = std::move(nextLock);
            }
        }
    }
};

int main()
{





}
