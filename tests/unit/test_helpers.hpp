#ifndef LVKW_TEST_HELPERS_HPP_INCLUDED
#define LVKW_TEST_HELPERS_HPP_INCLUDED

#include <lvkw/lvkw.h>

#include <map>
#include <mutex>

class TrackingAllocator {
 public:
  struct AllocationInfo {
    size_t size;
  };

  static LVKW_Allocator get_allocator() {
    return {.alloc = [](size_t size, void* userdata) -> void* {
              return static_cast<TrackingAllocator*>(userdata)->allocate(size);
            },
            .free = [](void* ptr, void* userdata) { static_cast<TrackingAllocator*>(userdata)->deallocate(ptr); }};
  }

  void* allocate(size_t size) {
    void* ptr = malloc(size);
    if (ptr) {
      std::lock_guard lock(m_mutex);
      m_allocations[ptr] = {size};
      m_total_allocated += size;
      m_current_count++;
    }
    return ptr;
  }

  void deallocate(void* ptr) {
    if (!ptr) return;
    std::lock_guard lock(m_mutex);
    auto it = m_allocations.find(ptr);
    if (it != m_allocations.end()) {
      m_total_allocated -= it->second.size;
      m_current_count--;
      m_allocations.erase(it);
    }
    free(ptr);
  }

  size_t active_allocations() const { return m_current_count; }
  size_t total_bytes_allocated() const { return m_total_allocated; }

  bool has_leaks() const { return m_current_count > 0; }

 private:
  std::map<void*, AllocationInfo> m_allocations;
  size_t m_total_allocated = 0;
  size_t m_current_count = 0;
  mutable std::mutex m_mutex;
};

#endif
