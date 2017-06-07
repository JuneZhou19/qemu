#ifndef FBE_CSX_HOOK_H
#define FBE_CSX_HOOK_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#define fbe_api_allocate_memory(s)  calloc(1, s)
#define fbe_api_free_memory(p)  free(p)

#define fbe_copy_memory memcpy
#define fbe_zero_memory bzero

#define csx_p_strcat strcat

#define KvTrace printf

#define CSX_IS_ZERO(_v) ((_v) == 0)
#define CSX_NOT_ZERO(_v) ((_v) != 0)

#define CSX_ALIGN_N(_n) __attribute__ ((aligned (_n)))
#define FBE_ALIGN(x) CSX_ALIGN_N(x)

#define FBE_UNREFERENCED_PARAMETER(_p) (void) (_p)

#if !defined(UNREFERENCED_PARAMETER)
#define UNREFERENCED_PARAMETER(P)       \
    /*lint -e527 -e530 */               \
    {                                   \
        (P) = (P);                      \
    }                                   \
    /*lint +e527 +e530 */
#endif

typedef char csx_char_t;

// TODO
#define CSX_ASSERT_AT_COMPILE_TIME_GLOBAL_SCOPE(x)

#if !defined (SIZE_CHECK)
#define SIZE_CHECK(name, length) CSX_ASSERT_AT_COMPILE_TIME_GLOBAL_SCOPE((sizeof(name)) == length)
#endif

#define RETURN_ON_ERROR_STATUS \
    if(status != FBE_STATUS_OK) \
    {                           \
        return(status);         \
    }

typedef uint8_t fbe_u8_t;
typedef char fbe_char_t;
typedef uint16_t fbe_u16_t;
typedef uint32_t fbe_u32_t;
typedef uint64_t fbe_u64_t;
typedef fbe_u32_t fbe_bool_t;
typedef fbe_u64_t fbe_sas_address_t;

#define FBE_TRUE 1
#define FBE_FALSE 0
#define FBE_IS_TRUE(val) ((val) == FBE_TRUE)
#define FBE_IS_FALSE(val) ((val) == FBE_FALSE)

typedef fbe_u8_t PAD;
typedef char CHAR;

typedef void* fbe_terminator_device_ptr_t;

static inline void * fbe_terminator_allocate_memory(fbe_u32_t NumberOfBytes)
{
    return malloc(NumberOfBytes);
}

static inline void fbe_terminator_free_memory(void * mem_ptr)
{
    if (mem_ptr)
        free(mem_ptr);
}

#endif
