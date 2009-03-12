#ifndef __LIB_DEBUG_H
#define __LIB_DEBUG_H

#include <assert.h>

#define UNUSED __attribute__ ((unused))
#define NO_RETURN __attribute__ ((noreturn))
#define NO_INLINE __attribute__ ((noinline))
#define PRINTF_FORMAT(FMT, FIRST) __attribute__ ((format (printf, FMT, FIRST)))
#define ASSERT(COND) assert(COND)

#endif



