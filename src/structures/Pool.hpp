#ifndef BOITATAH_POOL_HPP
#define BOITATAH_POOL_HPP

#include <cstdint>
// #include <array>
#include <vector>

namespace boitatah
{
    template <typename T>
    struct Handle
    {
        uint32_t i = 0;
        uint32_t gen = 0;

        bool isValid() { return gen != 0; }
    };

    struct PoolOptions
    {
        uint32_t size = 100;
        uint32_t dynamic = true;
    };

    template <typename T>
    class Pool
    {
    public:
        Pool(PoolOptions options);
        //~Pool(void);

        T *get(Handle<T> handle);
        Handle<T> set(T elem);
        T *clear(Handle<T> handle);

    private:
        std::vector<T> data;
        std::vector<uint32_t> generations;
        std::vector<T> pool;

        std::vector<uint32_t> freeStack; // stack of free ids?
        uint32_t stackTop;
        uint32_t quantity;
        uint32_t popStack();
        void pushStack(uint32_t id);
    };

}

// TODO research alternatives.
// Based on the following question.
// https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor

template <typename T>
boitatah::Pool<T>::Pool(PoolOptions options)
{
    generations.resize(options.size, 1);
    // std::fill(generations.begin(), generations.end(), 0);

    freeStack.resize(options.size);
    for (uint32_t i = 0; i < freeStack.size(); i++)
    {
        freeStack[i] = i;
    }

    pool.resize(options.size);

    quantity = 0;
    stackTop = 0;
}

template <typename T>
T *boitatah::Pool<T>::get(Handle<T> handle)
{
    if (generations[handle.i] != handle.gen)
    {
        T *failed = nullptr;
        return failed;
    }
    return &data[handle.i];
}
template <typename T>
boitatah::Handle<T> boitatah::Pool<T>::set(T elem)
{
    if (stackTop == quantity)
        return Handle<T>{.gen = 0};
    uint32_t i = popStack();
    data[i] = elem;
    Handle<T> handle{.i = i, .gen = generations[i]};
    return handle;
}

// Returns an element T if it was successfully removed from the pool.
template <typename T>
T *boitatah::Pool<T>::clear(Handle<T> handle)
{
    if (generations[handle.i] != handle.gen)
    {
        T *succeed = nullptr;
        return succeed;
    }

    // increament in case of removal
    generations[handle.i] = generations[handle.i] + 1;
    pushStack(handle.i);
    return &data[handle.i];
}

template <typename T>
uint32_t boitatah::Pool<T>::popStack()
{
    int i = freeStack[stackTop];
    stackTop++;
    return i;
}

template <typename T>
void boitatah::Pool<T>::pushStack(uint32_t id)
{
    stackTop--;
    freeStack[stackTop] = id;
}

#endif // BOITATAH_POOL_HPP