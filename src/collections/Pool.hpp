#ifndef BOITATAH_POOL_HPP
#define BOITATAH_POOL_HPP

#include <cstdint>
// #include <array>
#include <vector>
#include <iostream>
#include <string>
namespace boitatah
{
    template <typename T>
    struct Handle
    {
        uint32_t i = 0;
        uint32_t gen = 0;

        bool isNull() const { return gen == 0; }
    };

    struct PoolOptions
    {
        uint32_t size = 100;
        uint32_t dynamic = true;
        std::string name;
    };

    template <typename T>
    class Pool
    {
    public:
        Pool(PoolOptions options);
        //~Pool(void);

        bool get(Handle<T> handle, T& item);
        Handle<T> set(T &elem);
        bool clear(Handle<T> handle, T& item);

    private:
        PoolOptions options;
        std::vector<uint32_t> generations;
        std::vector<T> pool;

        std::vector<uint32_t> freeStack; // stack of free ids?
        uint32_t stackTop = 0;
        uint32_t quantity = 0;
        uint32_t popStack();
        void pushStack(uint32_t id);
        int created = 0;
        int destroyed = 0;
    };

}

// TODO research alternatives.
// Based on the following question.
// https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor

template <typename T>
boitatah::Pool<T>::Pool(PoolOptions options)
{
    this->options = options;
    generations.resize(options.size, 1);
    // std::fill(generations.begin(), generations.end(), 0);

    freeStack.resize(options.size);
    for (uint32_t i = 0; i < freeStack.size(); i++)
    {
        freeStack[i] = i;
    }

    pool.resize(options.size);

    stackTop = 0;
}

template <typename T>
bool boitatah::Pool<T>::get(Handle<T> handle, T& item)
{
    if (generations[handle.i] != handle.gen)
    {
        return false;
    }
    item = pool[handle.i];
    return true;
}

template <typename T>
boitatah::Handle<T> boitatah::Pool<T>::set(T &elem)
{
    if (stackTop == pool.size()){
        std::cout << options.name << " is full " << std::endl;
        return Handle<T>{.gen = 0};
    }

    uint32_t i = popStack();
    pool[i] = elem;
    Handle<T> handle{.i = i, .gen = generations[i]};
    created+=1;
    quantity += 1;
    return handle;
}

// Returns an element T if it was successfully removed from the pool.
template <typename T>
bool boitatah::Pool<T>::clear(Handle<T> handle, T& item)
{
    if (generations[handle.i] != handle.gen)
    {
        return false;
    }

    // increament in case of removal
    generations[handle.i] = generations[handle.i] + 1;
    pushStack(handle.i);
    item = pool[handle.i];
    quantity -=1;
    destroyed += 1;
    return true;
}

template <typename T>
uint32_t boitatah::Pool<T>::popStack()
{
    //std::cout << options.name << " popped " << quantity << " " << created << " " << destroyed << std::endl;
    int i = freeStack[stackTop];
    stackTop++;
    return i;
}

template <typename T>
void boitatah::Pool<T>::pushStack(uint32_t id)
{
    //std::cout << options.name << " pushed " << quantity << " " << created << " " << destroyed << std::endl;
    stackTop--;
    freeStack[stackTop] = id;
}

#endif // BOITATAH_POOL_HPP