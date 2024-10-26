#pragma once
#include <functional>
#include <stdlib.h>

class ArenaAllocator {
public:
    inline explicit ArenaAllocator(size_t bytes)
        : m_size(bytes)
    {
        m_buffer = malloc(m_size);
        m_offset = m_buffer;
    }
    template<typename T>
    inline T* alloc() {
        void* offset = m_offset;
        m_offset = static_cast<T*>(offset) + sizeof(T);
        return static_cast<T*>(offset);
    }
    inline ArenaAllocator(const ArenaAllocator& other) = delete;

    inline ArenaAllocator& operator=(const ArenaAllocator& other) = delete;

    inline ~ArenaAllocator() {
        free(m_buffer);
    }

private:
    size_t m_size;
    void *m_buffer;
    void *m_offset;
};
