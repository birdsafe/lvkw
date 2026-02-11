#ifndef LVKW_DETAILS_H_INCLUDED
#define LVKW_DETAILS_H_INCLUDED

// Vulkan forward declarations
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;
typedef struct VkInstance_T *VkInstance;

/* --- Optimization and Documentation Hints --- */

// TODO: play around with __attribute__((hot|cold)) and see what kind of impact it has.
/** @brief Marker for functions in the performance-critical "hot path". */
#define LVKW_HOT

/** @brief Marker for initialization or infrequent "cold path" functions. */
#define LVKW_COLD __attribute__((cold))

#endif  // LVKW_DETAILS_H_INCLUDED
