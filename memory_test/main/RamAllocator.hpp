/**
 * @brief 
 * 
 */

#pragma once
 #include "esp_heap_caps.h"  // Add this for PSRAM functions

namespace Ragot
{    
    // Custom allocator that uses PSRAM
    template <typename T>
    class PSRAMAllocator 
    {
    public:
        using value_type        = T;
        using pointer           = value_type*;
        using const_pointer     = const value_type*;
        using reference         = value_type&;
        using const_reference   = const value_type&;
        using size_type         = std::size_t;
        using difference_type   = std::ptrdiff_t;
        
        template <typename U>
        struct rebind { typedef PSRAMAllocator<U> other; };
        
        PSRAMAllocator() noexcept {}
        template <class U> PSRAMAllocator(const PSRAMAllocator<U>&) noexcept {}
        
        pointer allocate(size_type n) 
        {
            pointer p = static_cast<pointer>(heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM));
            if (not p)
            {
                throw std::bad_alloc();
            }
            return p;
        }
        
        void deallocate(pointer p, size_type) noexcept 
        {
            heap_caps_free(p);
        }
    };
    
    template <typename T, typename U>
    bool operator==(const PSRAMAllocator<T>&, const PSRAMAllocator<U>&) { return true; }
    
    template <typename T, typename U>
    bool operator!=(const PSRAMAllocator<T>&, const PSRAMAllocator<U>&) { return false; }

}