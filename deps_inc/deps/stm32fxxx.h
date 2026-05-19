#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __SCB_DCACHE_LINE_SIZE 32  // NOLINT

/**
  \brief   D-Cache Invalidate by address
  \details Invalidates D-Cache for the given address.
           D-Cache is invalidated starting from a 32 byte aligned address in 32 byte granularity.
           D-Cache memory blocks which are part of given address + given size are invalidated.
  \param[in]   addr    address
  \param[in]   dsize   size of memory block (in number of bytes)
*/
static inline void SCB_InvalidateDCache_by_Addr(volatile void* addr, int32_t dsize) {}

/**
  \brief   D-Cache Clean by address
  \details Cleans D-Cache for the given address
           D-Cache is cleaned starting from a 32 byte aligned address in 32 byte granularity.
           D-Cache memory blocks which are part of given address + given size are cleaned.
  \param[in]   addr    address
  \param[in]   dsize   size of memory block (in number of bytes)
*/
static inline void SCB_CleanDCache_by_Addr(volatile void* addr, int32_t dsize) {}

/**
  \brief   D-Cache Clean and Invalidate by address
  \details Cleans and invalidates D_Cache for the given address
           D-Cache is cleaned and invalidated starting from a 32 byte aligned address in 32 byte
  granularity. D-Cache memory blocks which are part of given address + given size are cleaned and
  invalidated.
  \param[in]   addr    address (aligned to 32-byte boundary)
  \param[in]   dsize   size of memory block (in number of bytes)
*/
static inline void SCB_CleanInvalidateDCache_by_Addr(volatile void* addr, int32_t dsize) {}

#ifdef __cplusplus
}
#endif