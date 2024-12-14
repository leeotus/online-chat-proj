#pragma once

#include <type_traits>
#include <vector>

template <typename T>
class MemoryPool {
    static constexpr int INITIAL_POOL_SIZE = 1024;
    static constexpr double CHUNK_GROWTH_FACTOR = 2.0;
    static_assert(std::is_member_pointer<decltype(&T::next)>::value, "T has not a member pointer");
    static_assert(std::is_same<decltype(T::next), T*>::value, "T must have a pointer `next` of type `T*`");
public:
    MemoryPool(int initialPoolSize = INITIAL_POOL_SIZE);
    ~MemoryPool();

    void returnEntry(T*);

    void allocNewChunk();

    T* get();

    bool isAvailEmpty();

    bool isCurChunkEmpty();

private:
    int cursor;
    int curChunkSize;
    T* avail;
    std::vector<std::vector<T>> mempool;
};

template <typename T>
MemoryPool<T>::MemoryPool(int initialPoolSize) : 
    avail(nullptr), 
    cursor(0), 
    curChunkSize(initialPoolSize),
    mempool(1,std::vector<T>(initialPoolSize)) {}

