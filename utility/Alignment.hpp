/**
 * This file copyright (c) 2011-2015, Quickstep Technologies LLC.
 * Copyright (c) 2015, Pivotal Software, Inc.
 * All rights reserved.
 * See file CREDITS.txt for details.
 **/

#ifndef QUICKSTEP_UTILITY_ALIGNMENT_HPP_
#define QUICKSTEP_UTILITY_ALIGNMENT_HPP_

#include <cstddef>
#include <cstdint>

#include "utility/Macros.hpp"
#include "utility/UtilityConfig.h"

#ifdef QUICKSTEP_HAVE_STD_ALIGN
#include <memory>
#endif

#if defined(QUICKSTEP_HAVE_TCMALLOC)
#include "gperftools/include/gperftools/tcmalloc.h"
#elif defined(QUICKSTEP_HAVE_ALIGNED_ALLOC) || defined(QUICKSTEP_HAVE_POSIX_MEMALIGN)
#include <stdlib.h>
#elif defined(QUICKSTEP_HAVE_MEMALIGN) || defined(QUICKSTEP_HAVE_WIN_ALIGNED_MALLOC)
#include <malloc.h>
#endif

namespace quickstep {

/** \addtogroup Utility
 *  @{
 */

/**
 * @brief Fallback for std::align (works exactly the same way) with an
 *        implementation for systems where std::align isn't available.
 **/
inline void* align(std::size_t alignment,
                   std::size_t size,
                   void *&ptr,            // NOLINT(runtime/references)
                   std::size_t &space) {  // NOLINT(runtime/references)
#ifdef QUICKSTEP_HAVE_STD_ALIGN
  return std::align(alignment, size, ptr, space);
#else
  std::uintptr_t ptr_as_uint = reinterpret_cast<std::uintptr_t>(ptr);
  std::uintptr_t adjustment = alignment - (ptr_as_uint % alignment);
  if (adjustment == alignment) {
    adjustment = 0;
  }
  if (adjustment + size <= space) {
    ptr = reinterpret_cast<void*>(ptr_as_uint + adjustment);
    space -= adjustment;
    return ptr;
  } else {
    return nullptr;
  }
#endif
}

/**
 * @brief Allocate size bytes of memory aligned to the specified alignment
 *        using a platform-specific allocator.
 **/
inline void* malloc_with_alignment(const std::size_t size,
                                   const std::size_t alignment) {
#if defined(QUICKSTEP_HAVE_TCMALLOC)
  void *ptr;
  DO_AND_DEBUG_ASSERT_ZERO(tc_posix_memalign(&ptr, alignment, size));
  return ptr;
#elif defined(QUICKSTEP_HAVE_ALIGNED_ALLOC)
  return aligned_alloc(alignment, size);
#elif defined(QUICKSTEP_HAVE_POSIX_MEMALIGN)
  void *ptr;
  DO_AND_DEBUG_ASSERT_ZERO(posix_memalign(&ptr, alignment, size));
  return ptr;
#elif defined(QUICKSTEP_HAVE_MEMALIGN)
  return memalign(alignment, size);
#elif defined(QUICKSTEP_HAVE_WIN_ALIGNED_MALLOC)
  return _aligned_malloc(size, alignment);
#endif
}

/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_UTILITY_ALIGNMENT_HPP_
