#line 2 "suites/main_test.function"
/*
 * *** THIS FILE HAS BEEN MACHINE GENERATED ***
 *
 * This file has been machine generated using the script:
 * generate_test_code.py
 *
 * Test file      : ./test_suite_pk.c
 *
 * The following files were used to create this file.
 *
 *      Main code file      : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/main_test.function
 *      Platform code file  : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/host_test.function
 *      Helper file         : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/helpers.function
 *      Test suite file     : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function
 *      Test suite data     : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.data
 *
 */

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 1 // for fileno() from <stdio.h>
#endif
#endif

/*
 * for arc4random_buf() from <stdlib.h>
 */
#if defined(__NetBSD__)
#define _NETBSD_SOURCE 1
#elif defined(__OpenBSD__)
#define _BSD_SOURCE 1
#endif

#if !defined(MBEDTLS_CONFIG_FILE)
#include <mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE
#endif


/*----------------------------------------------------------------------------*/
/* Common helper code */

#line 2 "suites/helpers.function"
/*----------------------------------------------------------------------------*/
/* Headers */

#include <stdlib.h>

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_fprintf    fprintf
#define mbedtls_snprintf   snprintf
#define mbedtls_calloc     calloc
#define mbedtls_free       free
#define mbedtls_exit       exit
#define mbedtls_time       time
#define mbedtls_time_t     time_t
#define MBEDTLS_EXIT_SUCCESS EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE EXIT_FAILURE
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include "mbedtls/memory_buffer_alloc.h"
#endif

#if defined(MBEDTLS_CHECK_PARAMS)
#include "mbedtls/platform_util.h"
#include <setjmp.h>
#endif

#ifdef _MSC_VER
#include <basetsd.h>
typedef UINT8 uint8_t;
typedef INT32 int32_t;
typedef UINT32 uint32_t;
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#else
#include <stdint.h>
#endif

#include <string.h>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <strings.h>
#endif

/*
 * Define the two macros
 *
 *  #define TEST_CF_SECRET(ptr, size)
 *  #define TEST_CF_PUBLIC(ptr, size)
 *
 * that can be used in tests to mark a memory area as secret (no branch or
 * memory access should depend on it) or public (default, only needs to be
 * marked explicitly when it was derived from secret data).
 *
 * Arguments:
 * - ptr: a pointer to the memory area to be marked
 * - size: the size in bytes of the memory area
 *
 * Implementation:
 * The basic idea is that of ctgrind <https://github.com/agl/ctgrind>: we can
 * re-use tools that were designed for checking use of uninitialized memory.
 * This file contains two implementations: one based on MemorySanitizer, the
 * other on valgrind's memcheck. If none of them is enabled, dummy macros that
 * do nothing are defined for convenience.
 */
#if defined(MBEDTLS_TEST_CONSTANT_FLOW_MEMSAN)
#include <sanitizer/msan_interface.h>

/* Use macros to avoid messing up with origin tracking */
#define TEST_CF_SECRET  __msan_allocated_memory
// void __msan_allocated_memory(const volatile void* data, size_t size);
#define TEST_CF_PUBLIC  __msan_unpoison
// void __msan_unpoison(const volatile void *a, size_t size);

#elif defined(MBEDTLS_TEST_CONSTANT_FLOW_VALGRIND)
#include <valgrind/memcheck.h>

#define TEST_CF_SECRET  VALGRIND_MAKE_MEM_UNDEFINED
// VALGRIND_MAKE_MEM_UNDEFINED(_qzz_addr, _qzz_len)
#define TEST_CF_PUBLIC  VALGRIND_MAKE_MEM_DEFINED
// VALGRIND_MAKE_MEM_DEFINED(_qzz_addr, _qzz_len)

#else /* MBEDTLS_TEST_CONSTANT_FLOW_MEMSAN ||
         MBEDTLS_TEST_CONSTANT_FLOW_VALGRIND */

#define TEST_CF_SECRET(ptr, size)
#define TEST_CF_PUBLIC(ptr, size)

#endif /* MBEDTLS_TEST_CONSTANT_FLOW_MEMSAN */

/* Type for Hex parameters */
typedef struct data_tag
{
    uint8_t *   x;
    uint32_t    len;
} data_t;

/*----------------------------------------------------------------------------*/
/* Status and error constants */

#define DEPENDENCY_SUPPORTED            0   /* Dependency supported by build */
#define KEY_VALUE_MAPPING_FOUND         0   /* Integer expression found */
#define DISPATCH_TEST_SUCCESS           0   /* Test dispatch successful */

#define KEY_VALUE_MAPPING_NOT_FOUND     -1  /* Integer expression not found */
#define DEPENDENCY_NOT_SUPPORTED        -2  /* Dependency not supported */
#define DISPATCH_TEST_FN_NOT_FOUND      -3  /* Test function not found */
#define DISPATCH_INVALID_TEST_DATA      -4  /* Invalid test parameter type.
                                               Only int, string, binary data
                                               and integer expressions are
                                               allowed */
#define DISPATCH_UNSUPPORTED_SUITE      -5  /* Test suite not supported by the
                                               build */

typedef enum
{
    PARAMFAIL_TESTSTATE_IDLE = 0,           /* No parameter failure call test */
    PARAMFAIL_TESTSTATE_PENDING,            /* Test call to the parameter failure
                                             * is pending */
    PARAMFAIL_TESTSTATE_CALLED              /* The test call to the parameter
                                             * failure function has been made */
} paramfail_test_state_t;


/*----------------------------------------------------------------------------*/
/* Macros */

/**
 * \brief   This macro tests the expression passed to it as a test step or
 *          individual test in a test case.
 *
 *          It allows a library function to return a value and return an error
 *          code that can be tested.
 *
 *          When MBEDTLS_CHECK_PARAMS is enabled, calls to the parameter failure
 *          callback, MBEDTLS_PARAM_FAILED(), will be assumed to be a test
 *          failure.
 *
 *          This macro is not suitable for negative parameter validation tests,
 *          as it assumes the test step will not create an error.
 *
 * \param   TEST    The test expression to be tested.
 */
