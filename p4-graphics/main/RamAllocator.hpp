/**
 * @file RamAllocator.hpp
 * @author Andrés Ragot (github.com/andresragot)
 * @brief 
 * @version 0.1
 * @date 2025-04-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once
#include "esp_heap_caps.h"

namespace Ragot {

  template <typename T, uint16_t Flag>
  class PSRAMAllocator 
  {
  public:
    using value_type    = T;
    using pointer       = T*;
    using size_type     = std::size_t;

    template <typename U>
    struct rebind { using other = PSRAMAllocator<U, Flag>; };

    PSRAMAllocator() noexcept {}
    template <typename U, uint16_t F2>
    PSRAMAllocator(const PSRAMAllocator<U, F2>&) noexcept {}

    T* allocate(size_type n) 
    {
      T* p = static_cast<T*>(heap_caps_malloc(n * sizeof(T), Flag));
      if (not p) throw std::bad_alloc();
      return p;
    }

    void deallocate(T* p, size_type) noexcept 
    {
      heap_caps_free(p);
    }
  };
  
  template <typename T, uint16_t F1, typename U, uint16_t F2>
  bool operator==(const PSRAMAllocator<T, F1>&, const PSRAMAllocator<U, F2>&) 
  { 
    return F1 == F2; 
  }
  template <typename T, uint16_t F1, typename U, uint16_t F2>
  bool operator!=(const PSRAMAllocator<T, F1>& a, const PSRAMAllocator<U, F2>& b) 
  {
    return !(a == b);
  }

} // namespace Ragot
