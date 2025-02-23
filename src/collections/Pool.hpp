#pragma once

#include <cstdint>
// #include <array>
#include <vector>
#include <iostream>
#include <string>
#include <optional>

namespace boitatah
{
    template <typename T>
    struct Handle
    {
        uint32_t i = 0;
        uint32_t gen = 0;
        
        bool isNull() const { return gen == 0; }
        bool operator == (const Handle &other) const
        {
            if( i == other.i && gen == other.gen)
                return true;
            else
                return false;
        } 

        explicit operator bool() const{
            return !isNull();
        }

    };

    struct HandleHasher
    {
        template <typename T>
        std::size_t operator()(const Handle<T>& handle) const
        {
            using std::hash;
                // Compute individual hash values for first, second and third
                // http://stackoverflow.com/a/1646913/126995
                std::size_t res = 17;
                res = res * 31 + hash<uint32_t>()( handle.i );
                res = res * 31 + hash<uint32_t>()( handle.gen );
                return res;
        };
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

        bool tryGet(const Handle<T> handle, T& item);
        T* tryGet(const Handle<T> handle);
        T& get(const Handle<T> handle) noexcept(false);
        Handle<T> set(T &elem);
        Handle<T> move_set(T& elem);
        Handle<T> getHandle();
        bool clear(Handle<T> handle, T& item);
        bool clear(Handle<T> handle);
        bool contains(Handle<T> handle);
        
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
bool boitatah::Pool<T>::tryGet(const Handle<T> handle, T& item)
{
    if (generations[handle.i] != handle.gen)
    {
        return false;
    }
    item = pool[handle.i];
    return true;
}

template <typename T>
inline T *boitatah::Pool<T>::tryGet(const Handle<T> handle)
{
    if (generations[handle.i] != handle.gen)
    {
        return nullptr;
    }
    return &pool[handle.i];
}

template <typename T>
T& boitatah::Pool<T>::get(const Handle<T> handle) noexcept(false)
{
    if (generations[handle.i] != handle.gen)
    {
        auto error = "Invalid Handle. on " + options.name;
        throw std::runtime_error(error);
    }
    return pool[handle.i];
}

template <typename T>
boitatah::Handle<T> boitatah::Pool<T>::set(T &elem)
{
    auto handle = getHandle();
    pool[handle.i] = elem;

    return handle;
}

template <typename T>
inline boitatah::Handle<T> boitatah::Pool<T>::move_set(T &elem)
{
    auto handle = getHandle();
    pool[handle.i] = std::move(elem);
    return handle;
}

template <typename T>
inline boitatah::Handle<T> boitatah::Pool<T>::getHandle()
{
        if (stackTop == pool.size()){
        if(!options.dynamic){
            std::cout << options.name << " is full " << std::endl;
            return Handle<T>{.gen = 0};
        }else{ //resize and keep going
            int old_size = options.size;
            options.size = options.size * static_cast<uint32_t>(2);

            pool.resize(options.size);
            freeStack.resize(options.size);
            for (uint32_t i = old_size; i < freeStack.size(); i++)
            {
                freeStack[i] = i;
            }
        }
    }

    uint32_t i = popStack();
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
    item = get(handle);
    return clear(handle);
}

template <typename T>
inline bool boitatah::Pool<T>::clear(Handle<T> handle)
{   
    if (generations[handle.i] != handle.gen)
    {
        return false;
    }
    generations[handle.i] = generations[handle.i] + 1;
    pushStack(handle.i);
    quantity -=1;
    destroyed += 1;
    return true;
}

template <typename T>
inline bool boitatah::Pool<T>::contains(Handle<T> handle)
{
    if (generations[handle.i] != handle.gen)
    {
        return false;
    }
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

