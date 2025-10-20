#pragma once
#include "arena.h"
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <vector>

struct Arena_Allocator
{
    Arena_Allocator() = default;
    constexpr Arena_Allocator(const Arena_Allocator&) = delete;

    template<class T>
    [[nodiscard]] T* allocate(std::size_t n = 1)
    {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();

        if (auto p = static_cast<T*>(arena_alloc(&m_arena, n * sizeof(T))))
        {
            return p;
        }

        throw std::bad_alloc();
    }
    ~Arena_Allocator()
    {
        arena_free(&m_arena);
    }
private:
    Arena m_arena = { 0, 0 };
};