#define TEST_ASSERT( TEST )                                 \
    do {                                                    \
       if( ! (TEST) )                                       \
       {                                                    \
          test_fail( #TEST, __LINE__, __FILE__ );           \
          goto exit;                                        \
       }                                                    \
    } while( 0 )

/** Compare two buffers and fail the test case if they differ.
 *
 * This macro expands to an instruction, not an expression.
 * It may jump to the \c exit label.
 *
 * \param p1        Pointer to the start of the first buffer.
 * \param size1     Size of the first buffer in bytes.
 *                  This expression may be evaluated multiple times.
 * \param p2        Pointer to the start of the second buffer.
 * \param size2     Size of the second buffer in bytes.
 *                  This expression may be evaluated multiple times.
 */
#define ASSERT_COMPARE( p1, size1, p2, size2 )                          \
    do                                                                  \
    {                                                                   \
        TEST_ASSERT( ( size1 ) == ( size2 ) );                          \
        if( ( size1 ) != 0 )                                            \
            TEST_ASSERT( memcmp( ( p1 ), ( p2 ), ( size1 ) ) == 0 );    \
    }                                                                   \
    while( 0 )

/**
 * \brief   This macro tests the expression passed to it and skips the
 *          running test if it doesn't evaluate to 'true'.
 *
 * \param   TEST    The test expression to be tested.
 */
#define TEST_ASSUME( TEST )                         \
    do {                                            \
        if( ! (TEST) )                              \
        {                                           \
            test_skip( #TEST, __LINE__, __FILE__ ); \
            goto exit;                              \
        }                                           \
    } while( 0 )

#if defined(MBEDTLS_CHECK_PARAMS) && !defined(MBEDTLS_PARAM_FAILED_ALT)
/**
 * \brief   This macro tests the statement passed to it as a test step or
 *          individual test in a test case. The macro assumes the test will fail
 *          and will generate an error.
 *
 *          It allows a library function to return a value and tests the return
 *          code on return to confirm the given error code was returned.
 *
 *          When MBEDTLS_CHECK_PARAMS is enabled, calls to the parameter failure
 *          callback, MBEDTLS_PARAM_FAILED(), are assumed to indicate the
 *          expected failure, and the test will pass.
 *
 *          This macro is intended for negative parameter validation tests,
 *          where the failing function may return an error value or call
 *          MBEDTLS_PARAM_FAILED() to indicate the error.
 *
 * \param   PARAM_ERROR_VALUE   The expected error code.
 *
 * \param   TEST                The test expression to be tested.
 */
#define TEST_INVALID_PARAM_RET( PARAM_ERR_VALUE, TEST )                     \
    do {                                                                    \
        test_info.paramfail_test_state = PARAMFAIL_TESTSTATE_PENDING;       \
        if( (TEST) != (PARAM_ERR_VALUE) ||                                  \
            test_info.paramfail_test_state != PARAMFAIL_TESTSTATE_CALLED )  \
        {                                                                   \
            test_fail( #TEST, __LINE__, __FILE__ );                         \
            goto exit;                                                      \
        }                                                                   \
   } while( 0 )

/**
 * \brief   This macro tests the statement passed to it as a test step or
 *          individual test in a test case. The macro assumes the test will fail
 *          and will generate an error.
 *
 *          It assumes the library function under test cannot return a value and
 *          assumes errors can only be indicated byt calls to
 *          MBEDTLS_PARAM_FAILED().
 *
 *          When MBEDTLS_CHECK_PARAMS is enabled, calls to the parameter failure
 *          callback, MBEDTLS_PARAM_FAILED(), are assumed to indicate the
 *          expected failure. If MBEDTLS_CHECK_PARAMS is not enabled, no test
 *          can be made.
 *
 *          This macro is intended for negative parameter validation tests,
 *          where the failing function can only return an error by calling
 *          MBEDTLS_PARAM_FAILED() to indicate the error.
 *
 * \param   TEST                The test expression to be tested.
 */
#define TEST_INVALID_PARAM( TEST )                                          \
    do {                                                                    \
        memcpy(jmp_tmp, param_fail_jmp, sizeof(jmp_buf));                   \
        if( setjmp( param_fail_jmp ) == 0 )                                 \
        {                                                                   \
            TEST;                                                           \
            test_fail( #TEST, __LINE__, __FILE__ );                         \
            goto exit;                                                      \
        }                                                                   \
        memcpy(param_fail_jmp, jmp_tmp, sizeof(jmp_buf));                   \
    } while( 0 )
#endif /* MBEDTLS_CHECK_PARAMS && !MBEDTLS_PARAM_FAILED_ALT */

/**
 * \brief   This macro tests the statement passed to it as a test step or
 *          individual test in a test case. The macro assumes the test will not fail.
 *
 *          It assumes the library function under test cannot return a value and
 *          assumes errors can only be indicated by calls to
 *          MBEDTLS_PARAM_FAILED().
 *
 *          When MBEDTLS_CHECK_PARAMS is enabled, calls to the parameter failure
 *          callback, MBEDTLS_PARAM_FAILED(), are assumed to indicate the
 *          expected failure. If MBEDTLS_CHECK_PARAMS is not enabled, no test
 *          can be made.
 *
 *          This macro is intended to test that functions returning void
 *          accept all of the parameter values they're supposed to accept - eg
 *          that they don't call MBEDTLS_PARAM_FAILED() when a parameter
 *          that's allowed to be NULL happens to be NULL.
 *
 *          Note: for functions that return something other that void,
 *          checking that they accept all the parameters they're supposed to
 *          accept is best done by using TEST_ASSERT() and checking the return
 *          value as well.
 *
 *          Note: this macro is available even when #MBEDTLS_CHECK_PARAMS is
 *          disabled, as it makes sense to check that the functions accept all
 *          legal values even if this option is disabled - only in that case,
 *          the test is more about whether the function segfaults than about
 *          whether it invokes MBEDTLS_PARAM_FAILED().
 *
 * \param   TEST                The test expression to be tested.
 */
#define TEST_VALID_PARAM( TEST )                                    \
    TEST_ASSERT( ( TEST, 1 ) );

#define TEST_HELPER_ASSERT(a) if( !( a ) )                                      \
{                                                                   \
    mbedtls_fprintf( stderr, "Assertion Failed at %s:%d - %s\n",   \
                             __FILE__, __LINE__, #a );              \
    mbedtls_exit( 1 );                                             \
}

#if defined(__GNUC__)
/* Test if arg and &(arg)[0] have the same type. This is true if arg is
 * an array but not if it's a pointer. */
#define IS_ARRAY_NOT_POINTER( arg )                                     \
    ( ! __builtin_types_compatible_p( __typeof__( arg ),                \
                                      __typeof__( &( arg )[0] ) ) )
#else
/* On platforms where we don't know how to implement this check,
 * omit it. Oh well, a non-portable check is better than nothing. */
#define IS_ARRAY_NOT_POINTER( arg ) 1
#endif

/* A compile-time constant with the value 0. If `const_expr` is not a
 * compile-time constant with a nonzero value, cause a compile-time error. */
#define STATIC_ASSERT_EXPR( const_expr )                                \
    ( 0 && sizeof( struct { unsigned int STATIC_ASSERT : 1 - 2 * ! ( const_expr ); } ) )
/* Return the scalar value `value` (possibly promoted). This is a compile-time
 * constant if `value` is. `condition` must be a compile-time constant.
 * If `condition` is false, arrange to cause a compile-time error. */
#define STATIC_ASSERT_THEN_RETURN( condition, value )   \
    ( STATIC_ASSERT_EXPR( condition ) ? 0 : ( value ) )

#define ARRAY_LENGTH_UNSAFE( array )            \
    ( sizeof( array ) / sizeof( *( array ) ) )
/** Return the number of elements of a static or stack array.
 *
 * \param array         A value of array (not pointer) type.
 *
 * \return The number of elements of the array.
 */
#define ARRAY_LENGTH( array )                                           \
    ( STATIC_ASSERT_THEN_RETURN( IS_ARRAY_NOT_POINTER( array ),         \
                                 ARRAY_LENGTH_UNSAFE( array ) ) )

/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)                            \
{                                                       \
    (n) = ( (uint32_t) (b)[(i)    ] << 24 )             \
        | ( (uint32_t) (b)[(i) + 1] << 16 )             \
        | ( (uint32_t) (b)[(i) + 2] <<  8 )             \
        | ( (uint32_t) (b)[(i) + 3]       );            \
}
#endif

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                            \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}
#endif


/*----------------------------------------------------------------------------*/
/* Global variables */

typedef enum
{
    TEST_RESULT_SUCCESS = 0,
    TEST_RESULT_FAILED,
    TEST_RESULT_SKIPPED
} test_result_t;

static struct
{
    paramfail_test_state_t paramfail_test_state;
    test_result_t result;
    const char *test;
    const char *filename;
    int line_no;
}
test_info;

#if defined(MBEDTLS_PLATFORM_C)
mbedtls_platform_context platform_ctx;
#endif

#if defined(MBEDTLS_CHECK_PARAMS)
jmp_buf param_fail_jmp;
jmp_buf jmp_tmp;
#endif

/*----------------------------------------------------------------------------*/
/* Helper flags for complex dependencies */

/* Indicates whether we expect mbedtls_entropy_init
 * to initialize some strong entropy source. */
#if defined(MBEDTLS_TEST_NULL_ENTROPY) ||             \
    ( !defined(MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES) && \
      ( !defined(MBEDTLS_NO_PLATFORM_ENTROPY)  ||     \
         defined(MBEDTLS_HAVEGE_C)             ||     \
         defined(MBEDTLS_ENTROPY_HARDWARE_ALT) ||     \
         defined(ENTROPY_NV_SEED) ) )
#define ENTROPY_HAVE_STRONG
#endif


/*----------------------------------------------------------------------------*/
/* Helper Functions */

void test_fail( const char *test, int line_no, const char* filename )
{
    if( test_info.result == TEST_RESULT_FAILED )
    {
        /* We've already recorded the test as having failed. Don't
         * overwrite any previous information about the failure. */
        return;
    }
    test_info.result = TEST_RESULT_FAILED;
    test_info.test = test;
    test_info.line_no = line_no;
    test_info.filename = filename;
}

void test_skip( const char *test, int line_no, const char* filename )
{
    test_info.result = TEST_RESULT_SKIPPED;
    test_info.test = test;
    test_info.line_no = line_no;
    test_info.filename = filename;
}

static int platform_setup()
{
    int ret = 0;
#if defined(MBEDTLS_PLATFORM_C)
    ret = mbedtls_platform_setup( &platform_ctx );
#endif /* MBEDTLS_PLATFORM_C */
    return( ret );
}

static void platform_teardown()
{
#if defined(MBEDTLS_PLATFORM_C)
    mbedtls_platform_teardown( &platform_ctx );
#endif /* MBEDTLS_PLATFORM_C */
}

#if defined(MBEDTLS_CHECK_PARAMS)
void mbedtls_param_failed( const char *failure_condition,
                           const char *file,
                           int line )
{
    /* If we are testing the callback function...  */
    if( test_info.paramfail_test_state == PARAMFAIL_TESTSTATE_PENDING )
    {
        test_info.paramfail_test_state = PARAMFAIL_TESTSTATE_CALLED;
    }
    else
    {
        /* ...else we treat this as an error */

        /* Record the location of the failure, but not as a failure yet, in case
         * it was part of the test */
        test_fail( failure_condition, line, file );
        test_info.result = TEST_RESULT_SUCCESS;

        longjmp( param_fail_jmp, 1 );
    }
}
#endif

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
static int redirect_output( FILE* out_stream, const char* path )
{
    int out_fd, dup_fd;
    FILE* path_stream;

    out_fd = fileno( out_stream );
    dup_fd = dup( out_fd );

    if( dup_fd == -1 )
    {
        return( -1 );
    }

    path_stream = fopen( path, "w" );
    if( path_stream == NULL )
    {
        close( dup_fd );
        return( -1 );
    }

    fflush( out_stream );
    if( dup2( fileno( path_stream ), out_fd ) == -1 )
    {
        close( dup_fd );
        fclose( path_stream );
        return( -1 );
    }

    fclose( path_stream );
    return( dup_fd );
}

static int restore_output( FILE* out_stream, int dup_fd )
{
    int out_fd = fileno( out_stream );

    fflush( out_stream );
    if( dup2( dup_fd, out_fd ) == -1 )
    {
        close( out_fd );
        close( dup_fd );
        return( -1 );
    }

    close( dup_fd );
    return( 0 );
}
#endif /* __unix__ || __APPLE__ __MACH__ */

int mbedtls_test_unhexify( unsigned char *obuf, const char *ibuf )
{
    unsigned char c, c2;
    int len = strlen( ibuf ) / 2;
    TEST_HELPER_ASSERT( strlen( ibuf ) % 2 == 0 ); /* must be even number of bytes */

    while( *ibuf != 0 )
    {
        c = *ibuf++;
        if( c >= '0' && c <= '9' )
            c -= '0';
        else if( c >= 'a' && c <= 'f' )
            c -= 'a' - 10;
        else if( c >= 'A' && c <= 'F' )
            c -= 'A' - 10;
        else
            TEST_HELPER_ASSERT( 0 );

        c2 = *ibuf++;
        if( c2 >= '0' && c2 <= '9' )
            c2 -= '0';
        else if( c2 >= 'a' && c2 <= 'f' )
            c2 -= 'a' - 10;
        else if( c2 >= 'A' && c2 <= 'F' )
            c2 -= 'A' - 10;
        else
            TEST_HELPER_ASSERT( 0 );

        *obuf++ = ( c << 4 ) | c2;
    }

    return len;
}

void mbedtls_test_hexify( unsigned char *obuf, const unsigned char *ibuf, int len )
{
    unsigned char l, h;

    while( len != 0 )
    {
        h = *ibuf / 16;
        l = *ibuf % 16;

        if( h < 10 )
            *obuf++ = '0' + h;
        else
            *obuf++ = 'a' + h - 10;

        if( l < 10 )
            *obuf++ = '0' + l;
        else
            *obuf++ = 'a' + l - 10;

        ++ibuf;
        len--;
    }
}

/**
 * Allocate and zeroize a buffer.
 *
 * If the size if zero, a pointer to a zeroized 1-byte buffer is returned.
 *
 * For convenience, dies if allocation fails.
 */
static unsigned char *zero_alloc( size_t len )
{
    void *p;
    size_t actual_len = ( len != 0 ) ? len : 1;

    p = mbedtls_calloc( 1, actual_len );
    TEST_HELPER_ASSERT( p != NULL );

    memset( p, 0x00, actual_len );

    return( p );
}

/**
 * Allocate and fill a buffer from hex data.
 *
 * The buffer is sized exactly as needed. This allows to detect buffer
 * overruns (including overreads) when running the test suite under valgrind.
 *
 * If the size if zero, a pointer to a zeroized 1-byte buffer is returned.
 *
 * For convenience, dies if allocation fails.
 */
unsigned char *unhexify_alloc( const char *ibuf, size_t *olen )
{
    unsigned char *obuf;

    *olen = strlen( ibuf ) / 2;

    if( *olen == 0 )
        return( zero_alloc( *olen ) );

    obuf = mbedtls_calloc( 1, *olen );
    TEST_HELPER_ASSERT( obuf != NULL );

    (void) mbedtls_test_unhexify( obuf, ibuf );

    return( obuf );
}

/**
 * This function just returns data from rand().
 * Although predictable and often similar on multiple
 * runs, this does not result in identical random on
 * each run. So do not use this if the results of a
 * test depend on the random data that is generated.
 *
 * rng_state shall be NULL.
 */
static int rnd_std_rand( void *rng_state, unsigned char *output, size_t len )
{
#if !defined(__OpenBSD__) && !defined(__NetBSD__)
    size_t i;

    if( rng_state != NULL )
        rng_state  = NULL;

    for( i = 0; i < len; ++i )
        output[i] = rand();
#else
    if( rng_state != NULL )
        rng_state = NULL;

    arc4random_buf( output, len );
#endif /* !OpenBSD && !NetBSD */

    return( 0 );
}

/**
 * This function only returns zeros
 *
 * rng_state shall be NULL.
 */
int rnd_zero_rand( void *rng_state, unsigned char *output, size_t len )
{
    if( rng_state != NULL )
        rng_state  = NULL;

    memset( output, 0, len );

    return( 0 );
}

typedef struct
{
    unsigned char *buf;
    size_t length;
} rnd_buf_info;

/**
 * This function returns random based on a buffer it receives.
 *
 * rng_state shall be a pointer to a rnd_buf_info structure.
 *
 * The number of bytes released from the buffer on each call to
 * the random function is specified by per_call. (Can be between
 * 1 and 4)
 *
 * After the buffer is empty it will return rand();
 */
int rnd_buffer_rand( void *rng_state, unsigned char *output, size_t len )
{
    rnd_buf_info *info = (rnd_buf_info *) rng_state;
    size_t use_len;

    if( rng_state == NULL )
        return( rnd_std_rand( NULL, output, len ) );

    use_len = len;
    if( len > info->length )
        use_len = info->length;

    if( use_len )
    {
        memcpy( output, info->buf, use_len );
        info->buf += use_len;
        info->length -= use_len;
    }

    if( len - use_len > 0 )
        return( rnd_std_rand( NULL, output + use_len, len - use_len ) );

    return( 0 );
}

/**
 * Info structure for the pseudo random function
 *
 * Key should be set at the start to a test-unique value.
 * Do not forget endianness!
 * State( v0, v1 ) should be set to zero.
 */
typedef struct
{
    uint32_t key[16];
    uint32_t v0, v1;
} rnd_pseudo_info;

/**
 * This function returns random based on a pseudo random function.
 * This means the results should be identical on all systems.
 * Pseudo random is based on the XTEA encryption algorithm to
 * generate pseudorandom.
 *
 * rng_state shall be a pointer to a rnd_pseudo_info structure.
 */
int rnd_pseudo_rand( void *rng_state, unsigned char *output, size_t len )
{
    rnd_pseudo_info *info = (rnd_pseudo_info *) rng_state;
    uint32_t i, *k, sum, delta=0x9E3779B9;
    unsigned char result[4], *out = output;

    if( rng_state == NULL )
        return( rnd_std_rand( NULL, output, len ) );

    k = info->key;

    while( len > 0 )
    {
        size_t use_len = ( len > 4 ) ? 4 : len;
        sum = 0;

        for( i = 0; i < 32; i++ )
        {
            info->v0 += ( ( ( info->v1 << 4 ) ^ ( info->v1 >> 5 ) )
                            + info->v1 ) ^ ( sum + k[sum & 3] );
            sum += delta;
            info->v1 += ( ( ( info->v0 << 4 ) ^ ( info->v0 >> 5 ) )
                            + info->v0 ) ^ ( sum + k[( sum>>11 ) & 3] );
        }

        PUT_UINT32_BE( info->v0, result, 0 );
        memcpy( out, result, use_len );
        len -= use_len;
        out += 4;
    }

    return( 0 );
}

int mbedtls_test_hexcmp( uint8_t * a, uint8_t * b, uint32_t a_len, uint32_t b_len )
{
    int ret = 0;
    uint32_t i = 0;

    if( a_len != b_len )
        return( -1 );

    for( i = 0; i < a_len; i++ )
    {
        if( a[i] != b[i] )
        {
            ret = -1;
            break;
        }
    }
    return ret;
}


#line 48 "suites/main_test.function"


/*----------------------------------------------------------------------------*/
/* Test Suite Code */


#define TEST_SUITE_ACTIVE

#if defined(MBEDTLS_PK_C)
#line 2 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
#include "mbedtls/pk.h"

/* For error codes */
#include "mbedtls/asn1.h"
#include "mbedtls/base64.h"
#include "mbedtls/ecp.h"
#include "mbedtls/rsa.h"

#include <limits.h>
#include <stdint.h>

static int rnd_std_rand( void *rng_state, unsigned char *output, size_t len );

#define RSA_KEY_SIZE 512
#define RSA_KEY_LEN   64

static int pk_genkey( mbedtls_pk_context *pk )
{
    ((void) pk);

#if defined(MBEDTLS_RSA_C) && defined(MBEDTLS_GENPRIME)
    if( mbedtls_pk_get_type( pk ) == MBEDTLS_PK_RSA )
        return mbedtls_rsa_gen_key( mbedtls_pk_rsa( *pk ), rnd_std_rand, NULL, RSA_KEY_SIZE, 3 );
#endif
#if defined(MBEDTLS_ECP_C)
    if( mbedtls_pk_get_type( pk ) == MBEDTLS_PK_ECKEY ||
        mbedtls_pk_get_type( pk ) == MBEDTLS_PK_ECKEY_DH ||
        mbedtls_pk_get_type( pk ) == MBEDTLS_PK_ECDSA )
    {
        int ret;
        if( ( ret = mbedtls_ecp_group_load( &mbedtls_pk_ec( *pk )->grp,
                                      MBEDTLS_ECP_DP_SECP192R1 ) ) != 0 )
            return( ret );

        return mbedtls_ecp_gen_keypair( &mbedtls_pk_ec( *pk )->grp, &mbedtls_pk_ec( *pk )->d,
                                &mbedtls_pk_ec( *pk )->Q, rnd_std_rand, NULL );
    }
#endif
    return( -1 );
}

#if defined(MBEDTLS_RSA_C)
int mbedtls_rsa_decrypt_func( void *ctx, int mode, size_t *olen,
                       const unsigned char *input, unsigned char *output,
                       size_t output_max_len )
{
    return( mbedtls_rsa_pkcs1_decrypt( (mbedtls_rsa_context *) ctx,
                                       rnd_std_rand, NULL, mode, olen,
                                       input, output, output_max_len ) );
}
int mbedtls_rsa_sign_func( void *ctx,
                   int (*f_rng)(void *, unsigned char *, size_t), void *p_rng,
                   int mode, mbedtls_md_type_t md_alg, unsigned int hashlen,
                   const unsigned char *hash, unsigned char *sig )
{
    ((void) f_rng);
    ((void) p_rng);
    return( mbedtls_rsa_pkcs1_sign( (mbedtls_rsa_context *) ctx, rnd_std_rand, NULL, mode,
                            md_alg, hashlen, hash, sig ) );
}
size_t mbedtls_rsa_key_len_func( void *ctx )
{
    return( ((const mbedtls_rsa_context *) ctx)->len );
}
#endif /* MBEDTLS_RSA_C */
#line 75 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_valid_parameters( )
{
    mbedtls_pk_context pk;
    unsigned char buf[1];
    size_t len;
    void *options = NULL;

    mbedtls_pk_init( &pk );

    TEST_VALID_PARAM( mbedtls_pk_free( NULL ) );

#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ECP_RESTARTABLE)
    TEST_VALID_PARAM( mbedtls_pk_restart_free( NULL ) );
#endif

    TEST_ASSERT( mbedtls_pk_setup( &pk, NULL ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );

    /* In informational functions, we accept NULL where a context pointer
     * is expected because that's what the library has done forever.
     * We do not document that NULL is accepted, so we may wish to change
     * the behavior in a future version. */
    TEST_ASSERT( mbedtls_pk_get_bitlen( NULL ) == 0 );
    TEST_ASSERT( mbedtls_pk_get_len( NULL ) == 0 );
    TEST_ASSERT( mbedtls_pk_can_do( NULL, MBEDTLS_PK_NONE ) == 0 );

    TEST_ASSERT( mbedtls_pk_sign_restartable( &pk,
                                              MBEDTLS_MD_NONE,
                                              NULL, 0,
                                              buf, &len,
                                              rnd_std_rand, NULL,
                                              NULL ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );

    TEST_ASSERT( mbedtls_pk_sign_restartable( &pk,
                                              MBEDTLS_MD_NONE,
                                              NULL, 0,
                                              buf, &len,
                                              rnd_std_rand, NULL,
                                              NULL ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );

    TEST_ASSERT( mbedtls_pk_sign( &pk,
                                  MBEDTLS_MD_NONE,
                                  NULL, 0,
                                  buf, &len,
                                  rnd_std_rand, NULL ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );

    TEST_ASSERT( mbedtls_pk_verify_restartable( &pk,
                                                MBEDTLS_MD_NONE,
                                                NULL, 0,
                                                buf, sizeof( buf ),
                                                NULL ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );

    TEST_ASSERT( mbedtls_pk_verify( &pk,
                                    MBEDTLS_MD_NONE,
                                    NULL, 0,
                                    buf, sizeof( buf ) ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );

    TEST_ASSERT( mbedtls_pk_verify_ext( MBEDTLS_PK_NONE, options,
                                        &pk,
                                        MBEDTLS_MD_NONE,
                                        NULL, 0,
                                        buf, sizeof( buf ) ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );

    TEST_ASSERT( mbedtls_pk_encrypt( &pk,
                                     NULL, 0,
                                     NULL, &len, 0,
                                     rnd_std_rand, NULL ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );

    TEST_ASSERT( mbedtls_pk_decrypt( &pk,
                                     NULL, 0,
                                     NULL, &len, 0,
                                     rnd_std_rand, NULL ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );

#if defined(MBEDTLS_PK_PARSE_C)
    TEST_ASSERT( mbedtls_pk_parse_key( &pk, NULL, 0, NULL, 1 ) ==
                 MBEDTLS_ERR_PK_KEY_INVALID_FORMAT );

    TEST_ASSERT( mbedtls_pk_parse_public_key( &pk, NULL, 0 ) ==
                 MBEDTLS_ERR_PK_KEY_INVALID_FORMAT );
#endif /* MBEDTLS_PK_PARSE_C */
exit:
    ;
}

void test_valid_parameters_wrapper( void ** params )
{
    (void)params;

    test_valid_parameters(  );
}
#if defined(MBEDTLS_PK_WRITE_C)
#line 167 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_valid_parameters_pkwrite( data_t *key_data )
{
    mbedtls_pk_context pk;

    /* For the write tests to be effective, we need a valid key pair. */
    mbedtls_pk_init( &pk );
    TEST_ASSERT( mbedtls_pk_parse_key( &pk,
                                       key_data->x, key_data->len,
                                       NULL, 0 ) == 0 );

    TEST_ASSERT( mbedtls_pk_write_key_der( &pk, NULL, 0 ) ==
                 MBEDTLS_ERR_ASN1_BUF_TOO_SMALL );

    TEST_ASSERT( mbedtls_pk_write_pubkey_der( &pk, NULL, 0 ) ==
                 MBEDTLS_ERR_ASN1_BUF_TOO_SMALL );

#if defined(MBEDTLS_PEM_WRITE_C)
    TEST_ASSERT( mbedtls_pk_write_key_pem( &pk, NULL, 0 ) ==
                 MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL );

    TEST_ASSERT( mbedtls_pk_write_pubkey_pem( &pk, NULL, 0 ) ==
                 MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL );
#endif /* MBEDTLS_PEM_WRITE_C */

exit:
    mbedtls_pk_free( &pk );
}

void test_valid_parameters_pkwrite_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};

    test_valid_parameters_pkwrite( &data0 );
}
#endif /* MBEDTLS_PK_WRITE_C */
#if defined(MBEDTLS_CHECK_PARAMS)
#if !defined(MBEDTLS_PARAM_FAILED_ALT)
#line 197 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_invalid_parameters( )
{
    size_t len;
    unsigned char *null_buf = NULL;
    unsigned char buf[1];
    unsigned char *p = buf;
    char str[1] = {0};
    mbedtls_pk_context pk;
    mbedtls_md_type_t valid_md = MBEDTLS_MD_SHA256;
    void *options = buf;

    (void) null_buf;
    (void) p;
    (void) str;

    mbedtls_pk_init( &pk );

    TEST_INVALID_PARAM( mbedtls_pk_init( NULL ) );

#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ECP_RESTARTABLE)
    TEST_INVALID_PARAM( mbedtls_pk_restart_init( NULL ) );
#endif

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_setup( NULL, NULL ) );

#if defined(MBEDTLS_PK_RSA_ALT_SUPPORT)
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_setup_rsa_alt( NULL, buf,
                                                      NULL, NULL, NULL ) );
#endif

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify_restartable( NULL,
                                                           MBEDTLS_MD_NONE,
                                                           buf, sizeof( buf ),
                                                           buf, sizeof( buf ),
                                                           NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify_restartable( &pk,
                                                           MBEDTLS_MD_NONE,
                                                           NULL, sizeof( buf ),
                                                           buf, sizeof( buf ),
                                                           NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify_restartable( &pk,
                                                           valid_md,
                                                           NULL, 0,
                                                           buf, sizeof( buf ),
                                                           NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify_restartable( &pk,
                                                           MBEDTLS_MD_NONE,
                                                           buf, sizeof( buf ),
                                                           NULL, sizeof( buf ),
                                                           NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify( NULL,
                                               MBEDTLS_MD_NONE,
                                               buf, sizeof( buf ),
                                               buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify( &pk,
                                               MBEDTLS_MD_NONE,
                                               NULL, sizeof( buf ),
                                               buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify( &pk,
                                               valid_md,
                                               NULL, 0,
                                               buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify( &pk,
                                               MBEDTLS_MD_NONE,
                                               buf, sizeof( buf ),
                                               NULL, sizeof( buf ) ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify_ext( MBEDTLS_PK_NONE, options,
                                                   NULL,
                                                   MBEDTLS_MD_NONE,
                                                   buf, sizeof( buf ),
                                                   buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify_ext( MBEDTLS_PK_NONE, options,
                                                   &pk,
                                                   MBEDTLS_MD_NONE,
                                                   NULL, sizeof( buf ),
                                                   buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify_ext( MBEDTLS_PK_NONE, options,
                                                   &pk,
                                                   valid_md,
                                                   NULL, 0,
                                                   buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_verify_ext( MBEDTLS_PK_NONE, options,
                                                   &pk,
                                                   MBEDTLS_MD_NONE,
                                                   buf, sizeof( buf ),
                                                   NULL, sizeof( buf ) ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_sign_restartable( NULL,
                                                         MBEDTLS_MD_NONE,
                                                         buf, sizeof( buf ),
                                                         buf, &len,
                                                         rnd_std_rand, NULL,
                                                         NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_sign_restartable( &pk,
                                                         MBEDTLS_MD_NONE,
                                                         NULL, sizeof( buf ),
                                                         buf, &len,
                                                         rnd_std_rand, NULL,
                                                         NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_sign_restartable( &pk,
                                                         valid_md,
                                                         NULL, 0,
                                                         buf, &len,
                                                         rnd_std_rand, NULL,
                                                         NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_sign_restartable( &pk,
                                                         MBEDTLS_MD_NONE,
                                                         buf, sizeof( buf ),
                                                         NULL, &len,
                                                         rnd_std_rand, NULL,
                                                         NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_sign( NULL,
                                             MBEDTLS_MD_NONE,
                                             buf, sizeof( buf ),
                                             buf, &len,
                                             rnd_std_rand, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_sign( &pk,
                                             MBEDTLS_MD_NONE,
                                             NULL, sizeof( buf ),
                                             buf, &len,
                                             rnd_std_rand, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_sign( &pk,
                                             valid_md,
                                             NULL, 0,
                                             buf, &len,
                                             rnd_std_rand, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_sign( &pk,
                                             MBEDTLS_MD_NONE,
                                             buf, sizeof( buf ),
                                             NULL, &len,
                                             rnd_std_rand, NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_decrypt( NULL,
                                                buf, sizeof( buf ),
                                                buf, &len, sizeof( buf ),
                                                rnd_std_rand, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_decrypt( &pk,
                                                NULL, sizeof( buf ),
                                                buf, &len, sizeof( buf ),
                                                rnd_std_rand, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_decrypt( &pk,
                                                buf, sizeof( buf ),
                                                NULL, &len, sizeof( buf ),
                                                rnd_std_rand, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_decrypt( &pk,
                                                buf, sizeof( buf ),
                                                buf, NULL, sizeof( buf ),
                                                rnd_std_rand, NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_encrypt( NULL,
                                                buf, sizeof( buf ),
                                                buf, &len, sizeof( buf ),
                                                rnd_std_rand, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_encrypt( &pk,
                                                NULL, sizeof( buf ),
                                                buf, &len, sizeof( buf ),
                                                rnd_std_rand, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_encrypt( &pk,
                                                buf, sizeof( buf ),
                                                NULL, &len, sizeof( buf ),
                                                rnd_std_rand, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_encrypt( &pk,
                                                buf, sizeof( buf ),
                                                buf, NULL, sizeof( buf ),
                                                rnd_std_rand, NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_check_pair( NULL, &pk ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_check_pair( &pk, NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_debug( NULL, NULL ) );

#if defined(MBEDTLS_PK_PARSE_C)
#if defined(MBEDTLS_FS_IO)
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_load_file( NULL, &p, &len ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_load_file( str, NULL, &len ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_load_file( str, &p, NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_keyfile( NULL, str, NULL ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_keyfile( &pk, NULL, NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_public_keyfile( NULL, str ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_public_keyfile( &pk, NULL ) );
#endif

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_subpubkey( NULL, buf, &pk ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_subpubkey( &null_buf, buf, &pk ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_subpubkey( &p, NULL, &pk ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_subpubkey( &p, buf, NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_key( NULL,
                                                  buf, sizeof( buf ),
                                                  buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_key( &pk,
                                                  NULL, sizeof( buf ),
                                                  buf, sizeof( buf ) ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_public_key( NULL,
                                                         buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_parse_public_key( &pk,
                                                         NULL, sizeof( buf ) ) );
#endif /* MBEDTLS_PK_PARSE_C */

#if defined(MBEDTLS_PK_WRITE_C)
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_pubkey( NULL, p, &pk ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_pubkey( &null_buf, p, &pk ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_pubkey( &p, NULL, &pk ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_pubkey( &p, p, NULL ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_pubkey_der( NULL,
                                                         buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_pubkey_der( &pk,
                                                         NULL, sizeof( buf ) ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_key_der( NULL,
                                                      buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_key_der( &pk,
                                                      NULL, sizeof( buf ) ) );

#if defined(MBEDTLS_PEM_WRITE_C)
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_pubkey_pem( NULL,
                                                         buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_pubkey_pem( &pk,
                                                         NULL, sizeof( buf ) ) );

    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_key_pem( NULL,
                                                      buf, sizeof( buf ) ) );
    TEST_INVALID_PARAM_RET( MBEDTLS_ERR_PK_BAD_INPUT_DATA,
                            mbedtls_pk_write_key_pem( &pk,
                                                      NULL, sizeof( buf ) ) );
#endif /* MBEDTLS_PEM_WRITE_C */

#endif /* MBEDTLS_PK_WRITE_C */
exit:
    ;
}

void test_invalid_parameters_wrapper( void ** params )
{
    (void)params;

    test_invalid_parameters(  );
}
#endif /* !MBEDTLS_PARAM_FAILED_ALT */
#endif /* MBEDTLS_CHECK_PARAMS */
#line 495 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_utils( int type, int size, int len, char * name )
{
    mbedtls_pk_context pk;

    mbedtls_pk_init( &pk );

    TEST_ASSERT( mbedtls_pk_setup( &pk, mbedtls_pk_info_from_type( type ) ) == 0 );
    TEST_ASSERT( pk_genkey( &pk ) == 0 );

    TEST_ASSERT( (int) mbedtls_pk_get_type( &pk ) == type );
    TEST_ASSERT( mbedtls_pk_can_do( &pk, type ) );
    TEST_ASSERT( mbedtls_pk_get_bitlen( &pk ) == (unsigned) size );
    TEST_ASSERT( mbedtls_pk_get_len( &pk ) == (unsigned) len );
    TEST_ASSERT( strcmp( mbedtls_pk_get_name( &pk), name ) == 0 );

exit:
    mbedtls_pk_free( &pk );
}

void test_pk_utils_wrapper( void ** params )
{

    test_pk_utils( *( (int *) params[0] ), *( (int *) params[1] ), *( (int *) params[2] ), (char *) params[3] );
}
#if defined(MBEDTLS_PK_PARSE_C)
#if defined(MBEDTLS_FS_IO)
#line 516 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_mbedtls_pk_check_pair( char * pub_file, char * prv_file, int ret )
{
    mbedtls_pk_context pub, prv, alt;

    mbedtls_pk_init( &pub );
    mbedtls_pk_init( &prv );
    mbedtls_pk_init( &alt );

    TEST_ASSERT( mbedtls_pk_parse_public_keyfile( &pub, pub_file ) == 0 );
    TEST_ASSERT( mbedtls_pk_parse_keyfile( &prv, prv_file, NULL ) == 0 );

    TEST_ASSERT( mbedtls_pk_check_pair( &pub, &prv ) == ret );

#if defined(MBEDTLS_RSA_C) && defined(MBEDTLS_PK_RSA_ALT_SUPPORT)
    if( mbedtls_pk_get_type( &prv ) == MBEDTLS_PK_RSA )
    {
        TEST_ASSERT( mbedtls_pk_setup_rsa_alt( &alt, mbedtls_pk_rsa( prv ),
                     mbedtls_rsa_decrypt_func, mbedtls_rsa_sign_func,
                     mbedtls_rsa_key_len_func ) == 0 );
        TEST_ASSERT( mbedtls_pk_check_pair( &pub, &alt ) == ret );
    }
#endif

    mbedtls_pk_free( &pub );
    mbedtls_pk_free( &prv );
    mbedtls_pk_free( &alt );
exit:
    ;
}

void test_mbedtls_pk_check_pair_wrapper( void ** params )
{

    test_mbedtls_pk_check_pair( (char *) params[0], (char *) params[1], *( (int *) params[2] ) );
}
#endif /* MBEDTLS_FS_IO */
#endif /* MBEDTLS_PK_PARSE_C */
#if defined(MBEDTLS_RSA_C)
#line 546 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_rsa_verify_test_vec( data_t * message_str, int digest, int mod,
                             int radix_N, char * input_N, int radix_E,
                             char * input_E, data_t * result_str,
                             int result )
{
    unsigned char hash_result[MBEDTLS_MD_MAX_SIZE];
    mbedtls_rsa_context *rsa;
    mbedtls_pk_context pk;
    mbedtls_pk_restart_ctx *rs_ctx = NULL;
#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ECP_RESTARTABLE)
    mbedtls_pk_restart_ctx ctx;

    rs_ctx = &ctx;
    mbedtls_pk_restart_init( rs_ctx );
    // this setting would ensure restart would happen if ECC was used
    mbedtls_ecp_set_max_ops( 1 );
#endif

    mbedtls_pk_init( &pk );

    memset( hash_result, 0x00, MBEDTLS_MD_MAX_SIZE );

    TEST_ASSERT( mbedtls_pk_setup( &pk, mbedtls_pk_info_from_type( MBEDTLS_PK_RSA ) ) == 0 );
    rsa = mbedtls_pk_rsa( pk );

    rsa->len = mod / 8;
    TEST_ASSERT( mbedtls_mpi_read_string( &rsa->N, radix_N, input_N ) == 0 );
    TEST_ASSERT( mbedtls_mpi_read_string( &rsa->E, radix_E, input_E ) == 0 );


    if( mbedtls_md_info_from_type( digest ) != NULL )
        TEST_ASSERT( mbedtls_md( mbedtls_md_info_from_type( digest ), message_str->x, message_str->len, hash_result ) == 0 );

    TEST_ASSERT( mbedtls_pk_verify( &pk, digest, hash_result, 0,
                            result_str->x, mbedtls_pk_get_len( &pk ) ) == result );

    TEST_ASSERT( mbedtls_pk_verify_restartable( &pk, digest, hash_result, 0,
                    result_str->x, mbedtls_pk_get_len( &pk ), rs_ctx ) == result );

exit:
#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ECP_RESTARTABLE)
    mbedtls_pk_restart_free( rs_ctx );
#endif
    mbedtls_pk_free( &pk );
}

void test_pk_rsa_verify_test_vec_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};
    data_t data8 = {(uint8_t *) params[8], *( (uint32_t *) params[9] )};

    test_pk_rsa_verify_test_vec( &data0, *( (int *) params[2] ), *( (int *) params[3] ), *( (int *) params[4] ), (char *) params[5], *( (int *) params[6] ), (char *) params[7], &data8, *( (int *) params[10] ) );
}
#endif /* MBEDTLS_RSA_C */
#if defined(MBEDTLS_RSA_C)
#line 594 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_rsa_verify_ext_test_vec( data_t * message_str, int digest,
                                 int mod, int radix_N, char * input_N,
                                 int radix_E, char * input_E,
                                 data_t * result_str, int pk_type,
                                 int mgf1_hash_id, int salt_len, int result )
{
    unsigned char hash_result[MBEDTLS_MD_MAX_SIZE];
    mbedtls_rsa_context *rsa;
    mbedtls_pk_context pk;
    mbedtls_pk_rsassa_pss_options pss_opts;
    void *options;
    size_t hash_len;

    mbedtls_pk_init( &pk );

    memset( hash_result, 0x00, sizeof( hash_result ) );

    TEST_ASSERT( mbedtls_pk_setup( &pk, mbedtls_pk_info_from_type( MBEDTLS_PK_RSA ) ) == 0 );
    rsa = mbedtls_pk_rsa( pk );

    rsa->len = mod / 8;
    TEST_ASSERT( mbedtls_mpi_read_string( &rsa->N, radix_N, input_N ) == 0 );
    TEST_ASSERT( mbedtls_mpi_read_string( &rsa->E, radix_E, input_E ) == 0 );


    if( digest != MBEDTLS_MD_NONE )
    {
        TEST_ASSERT( mbedtls_md( mbedtls_md_info_from_type( digest ),
                     message_str->x, message_str->len, hash_result ) == 0 );
        hash_len = 0;
    }
    else
    {
        memcpy( hash_result, message_str->x, message_str->len );
        hash_len = message_str->len;
    }

    if( mgf1_hash_id < 0 )
    {
        options = NULL;
    }
    else
    {
        options = &pss_opts;

        pss_opts.mgf1_hash_id = mgf1_hash_id;
        pss_opts.expected_salt_len = salt_len;
    }

    TEST_ASSERT( mbedtls_pk_verify_ext( pk_type, options, &pk,
                                digest, hash_result, hash_len,
                                result_str->x, mbedtls_pk_get_len( &pk ) ) == result );

exit:
    mbedtls_pk_free( &pk );
}

void test_pk_rsa_verify_ext_test_vec_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};
    data_t data8 = {(uint8_t *) params[8], *( (uint32_t *) params[9] )};

    test_pk_rsa_verify_ext_test_vec( &data0, *( (int *) params[2] ), *( (int *) params[3] ), *( (int *) params[4] ), (char *) params[5], *( (int *) params[6] ), (char *) params[7], &data8, *( (int *) params[10] ), *( (int *) params[11] ), *( (int *) params[12] ), *( (int *) params[13] ) );
}
#endif /* MBEDTLS_RSA_C */
#if defined(MBEDTLS_ECDSA_C)
#line 653 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_ec_test_vec( int type, int id, data_t * key, data_t * hash,
                     data_t * sig, int ret )
{
    mbedtls_pk_context pk;
    mbedtls_ecp_keypair *eckey;

    mbedtls_pk_init( &pk );


    TEST_ASSERT( mbedtls_pk_setup( &pk, mbedtls_pk_info_from_type( type ) ) == 0 );

    TEST_ASSERT( mbedtls_pk_can_do( &pk, MBEDTLS_PK_ECDSA ) );
    eckey = mbedtls_pk_ec( pk );

    TEST_ASSERT( mbedtls_ecp_group_load( &eckey->grp, id ) == 0 );
    TEST_ASSERT( mbedtls_ecp_point_read_binary( &eckey->grp, &eckey->Q,
                                        key->x, key->len ) == 0 );

    TEST_ASSERT( mbedtls_pk_verify( &pk, MBEDTLS_MD_NONE,
                            hash->x, hash->len, sig->x, sig->len ) == ret );

exit:
    mbedtls_pk_free( &pk );
}

void test_pk_ec_test_vec_wrapper( void ** params )
{
    data_t data2 = {(uint8_t *) params[2], *( (uint32_t *) params[3] )};
    data_t data4 = {(uint8_t *) params[4], *( (uint32_t *) params[5] )};
    data_t data6 = {(uint8_t *) params[6], *( (uint32_t *) params[7] )};

    test_pk_ec_test_vec( *( (int *) params[0] ), *( (int *) params[1] ), &data2, &data4, &data6, *( (int *) params[8] ) );
}
#endif /* MBEDTLS_ECDSA_C */
#if defined(MBEDTLS_ECP_RESTARTABLE)
#if defined(MBEDTLS_ECDSA_C)
#if defined(MBEDTLS_ECDSA_DETERMINISTIC)
#line 680 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_sign_verify_restart( int pk_type, int grp_id, char *d_str,
                              char *QX_str, char *QY_str,
                              int md_alg, char *msg, data_t *sig_check,
                              int max_ops, int min_restart, int max_restart )
{
    int ret, cnt_restart;
    mbedtls_pk_restart_ctx rs_ctx;
    mbedtls_pk_context prv, pub;
    unsigned char hash[MBEDTLS_MD_MAX_SIZE];
    unsigned char sig[MBEDTLS_ECDSA_MAX_LEN];
    size_t hlen, slen;
    const mbedtls_md_info_t *md_info;

    mbedtls_pk_restart_init( &rs_ctx );
    mbedtls_pk_init( &prv );
    mbedtls_pk_init( &pub );
    memset( hash, 0, sizeof( hash ) );
    memset( sig, 0, sizeof( sig ) );

    TEST_ASSERT( mbedtls_pk_setup( &prv, mbedtls_pk_info_from_type( pk_type ) ) == 0 );
    TEST_ASSERT( mbedtls_ecp_group_load( &mbedtls_pk_ec( prv )->grp, grp_id ) == 0 );
    TEST_ASSERT( mbedtls_mpi_read_string( &mbedtls_pk_ec( prv )->d, 16, d_str ) == 0 );

    TEST_ASSERT( mbedtls_pk_setup( &pub, mbedtls_pk_info_from_type( pk_type ) ) == 0 );
    TEST_ASSERT( mbedtls_ecp_group_load( &mbedtls_pk_ec( pub )->grp, grp_id ) == 0 );
    TEST_ASSERT( mbedtls_ecp_point_read_string( &mbedtls_pk_ec( pub )->Q, 16, QX_str, QY_str ) == 0 );

    md_info = mbedtls_md_info_from_type( md_alg );
    TEST_ASSERT( md_info != NULL );

    hlen = mbedtls_md_get_size( md_info );
    TEST_ASSERT( mbedtls_md( md_info,
                             (const unsigned char *) msg, strlen( msg ),
                             hash ) == 0 );

    mbedtls_ecp_set_max_ops( max_ops );

    slen = sizeof( sig );
    cnt_restart = 0;
    do {
        ret = mbedtls_pk_sign_restartable( &prv, md_alg, hash, hlen,
                                            sig, &slen, NULL, NULL, &rs_ctx );
    } while( ret == MBEDTLS_ERR_ECP_IN_PROGRESS && ++cnt_restart );

    TEST_ASSERT( ret == 0 );
    TEST_ASSERT( slen == sig_check->len );
    TEST_ASSERT( memcmp( sig, sig_check->x, slen ) == 0 );

    TEST_ASSERT( cnt_restart >= min_restart );
    TEST_ASSERT( cnt_restart <= max_restart );

    cnt_restart = 0;
    do {
        ret = mbedtls_pk_verify_restartable( &pub, md_alg,
                                 hash, hlen, sig, slen, &rs_ctx );
    } while( ret == MBEDTLS_ERR_ECP_IN_PROGRESS && ++cnt_restart );

    TEST_ASSERT( ret == 0 );
    TEST_ASSERT( cnt_restart >= min_restart );
    TEST_ASSERT( cnt_restart <= max_restart );

    hash[0]++;
    do {
        ret = mbedtls_pk_verify_restartable( &pub, md_alg,
                                 hash, hlen, sig, slen, &rs_ctx );
    } while( ret == MBEDTLS_ERR_ECP_IN_PROGRESS );
    TEST_ASSERT( ret != 0 );
    hash[0]--;

    sig[0]++;
    do {
        ret = mbedtls_pk_verify_restartable( &pub, md_alg,
                                 hash, hlen, sig, slen, &rs_ctx );
    } while( ret == MBEDTLS_ERR_ECP_IN_PROGRESS );
    TEST_ASSERT( ret != 0 );
    sig[0]--;

    /* Do we leak memory when aborting? try verify then sign
     * This test only makes sense when we actually restart */
    if( min_restart > 0 )
    {
        ret = mbedtls_pk_verify_restartable( &pub, md_alg,
                                 hash, hlen, sig, slen, &rs_ctx );
        TEST_ASSERT( ret == MBEDTLS_ERR_ECP_IN_PROGRESS );
        mbedtls_pk_restart_free( &rs_ctx );

        slen = sizeof( sig );
        ret = mbedtls_pk_sign_restartable( &prv, md_alg, hash, hlen,
                                            sig, &slen, NULL, NULL, &rs_ctx );
        TEST_ASSERT( ret == MBEDTLS_ERR_ECP_IN_PROGRESS );
    }

exit:
    mbedtls_pk_restart_free( &rs_ctx );
    mbedtls_pk_free( &prv );
    mbedtls_pk_free( &pub );
}

void test_pk_sign_verify_restart_wrapper( void ** params )
{
    data_t data7 = {(uint8_t *) params[7], *( (uint32_t *) params[8] )};

    test_pk_sign_verify_restart( *( (int *) params[0] ), *( (int *) params[1] ), (char *) params[2], (char *) params[3], (char *) params[4], *( (int *) params[5] ), (char *) params[6], &data7, *( (int *) params[9] ), *( (int *) params[10] ), *( (int *) params[11] ) );
}
#endif /* MBEDTLS_ECDSA_DETERMINISTIC */
#endif /* MBEDTLS_ECDSA_C */
#endif /* MBEDTLS_ECP_RESTARTABLE */
#if defined(MBEDTLS_SHA256_C)
#line 780 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_sign_verify( int type, int sign_ret, int verify_ret )
{
    mbedtls_pk_context pk;
    unsigned char hash[50], sig[5000];
    size_t sig_len;
    void *rs_ctx = NULL;
#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ECP_RESTARTABLE)
    mbedtls_pk_restart_ctx ctx;

    rs_ctx = &ctx;
    mbedtls_pk_restart_init( rs_ctx );
    /* This value is large enough that the operation will complete in one run.
     * See comments at the top of ecp_test_vect_restart in
     * test_suite_ecp.function for estimates of operation counts. */
    mbedtls_ecp_set_max_ops( 42000 );
#endif

    mbedtls_pk_init( &pk );

    memset( hash, 0x2a, sizeof hash );
    memset( sig, 0, sizeof sig );

    TEST_ASSERT( mbedtls_pk_setup( &pk, mbedtls_pk_info_from_type( type ) ) == 0 );
    TEST_ASSERT( pk_genkey( &pk ) == 0 );

    TEST_ASSERT( mbedtls_pk_sign_restartable( &pk, MBEDTLS_MD_SHA256,
                 hash, sizeof hash, sig, &sig_len,
                 rnd_std_rand, NULL, rs_ctx ) == sign_ret );

    TEST_ASSERT( mbedtls_pk_verify( &pk, MBEDTLS_MD_SHA256,
                            hash, sizeof hash, sig, sig_len ) == verify_ret );

    if( verify_ret == 0 )
    {
        hash[0]++;
        TEST_ASSERT( mbedtls_pk_verify( &pk, MBEDTLS_MD_SHA256,
                                hash, sizeof hash, sig, sig_len ) != 0 );
        hash[0]--;

        sig[0]++;
        TEST_ASSERT( mbedtls_pk_verify( &pk, MBEDTLS_MD_SHA256,
                                hash, sizeof hash, sig, sig_len ) != 0 );
        sig[0]--;
    }

    TEST_ASSERT( mbedtls_pk_sign( &pk, MBEDTLS_MD_SHA256, hash, sizeof hash,
                          sig, &sig_len, rnd_std_rand, NULL ) == sign_ret );

    TEST_ASSERT( mbedtls_pk_verify_restartable( &pk, MBEDTLS_MD_SHA256,
                 hash, sizeof hash, sig, sig_len, rs_ctx ) == verify_ret );

    if( verify_ret == 0 )
    {
        hash[0]++;
        TEST_ASSERT( mbedtls_pk_verify_restartable( &pk, MBEDTLS_MD_SHA256,
                     hash, sizeof hash, sig, sig_len, rs_ctx ) != 0 );
        hash[0]--;

        sig[0]++;
        TEST_ASSERT( mbedtls_pk_verify_restartable( &pk, MBEDTLS_MD_SHA256,
                     hash, sizeof hash, sig, sig_len, rs_ctx ) != 0 );
        sig[0]--;
    }

exit:
#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ECP_RESTARTABLE)
    mbedtls_pk_restart_free( rs_ctx );
#endif
    mbedtls_pk_free( &pk );
}

void test_pk_sign_verify_wrapper( void ** params )
{

    test_pk_sign_verify( *( (int *) params[0] ), *( (int *) params[1] ), *( (int *) params[2] ) );
}
#endif /* MBEDTLS_SHA256_C */
#if defined(MBEDTLS_RSA_C)
#line 853 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_rsa_encrypt_test_vec( data_t * message, int mod, int radix_N,
                              char * input_N, int radix_E, char * input_E,
                              data_t * result, int ret )
{
    unsigned char output[300];
    rnd_pseudo_info rnd_info;
    mbedtls_rsa_context *rsa;
    mbedtls_pk_context pk;
    size_t olen;

    memset( &rnd_info,  0, sizeof( rnd_pseudo_info ) );
    memset( output,     0, sizeof( output ) );


    mbedtls_pk_init( &pk );
    TEST_ASSERT( mbedtls_pk_setup( &pk, mbedtls_pk_info_from_type( MBEDTLS_PK_RSA ) ) == 0 );
    rsa = mbedtls_pk_rsa( pk );

    rsa->len = mod / 8;
    TEST_ASSERT( mbedtls_mpi_read_string( &rsa->N, radix_N, input_N ) == 0 );
    TEST_ASSERT( mbedtls_mpi_read_string( &rsa->E, radix_E, input_E ) == 0 );

    TEST_ASSERT( mbedtls_pk_encrypt( &pk, message->x, message->len,
                             output, &olen, sizeof( output ),
                             rnd_pseudo_rand, &rnd_info ) == ret );
    TEST_ASSERT( olen == result->len );
    TEST_ASSERT( memcmp( output, result->x, olen ) == 0 );

exit:
    mbedtls_pk_free( &pk );
}

void test_pk_rsa_encrypt_test_vec_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};
    data_t data7 = {(uint8_t *) params[7], *( (uint32_t *) params[8] )};

    test_pk_rsa_encrypt_test_vec( &data0, *( (int *) params[2] ), *( (int *) params[3] ), (char *) params[4], *( (int *) params[5] ), (char *) params[6], &data7, *( (int *) params[9] ) );
}
#endif /* MBEDTLS_RSA_C */
#if defined(MBEDTLS_RSA_C)
#line 887 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_rsa_decrypt_test_vec( data_t * cipher, int mod, int radix_P,
                              char * input_P, int radix_Q, char * input_Q,
                              int radix_N, char * input_N, int radix_E,
                              char * input_E, data_t * clear, int ret )
{
    unsigned char output[256];
    rnd_pseudo_info rnd_info;
    mbedtls_mpi N, P, Q, E;
    mbedtls_rsa_context *rsa;
    mbedtls_pk_context pk;
    size_t olen;

    mbedtls_pk_init( &pk );
    mbedtls_mpi_init( &N ); mbedtls_mpi_init( &P );
    mbedtls_mpi_init( &Q ); mbedtls_mpi_init( &E );

    memset( &rnd_info,  0, sizeof( rnd_pseudo_info ) );


    /* init pk-rsa context */
    TEST_ASSERT( mbedtls_pk_setup( &pk, mbedtls_pk_info_from_type( MBEDTLS_PK_RSA ) ) == 0 );
    rsa = mbedtls_pk_rsa( pk );

    /* load public key */
    TEST_ASSERT( mbedtls_mpi_read_string( &N, radix_N, input_N ) == 0 );
    TEST_ASSERT( mbedtls_mpi_read_string( &E, radix_E, input_E ) == 0 );

    /* load private key */
    TEST_ASSERT( mbedtls_mpi_read_string( &P, radix_P, input_P ) == 0 );
    TEST_ASSERT( mbedtls_mpi_read_string( &Q, radix_Q, input_Q ) == 0 );
    TEST_ASSERT( mbedtls_rsa_import( rsa, &N, &P, &Q, NULL, &E ) == 0 );
    TEST_ASSERT( mbedtls_rsa_get_len( rsa ) == (size_t) ( mod / 8 ) );
    TEST_ASSERT( mbedtls_rsa_complete( rsa ) == 0 );

    /* decryption test */
    memset( output, 0, sizeof( output ) );
    olen = 0;
    TEST_ASSERT( mbedtls_pk_decrypt( &pk, cipher->x, cipher->len,
                             output, &olen, sizeof( output ),
                             rnd_pseudo_rand, &rnd_info ) == ret );
    if( ret == 0 )
    {
        TEST_ASSERT( olen == clear->len );
        TEST_ASSERT( memcmp( output, clear->x, olen ) == 0 );
    }

exit:
    mbedtls_mpi_free( &N ); mbedtls_mpi_free( &P );
    mbedtls_mpi_free( &Q ); mbedtls_mpi_free( &E );
    mbedtls_pk_free( &pk );
}

void test_pk_rsa_decrypt_test_vec_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};
    data_t data11 = {(uint8_t *) params[11], *( (uint32_t *) params[12] )};

    test_pk_rsa_decrypt_test_vec( &data0, *( (int *) params[2] ), *( (int *) params[3] ), (char *) params[4], *( (int *) params[5] ), (char *) params[6], *( (int *) params[7] ), (char *) params[8], *( (int *) params[9] ), (char *) params[10], &data11, *( (int *) params[13] ) );
}
#endif /* MBEDTLS_RSA_C */
#line 941 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_ec_nocrypt( int type )
{
    mbedtls_pk_context pk;
    unsigned char output[100];
    unsigned char input[100];
    rnd_pseudo_info rnd_info;
    size_t olen = 0;
    int ret = MBEDTLS_ERR_PK_TYPE_MISMATCH;

    mbedtls_pk_init( &pk );

    memset( &rnd_info,  0, sizeof( rnd_pseudo_info ) );
    memset( output,     0, sizeof( output ) );
    memset( input,      0, sizeof( input ) );

    TEST_ASSERT( mbedtls_pk_setup( &pk, mbedtls_pk_info_from_type( type ) ) == 0 );

    TEST_ASSERT( mbedtls_pk_encrypt( &pk, input, sizeof( input ),
                             output, &olen, sizeof( output ),
                             rnd_pseudo_rand, &rnd_info ) == ret );

    TEST_ASSERT( mbedtls_pk_decrypt( &pk, input, sizeof( input ),
                             output, &olen, sizeof( output ),
                             rnd_pseudo_rand, &rnd_info ) == ret );

exit:
    mbedtls_pk_free( &pk );
}

void test_pk_ec_nocrypt_wrapper( void ** params )
{

    test_pk_ec_nocrypt( *( (int *) params[0] ) );
}
#if defined(MBEDTLS_RSA_C)
#line 972 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_rsa_overflow( )
{
    mbedtls_pk_context pk;
    size_t hash_len = SIZE_MAX, sig_len = SIZE_MAX;
    unsigned char hash[50], sig[100];

    if( SIZE_MAX <= UINT_MAX )
        return;

    memset( hash, 0x2a, sizeof hash );
    memset( sig, 0, sizeof sig );

    mbedtls_pk_init( &pk );

    TEST_ASSERT( mbedtls_pk_setup( &pk,
                 mbedtls_pk_info_from_type( MBEDTLS_PK_RSA ) ) == 0 );

#if defined(MBEDTLS_PKCS1_V21)
    TEST_ASSERT( mbedtls_pk_verify_ext( MBEDTLS_PK_RSASSA_PSS, NULL, &pk,
                    MBEDTLS_MD_NONE, hash, hash_len, sig, sig_len ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );
#endif /* MBEDTLS_PKCS1_V21 */

    TEST_ASSERT( mbedtls_pk_verify( &pk, MBEDTLS_MD_NONE, hash, hash_len,
                    sig, sig_len ) == MBEDTLS_ERR_PK_BAD_INPUT_DATA );

    TEST_ASSERT( mbedtls_pk_sign( &pk, MBEDTLS_MD_NONE, hash, hash_len, sig, &sig_len,
                    rnd_std_rand, NULL ) == MBEDTLS_ERR_PK_BAD_INPUT_DATA );

exit:
    mbedtls_pk_free( &pk );
}

void test_pk_rsa_overflow_wrapper( void ** params )
{
    (void)params;

    test_pk_rsa_overflow(  );
}
#endif /* MBEDTLS_RSA_C */
#if defined(MBEDTLS_RSA_C)
#if defined(MBEDTLS_PK_RSA_ALT_SUPPORT)
#line 1007 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_pk.function"
void test_pk_rsa_alt(  )
{
    /*
     * An rsa_alt context can only do private operations (decrypt, sign).
     * Test it against the public operations (encrypt, verify) of a
     * corresponding rsa context.
     */
    mbedtls_rsa_context raw;
    mbedtls_pk_context rsa, alt;
    mbedtls_pk_debug_item dbg_items[10];
    unsigned char hash[50], sig[64];
    unsigned char msg[50], ciph[64], test[50];
    size_t sig_len, ciph_len, test_len;
    int ret = MBEDTLS_ERR_PK_TYPE_MISMATCH;

    mbedtls_rsa_init( &raw, MBEDTLS_RSA_PKCS_V15, MBEDTLS_MD_NONE );
    mbedtls_pk_init( &rsa ); mbedtls_pk_init( &alt );

    memset( hash, 0x2a, sizeof hash );
    memset( sig, 0, sizeof sig );
    memset( msg, 0x2a, sizeof msg );
    memset( ciph, 0, sizeof ciph );
    memset( test, 0, sizeof test );

    /* Initiliaze PK RSA context with random key */
    TEST_ASSERT( mbedtls_pk_setup( &rsa,
                              mbedtls_pk_info_from_type( MBEDTLS_PK_RSA ) ) == 0 );
    TEST_ASSERT( pk_genkey( &rsa ) == 0 );

    /* Extract key to the raw rsa context */
    TEST_ASSERT( mbedtls_rsa_copy( &raw, mbedtls_pk_rsa( rsa ) ) == 0 );

    /* Initialize PK RSA_ALT context */
    TEST_ASSERT( mbedtls_pk_setup_rsa_alt( &alt, (void *) &raw,
                 mbedtls_rsa_decrypt_func, mbedtls_rsa_sign_func, mbedtls_rsa_key_len_func ) == 0 );

    /* Test administrative functions */
    TEST_ASSERT( mbedtls_pk_can_do( &alt, MBEDTLS_PK_RSA ) );
    TEST_ASSERT( mbedtls_pk_get_bitlen( &alt ) == RSA_KEY_SIZE );
    TEST_ASSERT( mbedtls_pk_get_len( &alt ) == RSA_KEY_LEN );
    TEST_ASSERT( mbedtls_pk_get_type( &alt ) == MBEDTLS_PK_RSA_ALT );
    TEST_ASSERT( strcmp( mbedtls_pk_get_name( &alt ), "RSA-alt" ) == 0 );

    /* Test signature */
#if SIZE_MAX > UINT_MAX
    TEST_ASSERT( mbedtls_pk_sign( &alt, MBEDTLS_MD_NONE, hash, SIZE_MAX,
                                  sig, &sig_len, rnd_std_rand, NULL ) ==
                 MBEDTLS_ERR_PK_BAD_INPUT_DATA );
#endif /* SIZE_MAX > UINT_MAX */
    TEST_ASSERT( mbedtls_pk_sign( &alt, MBEDTLS_MD_NONE, hash, sizeof hash,
                                  sig, &sig_len, rnd_std_rand, NULL ) == 0 );
    TEST_ASSERT( sig_len == RSA_KEY_LEN );
    TEST_ASSERT( mbedtls_pk_verify( &rsa, MBEDTLS_MD_NONE,
                            hash, sizeof hash, sig, sig_len ) == 0 );

    /* Test decrypt */
    TEST_ASSERT( mbedtls_pk_encrypt( &rsa, msg, sizeof msg,
                             ciph, &ciph_len, sizeof ciph,
                             rnd_std_rand, NULL ) == 0 );
    TEST_ASSERT( mbedtls_pk_decrypt( &alt, ciph, ciph_len,
                             test, &test_len, sizeof test,
                             rnd_std_rand, NULL ) == 0 );
    TEST_ASSERT( test_len == sizeof msg );
    TEST_ASSERT( memcmp( test, msg, test_len ) == 0 );

    /* Test forbidden operations */
    TEST_ASSERT( mbedtls_pk_encrypt( &alt, msg, sizeof msg,
                             ciph, &ciph_len, sizeof ciph,
                             rnd_std_rand, NULL ) == ret );
    TEST_ASSERT( mbedtls_pk_verify( &alt, MBEDTLS_MD_NONE,
                            hash, sizeof hash, sig, sig_len ) == ret );
    TEST_ASSERT( mbedtls_pk_debug( &alt, dbg_items ) == ret );

exit:
    mbedtls_rsa_free( &raw );
    mbedtls_pk_free( &rsa ); mbedtls_pk_free( &alt );
}

void test_pk_rsa_alt_wrapper( void ** params )
{
    (void)params;

    test_pk_rsa_alt(  );
}
#endif /* MBEDTLS_PK_RSA_ALT_SUPPORT */
#endif /* MBEDTLS_RSA_C */
#endif /* MBEDTLS_PK_C */


#line 59 "suites/main_test.function"


/*----------------------------------------------------------------------------*/
/* Test dispatch code */


/**
 * \brief       Evaluates an expression/macro into its literal integer value.
 *              For optimizing space for embedded targets each expression/macro
 *              is identified by a unique identifier instead of string literals.
 *              Identifiers and evaluation code is generated by script:
 *              generate_test_code.py
 *
 * \param exp_id    Expression identifier.
 * \param out_value Pointer to int to hold the integer.
 *
 * \return       0 if exp_id is found. 1 otherwise.
 */
int get_expression( int32_t exp_id, int32_t * out_value )
{
    int ret = KEY_VALUE_MAPPING_FOUND;

    (void) exp_id;
    (void) out_value;

    switch( exp_id )
    {

#if defined(MBEDTLS_PK_C)

        case 0:
            {
                *out_value = MBEDTLS_PK_RSA;
            }
            break;
        case 1:
            {
                *out_value = MBEDTLS_PK_ECKEY;
            }
            break;
        case 2:
            {
                *out_value = MBEDTLS_PK_ECKEY_DH;
            }
            break;
        case 3:
            {
                *out_value = MBEDTLS_PK_ECDSA;
            }
            break;
        case 4:
            {
                *out_value = MBEDTLS_MD_SHA1;
            }
            break;
        case 5:
            {
                *out_value = MBEDTLS_ERR_RSA_VERIFY_FAILED;
            }
            break;
        case 6:
            {
                *out_value = MBEDTLS_ECP_DP_SECP192R1;
            }
            break;
        case 7:
            {
                *out_value = MBEDTLS_ERR_ECP_VERIFY_FAILED;
            }
            break;
        case 8:
            {
                *out_value = MBEDTLS_ERR_PK_TYPE_MISMATCH;
            }
            break;
        case 9:
            {
                *out_value = MBEDTLS_ERR_RSA_INVALID_PADDING;
            }
            break;
        case 10:
            {
                *out_value = MBEDTLS_MD_SHA256;
            }
            break;
        case 11:
            {
                *out_value = MBEDTLS_PK_RSASSA_PSS;
            }
            break;
        case 12:
            {
                *out_value = MBEDTLS_RSA_SALT_LEN_ANY;
            }
            break;
        case 13:
            {
                *out_value = MBEDTLS_MD_NONE;
            }
            break;
        case 14:
            {
                *out_value = -1;
            }
            break;
        case 15:
            {
                *out_value = MBEDTLS_ERR_PK_BAD_INPUT_DATA;
            }
            break;
        case 16:
            {
                *out_value = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
            }
            break;
        case 17:
            {
                *out_value = MBEDTLS_ERR_RSA_KEY_CHECK_FAILED;
            }
            break;
        case 18:
            {
                *out_value = MBEDTLS_ECP_DP_SECP256R1;
            }
            break;
#endif

#line 88 "suites/main_test.function"
        default:
           {
                ret = KEY_VALUE_MAPPING_NOT_FOUND;
           }
           break;
    }
    return( ret );
}


/**
 * \brief       Checks if the dependency i.e. the compile flag is set.
 *              For optimizing space for embedded targets each dependency
 *              is identified by a unique identifier instead of string literals.
 *              Identifiers and check code is generated by script:
 *              generate_test_code.py
 *
 * \param exp_id    Dependency identifier.
 *
 * \return       DEPENDENCY_SUPPORTED if set else DEPENDENCY_NOT_SUPPORTED
 */
int dep_check( int dep_id )
{
    int ret = DEPENDENCY_NOT_SUPPORTED;

    (void) dep_id;

    switch( dep_id )
    {

#if defined(MBEDTLS_PK_C)

        case 0:
            {
#if defined(MBEDTLS_RSA_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 1:
            {
#if defined(MBEDTLS_GENPRIME)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 2:
            {
#if defined(MBEDTLS_ECP_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 3:
            {
#if defined(MBEDTLS_ECP_DP_SECP192R1_ENABLED)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 4:
            {
#if defined(MBEDTLS_ECDSA_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 5:
            {
#if defined(MBEDTLS_SHA1_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 6:
            {
#if defined(MBEDTLS_PKCS1_V15)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 7:
            {
#if defined(MBEDTLS_PKCS1_V21)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 8:
            {
#if defined(MBEDTLS_SHA256_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 9:
            {
#if defined(MBEDTLS_ECP_DP_SECP256R1_ENABLED)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 10:
            {
#if defined(MBEDTLS_HAVE_INT64)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
#endif

#line 119 "suites/main_test.function"
        default:
            break;
    }
    return( ret );
}


/**
 * \brief       Function pointer type for test function wrappers.
 *
 *
 * \param void **   Pointer to void pointers. Represents an array of test
 *                  function parameters.
 *
 * \return       void
 */
typedef void (*TestWrapper_t)( void ** );


/**
 * \brief       Table of test function wrappers. Used by dispatch_test().
 *              This table is populated by script:
 *              generate_test_code.py
 *
 */
TestWrapper_t test_funcs[] =
{
/* Function Id: 0 */

#if defined(MBEDTLS_PK_C)
    test_valid_parameters_wrapper,
#else
    NULL,
#endif
/* Function Id: 1 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_PK_WRITE_C)
    test_valid_parameters_pkwrite_wrapper,
#else
    NULL,
#endif
/* Function Id: 2 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_CHECK_PARAMS) && !defined(MBEDTLS_PARAM_FAILED_ALT)
    test_invalid_parameters_wrapper,
#else
    NULL,
#endif
/* Function Id: 3 */

#if defined(MBEDTLS_PK_C)
    test_pk_utils_wrapper,
#else
    NULL,
#endif
/* Function Id: 4 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_PK_PARSE_C) && defined(MBEDTLS_FS_IO)
    test_mbedtls_pk_check_pair_wrapper,
#else
    NULL,
#endif
/* Function Id: 5 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_RSA_C)
    test_pk_rsa_verify_test_vec_wrapper,
#else
    NULL,
#endif
/* Function Id: 6 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_RSA_C)
    test_pk_rsa_verify_ext_test_vec_wrapper,
#else
    NULL,
#endif
/* Function Id: 7 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_ECDSA_C)
    test_pk_ec_test_vec_wrapper,
#else
    NULL,
#endif
/* Function Id: 8 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_ECP_RESTARTABLE) && defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_ECDSA_DETERMINISTIC)
    test_pk_sign_verify_restart_wrapper,
#else
    NULL,
#endif
/* Function Id: 9 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_SHA256_C)
    test_pk_sign_verify_wrapper,
#else
    NULL,
#endif
/* Function Id: 10 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_RSA_C)
    test_pk_rsa_encrypt_test_vec_wrapper,
#else
    NULL,
#endif
/* Function Id: 11 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_RSA_C)
    test_pk_rsa_decrypt_test_vec_wrapper,
#else
    NULL,
#endif
/* Function Id: 12 */

#if defined(MBEDTLS_PK_C)
    test_pk_ec_nocrypt_wrapper,
#else
    NULL,
#endif
/* Function Id: 13 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_RSA_C)
    test_pk_rsa_overflow_wrapper,
#else
    NULL,
#endif
/* Function Id: 14 */

#if defined(MBEDTLS_PK_C) && defined(MBEDTLS_RSA_C) && defined(MBEDTLS_PK_RSA_ALT_SUPPORT)
    test_pk_rsa_alt_wrapper,
#else
    NULL,
#endif

#line 148 "suites/main_test.function"
};

/**
 * \brief        Execute the test function.
 *
 *               This is a wrapper function around the test function execution
 *               to allow the setjmp() call used to catch any calls to the
 *               parameter failure callback, to be used. Calls to setjmp()
 *               can invalidate the state of any local auto variables.
 *
 * \param fp     Function pointer to the test function
 * \param params Parameters to pass
 *
 */
void execute_function_ptr(TestWrapper_t fp, void **params)
{
#if defined(MBEDTLS_CHECK_PARAMS)
    if ( setjmp( param_fail_jmp ) == 0 )
    {
        fp( params );
    }
    else
    {
        /* Unexpected parameter validation error */
        test_info.result = TEST_RESULT_FAILED;
    }

    memset( param_fail_jmp, 0, sizeof(jmp_buf) );
#else
    fp( params );
#endif
}

/**
 * \brief        Dispatches test functions based on function index.
 *
 * \param exp_id    Test function index.
 *
 * \return       DISPATCH_TEST_SUCCESS if found
 *               DISPATCH_TEST_FN_NOT_FOUND if not found
 *               DISPATCH_UNSUPPORTED_SUITE if not compile time enabled.
 */
int dispatch_test( size_t func_idx, void ** params )
{
    int ret = DISPATCH_TEST_SUCCESS;
    TestWrapper_t fp = NULL;

    if ( func_idx < (int)( sizeof( test_funcs ) / sizeof( TestWrapper_t ) ) )
    {
        fp = test_funcs[func_idx];
        if ( fp )
            execute_function_ptr(fp, params);
        else
            ret = DISPATCH_UNSUPPORTED_SUITE;
    }
    else
    {
        ret = DISPATCH_TEST_FN_NOT_FOUND;
    }

    return( ret );
}


/**
 * \brief       Checks if test function is supported
 *
 * \param exp_id    Test function index.
 *
 * \return       DISPATCH_TEST_SUCCESS if found
 *               DISPATCH_TEST_FN_NOT_FOUND if not found
 *               DISPATCH_UNSUPPORTED_SUITE if not compile time enabled.
 */
int check_test( size_t func_idx )
{
    int ret = DISPATCH_TEST_SUCCESS;
    TestWrapper_t fp = NULL;

    if ( func_idx < (int)( sizeof(test_funcs)/sizeof( TestWrapper_t ) ) )
    {
        fp = test_funcs[func_idx];
        if ( fp == NULL )
            ret = DISPATCH_UNSUPPORTED_SUITE;
    }
    else
    {
        ret = DISPATCH_TEST_FN_NOT_FOUND;
    }

    return( ret );
}


#line 2 "suites/host_test.function"

/**
 * \brief       Verifies that string is in string parameter format i.e. "<str>"
 *              It also strips enclosing '"' from the input string.
 *
 * \param str   String parameter.
 *
 * \return      0 if success else 1
 */
int verify_string( char **str )
{
    if( ( *str )[0] != '"' ||
        ( *str )[strlen( *str ) - 1] != '"' )
    {
        mbedtls_fprintf( stderr,
            "Expected string (with \"\") for parameter and got: %s\n", *str );
        return( -1 );
    }

    ( *str )++;
    ( *str )[strlen( *str ) - 1] = '\0';

    return( 0 );
}

/**
 * \brief       Verifies that string is an integer. Also gives the converted
 *              integer value.
 *
 * \param str   Input string.
 * \param value Pointer to int for output value.
 *
 * \return      0 if success else 1
 */
int verify_int( char *str, int *value )
{
    size_t i;
    int minus = 0;
    int digits = 1;
    int hex = 0;

    for( i = 0; i < strlen( str ); i++ )
    {
        if( i == 0 && str[i] == '-' )
        {
            minus = 1;
            continue;
        }

        if( ( ( minus && i == 2 ) || ( !minus && i == 1 ) ) &&
            str[i - 1] == '0' && ( str[i] == 'x' || str[i] == 'X' ) )
        {
            hex = 1;
            continue;
        }

        if( ! ( ( str[i] >= '0' && str[i] <= '9' ) ||
                ( hex && ( ( str[i] >= 'a' && str[i] <= 'f' ) ||
                           ( str[i] >= 'A' && str[i] <= 'F' ) ) ) ) )
        {
            digits = 0;
            break;
        }
    }

    if( digits )
    {
        if( hex )
            *value = strtol( str, NULL, 16 );
        else
            *value = strtol( str, NULL, 10 );

        return( 0 );
    }

    mbedtls_fprintf( stderr,
                    "Expected integer for parameter and got: %s\n", str );
    return( KEY_VALUE_MAPPING_NOT_FOUND );
}


/**
 * \brief       Usage string.
 *
 */
#define USAGE \
    "Usage: %s [OPTIONS] files...\n\n" \
    "   Command line arguments:\n" \
    "     files...          One or more test data files. If no file is\n" \
    "                       specified the following default test case\n" \
    "                       file is used:\n" \
    "                           %s\n\n" \
    "   Options:\n" \
    "     -v | --verbose    Display full information about each test\n" \
    "     -h | --help       Display this information\n\n", \
    argv[0], \
    "TESTCASE_FILENAME"


/**
 * \brief       Read a line from the passed file pointer.
 *
 * \param f     FILE pointer
 * \param buf   Pointer to memory to hold read line.
 * \param len   Length of the buf.
 *
 * \return      0 if success else -1
 */
int get_line( FILE *f, char *buf, size_t len )
{
    char *ret;
    int i = 0, str_len = 0, has_string = 0;

    /* Read until we get a valid line */
    do
    {
        ret = fgets( buf, len, f );
        if( ret == NULL )
            return( -1 );

        str_len = strlen( buf );

        /* Skip empty line and comment */
        if ( str_len == 0 || buf[0] == '#' )
            continue;
        has_string = 0;
        for ( i = 0; i < str_len; i++ )
        {
            char c = buf[i];
            if ( c != ' ' && c != '\t' && c != '\n' &&
                 c != '\v' && c != '\f' && c != '\r' )
            {
                has_string = 1;
                break;
            }
        }
    } while( !has_string );

    /* Strip new line and carriage return */
    ret = buf + strlen( buf );
    if( ret-- > buf && *ret == '\n' )
        *ret = '\0';
    if( ret-- > buf && *ret == '\r' )
        *ret = '\0';

    return( 0 );
}

/**
 * \brief       Splits string delimited by ':'. Ignores '\:'.
 *
 * \param buf           Input string
 * \param len           Input string length
 * \param params        Out params found
 * \param params_len    Out params array len
 *
 * \return      Count of strings found.
 */
static int parse_arguments( char *buf, size_t len, char **params,
                            size_t params_len )
{
    size_t cnt = 0, i;
    char *cur = buf;
    char *p = buf, *q;

    params[cnt++] = cur;

    while( *p != '\0' && p < ( buf + len ) )
    {
        if( *p == '\\' )
        {
            p++;
            p++;
            continue;
        }
        if( *p == ':' )
        {
            if( p + 1 < buf + len )
            {
                cur = p + 1;
                TEST_HELPER_ASSERT( cnt < params_len );
                params[cnt++] = cur;
            }
            *p = '\0';
        }

        p++;
    }

    /* Replace newlines, question marks and colons in strings */
    for( i = 0; i < cnt; i++ )
    {
        p = params[i];
        q = params[i];

        while( *p != '\0' )
        {
            if( *p == '\\' && *( p + 1 ) == 'n' )
            {
                p += 2;
                *( q++ ) = '\n';
            }
            else if( *p == '\\' && *( p + 1 ) == ':' )
            {
                p += 2;
                *( q++ ) = ':';
            }
            else if( *p == '\\' && *( p + 1 ) == '?' )
            {
                p += 2;
                *( q++ ) = '?';
            }
            else
                *( q++ ) = *( p++ );
        }
        *q = '\0';
    }

    return( cnt );
}

/**
 * \brief       Converts parameters into test function consumable parameters.
 *              Example: Input:  {"int", "0", "char*", "Hello",
 *                                "hex", "abef", "exp", "1"}
 *                      Output:  {
 *                                0,                // Verified int
 *                                "Hello",          // Verified string
 *                                2, { 0xab, 0xef },// Converted len,hex pair
 *                                9600              // Evaluated expression
 *                               }
 *
 *
 * \param cnt               Parameter array count.
 * \param params            Out array of found parameters.
 * \param int_params_store  Memory for storing processed integer parameters.
 *
 * \return      0 for success else 1
 */
static int convert_params( size_t cnt , char ** params , int * int_params_store )
{
    char ** cur = params;
    char ** out = params;
    int ret = DISPATCH_TEST_SUCCESS;

    while ( cur < params + cnt )
    {
        char * type = *cur++;
        char * val = *cur++;

        if ( strcmp( type, "char*" ) == 0 )
        {
            if ( verify_string( &val ) == 0 )
            {
              *out++ = val;
            }
            else
            {
                ret = ( DISPATCH_INVALID_TEST_DATA );
                break;
            }
        }
        else if ( strcmp( type, "int" ) == 0 )
        {
            if ( verify_int( val, int_params_store ) == 0 )
            {
              *out++ = (char *) int_params_store++;
            }
            else
            {
                ret = ( DISPATCH_INVALID_TEST_DATA );
                break;
            }
        }
        else if ( strcmp( type, "hex" ) == 0 )
        {
            if ( verify_string( &val ) == 0 )
            {
                *int_params_store = mbedtls_test_unhexify(
                                        (unsigned char *) val, val );
                *out++ = val;
                *out++ = (char *)(int_params_store++);
            }
            else
            {
                ret = ( DISPATCH_INVALID_TEST_DATA );
                break;
            }
        }
        else if ( strcmp( type, "exp" ) == 0 )
        {
            int exp_id = strtol( val, NULL, 10 );
            if ( get_expression ( exp_id, int_params_store ) == 0 )
            {
              *out++ = (char *)int_params_store++;
            }
            else
            {
              ret = ( DISPATCH_INVALID_TEST_DATA );
              break;
            }
        }
        else
        {
          ret = ( DISPATCH_INVALID_TEST_DATA );
          break;
        }
    }
    return( ret );
}

/**
 * \brief       Tests snprintf implementation with test input.
 *
 * \note
 * At high optimization levels (e.g. gcc -O3), this function may be
 * inlined in run_test_snprintf. This can trigger a spurious warning about
 * potential misuse of snprintf from gcc -Wformat-truncation (observed with
 * gcc 7.2). This warning makes tests in run_test_snprintf redundant on gcc
 * only. They are still valid for other compilers. Avoid this warning by
 * forbidding inlining of this function by gcc.
 *
 * \param n         Buffer test length.
 * \param ref_buf   Expected buffer.
 * \param ref_ret   Expected snprintf return value.
 *
 * \return      0 for success else 1
 */
#if defined(__GNUC__)
__attribute__((__noinline__))
#endif
static int test_snprintf( size_t n, const char *ref_buf, int ref_ret )
{
    int ret;
    char buf[10] = "xxxxxxxxx";
    const char ref[10] = "xxxxxxxxx";

    if( n >= sizeof( buf ) )
        return( -1 );
    ret = mbedtls_snprintf( buf, n, "%s", "123" );
    if( ret < 0 || (size_t) ret >= n )
        ret = -1;

    if( strncmp( ref_buf, buf, sizeof( buf ) ) != 0 ||
        ref_ret != ret ||
        memcmp( buf + n, ref + n, sizeof( buf ) - n ) != 0 )
    {
        return( 1 );
    }

    return( 0 );
}

/**
 * \brief       Tests snprintf implementation.
 *
 * \param none
 *
 * \return      0 for success else 1
 */
static int run_test_snprintf( void )
{
    return( test_snprintf( 0, "xxxxxxxxx",  -1 ) != 0 ||
            test_snprintf( 1, "",           -1 ) != 0 ||
            test_snprintf( 2, "1",          -1 ) != 0 ||
            test_snprintf( 3, "12",         -1 ) != 0 ||
            test_snprintf( 4, "123",         3 ) != 0 ||
            test_snprintf( 5, "123",         3 ) != 0 );
}


/**
 * \brief       Desktop implementation of execute_tests().
 *              Parses command line and executes tests from
 *              supplied or default data file.
 *
 * \param argc  Command line argument count.
 * \param argv  Argument array.
 *
 * \return      Program exit status.
 */
int execute_tests( int argc , const char ** argv )
{
    /* Local Configurations and options */
    const char *default_filename = "./test_suite_pk.datax";
    const char *test_filename = NULL;
    const char **test_files = NULL;
    size_t testfile_count = 0;
    int option_verbose = 0;
    size_t function_id = 0;

    /* Other Local variables */
    int arg_index = 1;
    const char *next_arg;
    size_t testfile_index, i, cnt;
    int ret;
    unsigned total_errors = 0, total_tests = 0, total_skipped = 0;
    FILE *file;
    char buf[5000];
    char *params[50];
    /* Store for proccessed integer params. */
    int int_params[50];
    void *pointer;
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
    int stdout_fd = -1;
#endif /* __unix__ || __APPLE__ __MACH__ */

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C) && \
    !defined(TEST_SUITE_MEMORY_BUFFER_ALLOC)
    unsigned char alloc_buf[1000000];
    mbedtls_memory_buffer_alloc_init( alloc_buf, sizeof( alloc_buf ) );
#endif

    /*
     * The C standard doesn't guarantee that all-bits-0 is the representation
     * of a NULL pointer. We do however use that in our code for initializing
     * structures, which should work on every modern platform. Let's be sure.
     */
    memset( &pointer, 0, sizeof( void * ) );
    if( pointer != NULL )
    {
        mbedtls_fprintf( stderr, "all-bits-zero is not a NULL pointer\n" );
        return( 1 );
    }

    /*
     * Make sure we have a snprintf that correctly zero-terminates
     */
    if( run_test_snprintf() != 0 )
    {
        mbedtls_fprintf( stderr, "the snprintf implementation is broken\n" );
        return( 1 );
    }

    while( arg_index < argc )
    {
        next_arg = argv[arg_index];

        if( strcmp( next_arg, "--verbose" ) == 0 ||
                 strcmp( next_arg, "-v" ) == 0 )
        {
            option_verbose = 1;
        }
        else if( strcmp(next_arg, "--help" ) == 0 ||
                 strcmp(next_arg, "-h" ) == 0 )
        {
            mbedtls_fprintf( stdout, USAGE );
            mbedtls_exit( EXIT_SUCCESS );
        }
        else
        {
            /* Not an option, therefore treat all further arguments as the file
             * list.
             */
            test_files = &argv[ arg_index ];
            testfile_count = argc - arg_index;
        }

        arg_index++;
    }

    /* If no files were specified, assume a default */
    if ( test_files == NULL || testfile_count == 0 )
    {
        test_files = &default_filename;
        testfile_count = 1;
    }

    /* Initialize the struct that holds information about the last test */
    memset( &test_info, 0, sizeof( test_info ) );

    /* Now begin to execute the tests in the testfiles */
    for ( testfile_index = 0;
          testfile_index < testfile_count;
          testfile_index++ )
    {
        size_t unmet_dep_count = 0;
        int unmet_dependencies[20];
        int missing_unmet_dependencies = 0;

        test_filename = test_files[ testfile_index ];

        file = fopen( test_filename, "r" );
        if( file == NULL )
        {
            mbedtls_fprintf( stderr, "Failed to open test file: %s\n",
                             test_filename );
            return( 1 );
        }

        while( !feof( file ) )
        {
            if( unmet_dep_count > 0 )
            {
                mbedtls_fprintf( stderr,
                    "FATAL: Dep count larger than zero at start of loop\n" );
                mbedtls_exit( MBEDTLS_EXIT_FAILURE );
            }
            unmet_dep_count = 0;
            missing_unmet_dependencies = 0;

            if( ( ret = get_line( file, buf, sizeof(buf) ) ) != 0 )
                break;
            mbedtls_fprintf( stdout, "%s%.66s",
                    test_info.result == TEST_RESULT_FAILED ? "\n" : "", buf );
            mbedtls_fprintf( stdout, " " );
            for( i = strlen( buf ) + 1; i < 67; i++ )
                mbedtls_fprintf( stdout, "." );
            mbedtls_fprintf( stdout, " " );
            fflush( stdout );

            total_tests++;

            if( ( ret = get_line( file, buf, sizeof( buf ) ) ) != 0 )
                break;
            cnt = parse_arguments( buf, strlen( buf ), params,
                                   sizeof( params ) / sizeof( params[0] ) );

            if( strcmp( params[0], "depends_on" ) == 0 )
            {
                for( i = 1; i < cnt; i++ )
                {
                    int dep_id = strtol( params[i], NULL, 10 );
                    if( dep_check( dep_id ) != DEPENDENCY_SUPPORTED )
                    {
                        if( unmet_dep_count <
                            ARRAY_LENGTH( unmet_dependencies ) )
                        {
                            unmet_dependencies[unmet_dep_count] = dep_id;
                            unmet_dep_count++;
                        }
                        else
                        {
                            missing_unmet_dependencies = 1;
                        }
                    }
                }

                if( ( ret = get_line( file, buf, sizeof( buf ) ) ) != 0 )
                    break;
                cnt = parse_arguments( buf, strlen( buf ), params,
                                       sizeof( params ) / sizeof( params[0] ) );
            }

            // If there are no unmet dependencies execute the test
            if( unmet_dep_count == 0 )
            {
                test_info.result = TEST_RESULT_SUCCESS;
                test_info.paramfail_test_state = PARAMFAIL_TESTSTATE_IDLE;

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
                /* Suppress all output from the library unless we're verbose
                 * mode
                 */
                if( !option_verbose )
                {
                    stdout_fd = redirect_output( stdout, "/dev/null" );
                    if( stdout_fd == -1 )
                    {
                        /* Redirection has failed with no stdout so exit */
                        exit( 1 );
                    }
                }
#endif /* __unix__ || __APPLE__ __MACH__ */

                function_id = strtoul( params[0], NULL, 10 );
                if ( (ret = check_test( function_id )) == DISPATCH_TEST_SUCCESS )
                {
                    ret = convert_params( cnt - 1, params + 1, int_params );
                    if ( DISPATCH_TEST_SUCCESS == ret )
                    {
                        ret = dispatch_test( function_id, (void **)( params + 1 ) );
                    }
                }

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
                if( !option_verbose && restore_output( stdout, stdout_fd ) )
                {
                        /* Redirection has failed with no stdout so exit */
                        exit( 1 );
                }
#endif /* __unix__ || __APPLE__ __MACH__ */

            }

            if( unmet_dep_count > 0 || ret == DISPATCH_UNSUPPORTED_SUITE )
            {
                total_skipped++;
                mbedtls_fprintf( stdout, "----" );

                if( 1 == option_verbose && ret == DISPATCH_UNSUPPORTED_SUITE )
                {
                    mbedtls_fprintf( stdout, "\n   Test Suite not enabled" );
                }

                if( 1 == option_verbose && unmet_dep_count > 0 )
                {
                    mbedtls_fprintf( stdout, "\n   Unmet dependencies: " );
                    for( i = 0; i < unmet_dep_count; i++ )
                    {
                        mbedtls_fprintf( stdout, "%d ",
                                        unmet_dependencies[i] );
                    }
                    if( missing_unmet_dependencies )
                        mbedtls_fprintf( stdout, "..." );
                }
                mbedtls_fprintf( stdout, "\n" );
                fflush( stdout );

                unmet_dep_count = 0;
                missing_unmet_dependencies = 0;
            }
            else if( ret == DISPATCH_TEST_SUCCESS )
            {
                if( test_info.result == TEST_RESULT_SUCCESS )
                {
                    mbedtls_fprintf( stdout, "PASS\n" );
                }
                else if( test_info.result == TEST_RESULT_SKIPPED )
                {
                    mbedtls_fprintf( stdout, "----\n" );
                    total_skipped++;
                }
                else
                {
                    total_errors++;
                    mbedtls_fprintf( stdout, "FAILED\n" );
                    mbedtls_fprintf( stdout, "  %s\n  at line %d, %s\n",
                                     test_info.test, test_info.line_no,
                                     test_info.filename );
                }
                fflush( stdout );
            }
            else if( ret == DISPATCH_INVALID_TEST_DATA )
            {
                mbedtls_fprintf( stderr, "FAILED: FATAL PARSE ERROR\n" );
                fclose( file );
                mbedtls_exit( 2 );
            }
            else if( ret == DISPATCH_TEST_FN_NOT_FOUND )
            {
                mbedtls_fprintf( stderr, "FAILED: FATAL TEST FUNCTION NOT FUND\n" );
                fclose( file );
                mbedtls_exit( 2 );
            }
            else
                total_errors++;
        }
        fclose( file );
    }

    mbedtls_fprintf( stdout, "\n----------------------------------------------------------------------------\n\n");
    if( total_errors == 0 )
        mbedtls_fprintf( stdout, "PASSED" );
    else
        mbedtls_fprintf( stdout, "FAILED" );

    mbedtls_fprintf( stdout, " (%u / %u tests (%u skipped))\n",
                     total_tests - total_errors, total_tests, total_skipped );

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C) && \
    !defined(TEST_SUITE_MEMORY_BUFFER_ALLOC)
#if defined(MBEDTLS_MEMORY_DEBUG)
    mbedtls_memory_buffer_alloc_status();
#endif
    mbedtls_memory_buffer_alloc_free();
#endif

    return( total_errors != 0 );
}


#line 244 "suites/main_test.function"

/*----------------------------------------------------------------------------*/
/* Main Test code */


/**
 * \brief       Program main. Invokes platform specific execute_tests().
 *
 * \param argc      Command line arguments count.
 * \param argv      Array of command line arguments.
 *
 * \return       Exit code.
 */
int main( int argc, const char *argv[] )
{
    int ret = platform_setup();
    if( ret != 0 )
    {
        mbedtls_fprintf( stderr,
                         "FATAL: Failed to initialize platform - error %d\n",
                         ret );
        return( -1 );
    }
    ret = execute_tests( argc, argv );
    platform_teardown();
    return( ret );
}

