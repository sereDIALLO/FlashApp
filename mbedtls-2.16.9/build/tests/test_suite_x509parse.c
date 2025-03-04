#line 2 "suites/main_test.function"
/*
 * *** THIS FILE HAS BEEN MACHINE GENERATED ***
 *
 * This file has been machine generated using the script:
 * generate_test_code.py
 *
 * Test file      : ./test_suite_x509parse.c
 *
 * The following files were used to create this file.
 *
 *      Main code file      : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/main_test.function
 *      Platform code file  : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/host_test.function
 *      Helper file         : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/helpers.function
 *      Test suite file     : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function
 *      Test suite data     : /home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.data
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

#if defined(MBEDTLS_BIGNUM_C)
#line 2 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
#include "mbedtls/bignum.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/x509_crl.h"
#include "mbedtls/x509_csr.h"
#include "mbedtls/pem.h"
#include "mbedtls/oid.h"
#include "mbedtls/base64.h"
#include "string.h"

#if MBEDTLS_X509_MAX_INTERMEDIATE_CA > 19
#error "The value of MBEDTLS_X509_MAX_INTERMEDIATE_C is larger \
than the current threshold 19. To test larger values, please \
adapt the script tests/data_files/dir-max/long.sh."
#endif

/* Test-only profile allowing all digests, PK algorithms, and curves. */
const mbedtls_x509_crt_profile profile_all =
{
    0xFFFFFFFF, /* Any MD        */
    0xFFFFFFFF, /* Any PK alg    */
    0xFFFFFFFF, /* Any curve     */
    1024,
};

/* Profile for backward compatibility. Allows SHA-1, unlike the default
   profile. */
const mbedtls_x509_crt_profile compat_profile =
{
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA1 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_RIPEMD160 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA224 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA256 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA512 ),
    0xFFFFFFF, /* Any PK alg    */
    0xFFFFFFF, /* Any curve     */
    1024,
};

const mbedtls_x509_crt_profile profile_rsa3072 =
{
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA256 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA384 ) |
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA512 ),
    MBEDTLS_X509_ID_FLAG( MBEDTLS_PK_RSA ),
    0,
    3072,
};

const mbedtls_x509_crt_profile profile_sha512 =
{
    MBEDTLS_X509_ID_FLAG( MBEDTLS_MD_SHA512 ),
    0xFFFFFFF, /* Any PK alg    */
    0xFFFFFFF, /* Any curve     */
    1024,
};

int verify_none( void *data, mbedtls_x509_crt *crt, int certificate_depth, uint32_t *flags )
{
    ((void) data);
    ((void) crt);
    ((void) certificate_depth);
    *flags |= MBEDTLS_X509_BADCERT_OTHER;

    return 0;
}

int verify_all( void *data, mbedtls_x509_crt *crt, int certificate_depth, uint32_t *flags )
{
    ((void) data);
    ((void) crt);
    ((void) certificate_depth);
    *flags = 0;

    return 0;
}

int verify_fatal( void *data, mbedtls_x509_crt *crt, int certificate_depth, uint32_t *flags )
{
    int *levels = (int *) data;

    ((void) crt);
    ((void) certificate_depth);

    /* Simulate a fatal error in the callback */
    if( *levels & ( 1 << certificate_depth ) )
    {
        *flags |= ( 1 << certificate_depth );
        return( -1 - certificate_depth );
    }

    return( 0 );
}

/* strsep() not available on Windows */
char *mystrsep(char **stringp, const char *delim)
{
    const char *p;
    char *ret = *stringp;

    if( *stringp == NULL )
        return( NULL );

    for( ; ; (*stringp)++ )
    {
        if( **stringp == '\0' )
        {
            *stringp = NULL;
            goto done;
        }

        for( p = delim; *p != '\0'; p++ )
            if( **stringp == *p )
            {
                **stringp = '\0';
                (*stringp)++;
                goto done;
            }
    }

done:
    return( ret );
}

#if defined(MBEDTLS_X509_CRT_PARSE_C)
typedef struct {
    char buf[512];
    char *p;
} verify_print_context;

void verify_print_init( verify_print_context *ctx )
{
    memset( ctx, 0, sizeof( verify_print_context ) );
    ctx->p = ctx->buf;
}

int verify_print( void *data, mbedtls_x509_crt *crt, int certificate_depth, uint32_t *flags )
{
    int ret;
    verify_print_context *ctx = (verify_print_context *) data;
    char *p = ctx->p;
    size_t n = ctx->buf + sizeof( ctx->buf ) - ctx->p;
    ((void) flags);

    ret = mbedtls_snprintf( p, n, "depth %d - serial ", certificate_depth );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ret = mbedtls_x509_serial_gets( p, n, &crt->serial );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ret = mbedtls_snprintf( p, n, " - subject " );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ret = mbedtls_x509_dn_gets( p, n, &crt->subject );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ret = mbedtls_snprintf( p, n, " - flags 0x%08x\n", *flags );
    MBEDTLS_X509_SAFE_SNPRINTF;

    ctx->p = p;

    return( 0 );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 175 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_cert_info( char * crt_file, char * result_str )
{
    mbedtls_x509_crt   crt;
    char buf[2000];
    int res;

    mbedtls_x509_crt_init( &crt );
    memset( buf, 0, 2000 );

    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == 0 );
    res = mbedtls_x509_crt_info( buf, 2000, "", &crt );

    TEST_ASSERT( res != -1 );
    TEST_ASSERT( res != -2 );

    TEST_ASSERT( strcmp( buf, result_str ) == 0 );

exit:
    mbedtls_x509_crt_free( &crt );
}

void test_x509_cert_info_wrapper( void ** params )
{

    test_x509_cert_info( (char *) params[0], (char *) params[1] );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRL_PARSE_C)
#line 198 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_crl_info( char * crl_file, char * result_str )
{
    mbedtls_x509_crl   crl;
    char buf[2000];
    int res;

    mbedtls_x509_crl_init( &crl );
    memset( buf, 0, 2000 );

    TEST_ASSERT( mbedtls_x509_crl_parse_file( &crl, crl_file ) == 0 );
    res = mbedtls_x509_crl_info( buf, 2000, "", &crl );

    TEST_ASSERT( res != -1 );
    TEST_ASSERT( res != -2 );

    TEST_ASSERT( strcmp( buf, result_str ) == 0 );

exit:
    mbedtls_x509_crl_free( &crl );
}

void test_mbedtls_x509_crl_info_wrapper( void ** params )
{

    test_mbedtls_x509_crl_info( (char *) params[0], (char *) params[1] );
}
#endif /* MBEDTLS_X509_CRL_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRL_PARSE_C)
#line 221 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_crl_parse( char * crl_file, int result )
{
    mbedtls_x509_crl   crl;
    char buf[2000];

    mbedtls_x509_crl_init( &crl );
    memset( buf, 0, 2000 );

    TEST_ASSERT( mbedtls_x509_crl_parse_file( &crl, crl_file ) == result );

exit:
    mbedtls_x509_crl_free( &crl );
}

void test_mbedtls_x509_crl_parse_wrapper( void ** params )
{

    test_mbedtls_x509_crl_parse( (char *) params[0], *( (int *) params[1] ) );
}
#endif /* MBEDTLS_X509_CRL_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CSR_PARSE_C)
#line 237 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_csr_info( char * csr_file, char * result_str )
{
    mbedtls_x509_csr   csr;
    char buf[2000];
    int res;

    mbedtls_x509_csr_init( &csr );
    memset( buf, 0, 2000 );

    TEST_ASSERT( mbedtls_x509_csr_parse_file( &csr, csr_file ) == 0 );
    res = mbedtls_x509_csr_info( buf, 2000, "", &csr );

    TEST_ASSERT( res != -1 );
    TEST_ASSERT( res != -2 );

    TEST_ASSERT( strcmp( buf, result_str ) == 0 );

exit:
    mbedtls_x509_csr_free( &csr );
}

void test_mbedtls_x509_csr_info_wrapper( void ** params )
{

    test_mbedtls_x509_csr_info( (char *) params[0], (char *) params[1] );
}
#endif /* MBEDTLS_X509_CSR_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 260 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_verify_info( int flags, char * prefix, char * result_str )
{
    char buf[2000];
    int res;

    memset( buf, 0, sizeof( buf ) );

    res = mbedtls_x509_crt_verify_info( buf, sizeof( buf ), prefix, flags );

    TEST_ASSERT( res >= 0 );

    TEST_ASSERT( strcmp( buf, result_str ) == 0 );
exit:
    ;
}

void test_x509_verify_info_wrapper( void ** params )
{

    test_x509_verify_info( *( (int *) params[0] ), (char *) params[1], (char *) params[2] );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#if defined(MBEDTLS_X509_CRL_PARSE_C)
#if defined(MBEDTLS_ECP_RESTARTABLE)
#if defined(MBEDTLS_ECDSA_C)
#line 276 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_verify_restart( char *crt_file, char *ca_file,
                          int result, int flags_result,
                          int max_ops, int min_restart, int max_restart )
{
    int ret, cnt_restart;
    mbedtls_x509_crt_restart_ctx rs_ctx;
    mbedtls_x509_crt crt;
    mbedtls_x509_crt ca;
    uint32_t flags = 0;

    /*
     * See comments on ecp_test_vect_restart() for op count precision.
     *
     * For reference, with mbed TLS 2.6 and default settings:
     * - ecdsa_verify() for P-256:  ~  6700
     * - ecdsa_verify() for P-384:  ~ 18800
     * - x509_verify() for server5 -> test-ca2:             ~ 18800
     * - x509_verify() for server10 -> int-ca3 -> int-ca2:  ~ 25500
     */

    mbedtls_x509_crt_restart_init( &rs_ctx );
    mbedtls_x509_crt_init( &crt );
    mbedtls_x509_crt_init( &ca );

    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == 0 );
    TEST_ASSERT( mbedtls_x509_crt_parse_file( &ca, ca_file ) == 0 );

    mbedtls_ecp_set_max_ops( max_ops );

    cnt_restart = 0;
    do {
        ret = mbedtls_x509_crt_verify_restartable( &crt, &ca, NULL,
                &mbedtls_x509_crt_profile_default, NULL, &flags,
                NULL, NULL, &rs_ctx );
    } while( ret == MBEDTLS_ERR_ECP_IN_PROGRESS && ++cnt_restart );

    TEST_ASSERT( ret == result );
    TEST_ASSERT( flags == (uint32_t) flags_result );

    TEST_ASSERT( cnt_restart >= min_restart );
    TEST_ASSERT( cnt_restart <= max_restart );

    /* Do we leak memory when aborting? */
    ret = mbedtls_x509_crt_verify_restartable( &crt, &ca, NULL,
            &mbedtls_x509_crt_profile_default, NULL, &flags,
            NULL, NULL, &rs_ctx );
    TEST_ASSERT( ret == result || ret == MBEDTLS_ERR_ECP_IN_PROGRESS );

exit:
    mbedtls_x509_crt_restart_free( &rs_ctx );
    mbedtls_x509_crt_free( &crt );
    mbedtls_x509_crt_free( &ca );
}

void test_x509_verify_restart_wrapper( void ** params )
{

    test_x509_verify_restart( (char *) params[0], (char *) params[1], *( (int *) params[2] ), *( (int *) params[3] ), *( (int *) params[4] ), *( (int *) params[5] ), *( (int *) params[6] ) );
}
#endif /* MBEDTLS_ECDSA_C */
#endif /* MBEDTLS_ECP_RESTARTABLE */
#endif /* MBEDTLS_X509_CRL_PARSE_C */
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#if defined(MBEDTLS_X509_CRL_PARSE_C)
#line 332 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_verify( char *crt_file, char *ca_file, char *crl_file,
                  char *cn_name_str, int result, int flags_result,
                  char *profile_str,
                  char *verify_callback )
{
    mbedtls_x509_crt   crt;
    mbedtls_x509_crt   ca;
    mbedtls_x509_crl    crl;
    uint32_t         flags = 0;
    int         res;
    int (*f_vrfy)(void *, mbedtls_x509_crt *, int, uint32_t *) = NULL;
    char *      cn_name = NULL;
    const mbedtls_x509_crt_profile *profile;

    mbedtls_x509_crt_init( &crt );
    mbedtls_x509_crt_init( &ca );
    mbedtls_x509_crl_init( &crl );

    if( strcmp( cn_name_str, "NULL" ) != 0 )
        cn_name = cn_name_str;

    if( strcmp( profile_str, "" ) == 0 )
        profile = &mbedtls_x509_crt_profile_default;
    else if( strcmp( profile_str, "next" ) == 0 )
        profile = &mbedtls_x509_crt_profile_next;
    else if( strcmp( profile_str, "suite_b" ) == 0 )
        profile = &mbedtls_x509_crt_profile_suiteb;
    else if( strcmp( profile_str, "compat" ) == 0 )
        profile = &compat_profile;
    else if( strcmp( profile_str, "all" ) == 0 )
        profile = &profile_all;
    else
        TEST_ASSERT( "Unknown algorithm profile" == 0 );

    if( strcmp( verify_callback, "NULL" ) == 0 )
        f_vrfy = NULL;
    else if( strcmp( verify_callback, "verify_none" ) == 0 )
        f_vrfy = verify_none;
    else if( strcmp( verify_callback, "verify_all" ) == 0 )
        f_vrfy = verify_all;
    else
        TEST_ASSERT( "No known verify callback selected" == 0 );

    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == 0 );
    TEST_ASSERT( mbedtls_x509_crt_parse_file( &ca, ca_file ) == 0 );
    TEST_ASSERT( mbedtls_x509_crl_parse_file( &crl, crl_file ) == 0 );

    res = mbedtls_x509_crt_verify_with_profile( &crt, &ca, &crl, profile, cn_name, &flags, f_vrfy, NULL );

    TEST_ASSERT( res == ( result ) );
    TEST_ASSERT( flags == (uint32_t)( flags_result ) );

exit:
    mbedtls_x509_crt_free( &crt );
    mbedtls_x509_crt_free( &ca );
    mbedtls_x509_crl_free( &crl );
}

void test_x509_verify_wrapper( void ** params )
{

    test_x509_verify( (char *) params[0], (char *) params[1], (char *) params[2], (char *) params[3], *( (int *) params[4] ), *( (int *) params[5] ), (char *) params[6], (char *) params[7] );
}
#endif /* MBEDTLS_X509_CRL_PARSE_C */
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 392 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_verify_callback( char *crt_file, char *ca_file, char *name,
                           int exp_ret, char *exp_vrfy_out )
{
    int ret;
    mbedtls_x509_crt crt;
    mbedtls_x509_crt ca;
    uint32_t flags = 0;
    verify_print_context vrfy_ctx;

    mbedtls_x509_crt_init( &crt );
    mbedtls_x509_crt_init( &ca );
    verify_print_init( &vrfy_ctx );

    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == 0 );
    TEST_ASSERT( mbedtls_x509_crt_parse_file( &ca, ca_file ) == 0 );

    if( strcmp( name, "NULL" ) == 0 )
        name = NULL;

    ret = mbedtls_x509_crt_verify_with_profile( &crt, &ca, NULL,
                                                &compat_profile,
                                                name, &flags,
                                                verify_print, &vrfy_ctx );

    TEST_ASSERT( ret == exp_ret );
    TEST_ASSERT( strcmp( vrfy_ctx.buf, exp_vrfy_out ) == 0 );

exit:
    mbedtls_x509_crt_free( &crt );
    mbedtls_x509_crt_free( &ca );
}

void test_x509_verify_callback_wrapper( void ** params )
{

    test_x509_verify_callback( (char *) params[0], (char *) params[1], (char *) params[2], *( (int *) params[3] ), (char *) params[4] );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 426 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_dn_gets( char * crt_file, char * entity, char * result_str )
{
    mbedtls_x509_crt   crt;
    char buf[2000];
    int res = 0;

    mbedtls_x509_crt_init( &crt );
    memset( buf, 0, 2000 );

    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == 0 );
    if( strcmp( entity, "subject" ) == 0 )
        res =  mbedtls_x509_dn_gets( buf, 2000, &crt.subject );
    else if( strcmp( entity, "issuer" ) == 0 )
        res =  mbedtls_x509_dn_gets( buf, 2000, &crt.issuer );
    else
        TEST_ASSERT( "Unknown entity" == 0 );

    TEST_ASSERT( res != -1 );
    TEST_ASSERT( res != -2 );

    TEST_ASSERT( strcmp( buf, result_str ) == 0 );

exit:
    mbedtls_x509_crt_free( &crt );
}

void test_mbedtls_x509_dn_gets_wrapper( void ** params )
{

    test_mbedtls_x509_dn_gets( (char *) params[0], (char *) params[1], (char *) params[2] );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 454 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_time_is_past( char * crt_file, char * entity, int result )
{
    mbedtls_x509_crt   crt;

    mbedtls_x509_crt_init( &crt );

    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == 0 );

    if( strcmp( entity, "valid_from" ) == 0 )
        TEST_ASSERT( mbedtls_x509_time_is_past( &crt.valid_from ) == result );
    else if( strcmp( entity, "valid_to" ) == 0 )
        TEST_ASSERT( mbedtls_x509_time_is_past( &crt.valid_to ) == result );
    else
        TEST_ASSERT( "Unknown entity" == 0 );

exit:
    mbedtls_x509_crt_free( &crt );
}

void test_mbedtls_x509_time_is_past_wrapper( void ** params )
{

    test_mbedtls_x509_time_is_past( (char *) params[0], (char *) params[1], *( (int *) params[2] ) );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 475 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_time_is_future( char * crt_file, char * entity, int result )
{
    mbedtls_x509_crt   crt;

    mbedtls_x509_crt_init( &crt );

    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == 0 );

    if( strcmp( entity, "valid_from" ) == 0 )
        TEST_ASSERT( mbedtls_x509_time_is_future( &crt.valid_from ) == result );
    else if( strcmp( entity, "valid_to" ) == 0 )
        TEST_ASSERT( mbedtls_x509_time_is_future( &crt.valid_to ) == result );
    else
        TEST_ASSERT( "Unknown entity" == 0 );

exit:
    mbedtls_x509_crt_free( &crt );
}

void test_mbedtls_x509_time_is_future_wrapper( void ** params )
{

    test_mbedtls_x509_time_is_future( (char *) params[0], (char *) params[1], *( (int *) params[2] ) );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#if defined(MBEDTLS_FS_IO)
#line 496 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509parse_crt_file( char * crt_file, int result )
{
    mbedtls_x509_crt crt;

    mbedtls_x509_crt_init( &crt );

    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == result );

exit:
    mbedtls_x509_crt_free( &crt );
}

void test_x509parse_crt_file_wrapper( void ** params )
{

    test_x509parse_crt_file( (char *) params[0], *( (int *) params[1] ) );
}
#endif /* MBEDTLS_FS_IO */
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 510 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509parse_crt( data_t * buf, char * result_str, int result )
{
    mbedtls_x509_crt   crt;
    unsigned char output[2000];
    int res;

    mbedtls_x509_crt_init( &crt );
    memset( output, 0, 2000 );


    TEST_ASSERT( mbedtls_x509_crt_parse( &crt, buf->x, buf->len ) == ( result ) );
    if( ( result ) == 0 )
    {
        res = mbedtls_x509_crt_info( (char *) output, 2000, "", &crt );

        TEST_ASSERT( res != -1 );
        TEST_ASSERT( res != -2 );

        TEST_ASSERT( strcmp( (char *) output, result_str ) == 0 );
    }

exit:
    mbedtls_x509_crt_free( &crt );
}

void test_x509parse_crt_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};

    test_x509parse_crt( &data0, (char *) params[2], *( (int *) params[3] ) );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#if defined(MBEDTLS_X509_CRL_PARSE_C)
#line 537 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509parse_crl( data_t * buf, char * result_str, int result )
{
    mbedtls_x509_crl   crl;
    unsigned char output[2000];
    int res;

    mbedtls_x509_crl_init( &crl );
    memset( output, 0, 2000 );


    TEST_ASSERT( mbedtls_x509_crl_parse( &crl, buf->x, buf->len ) == ( result ) );
    if( ( result ) == 0 )
    {
        res = mbedtls_x509_crl_info( (char *) output, 2000, "", &crl );

        TEST_ASSERT( res != -1 );
        TEST_ASSERT( res != -2 );

        TEST_ASSERT( strcmp( (char *) output, result_str ) == 0 );
    }

exit:
    mbedtls_x509_crl_free( &crl );
}

void test_x509parse_crl_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};

    test_x509parse_crl( &data0, (char *) params[2], *( (int *) params[3] ) );
}
#endif /* MBEDTLS_X509_CRL_PARSE_C */
#if defined(MBEDTLS_X509_CSR_PARSE_C)
#line 564 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_csr_parse( data_t * csr_der, char * ref_out, int ref_ret )
{
    mbedtls_x509_csr csr;
    char my_out[1000];
    int my_ret;

    mbedtls_x509_csr_init( &csr );
    memset( my_out, 0, sizeof( my_out ) );

    my_ret = mbedtls_x509_csr_parse_der( &csr, csr_der->x, csr_der->len );
    TEST_ASSERT( my_ret == ref_ret );

    if( ref_ret == 0 )
    {
        size_t my_out_len = mbedtls_x509_csr_info( my_out, sizeof( my_out ), "", &csr );
        TEST_ASSERT( my_out_len == strlen( ref_out ) );
        TEST_ASSERT( strcmp( my_out, ref_out ) == 0 );
    }

exit:
    mbedtls_x509_csr_free( &csr );
}

void test_mbedtls_x509_csr_parse_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};

    test_mbedtls_x509_csr_parse( &data0, (char *) params[2], *( (int *) params[3] ) );
}
#endif /* MBEDTLS_X509_CSR_PARSE_C */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 589 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_crt_parse_path( char * crt_path, int ret, int nb_crt )
{
    mbedtls_x509_crt chain, *cur;
    int i;

    mbedtls_x509_crt_init( &chain );

    TEST_ASSERT( mbedtls_x509_crt_parse_path( &chain, crt_path ) == ret );

    /* Check how many certs we got */
    for( i = 0, cur = &chain; cur != NULL; cur = cur->next )
        if( cur->raw.p != NULL )
            i++;

    TEST_ASSERT( i == nb_crt );

exit:
    mbedtls_x509_crt_free( &chain );
}

void test_mbedtls_x509_crt_parse_path_wrapper( void ** params )
{

    test_mbedtls_x509_crt_parse_path( (char *) params[0], *( (int *) params[1] ), *( (int *) params[2] ) );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 611 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_crt_verify_max( char *ca_file, char *chain_dir, int nb_int,
                                  int ret_chk, int flags_chk )
{
    char file_buf[128];
    int ret;
    uint32_t flags;
    mbedtls_x509_crt trusted, chain;

    /*
     * We expect chain_dir to contain certificates 00.crt, 01.crt, etc.
     * with NN.crt signed by NN-1.crt
     */

    mbedtls_x509_crt_init( &trusted );
    mbedtls_x509_crt_init( &chain );

    /* Load trusted root */
    TEST_ASSERT( mbedtls_x509_crt_parse_file( &trusted, ca_file ) == 0 );

    /* Load a chain with nb_int intermediates (from 01 to nb_int),
     * plus one "end-entity" cert (nb_int + 1) */
    ret = mbedtls_snprintf( file_buf, sizeof file_buf, "%s/c%02d.pem", chain_dir,
                                                            nb_int + 1 );
    TEST_ASSERT( ret > 0 && (size_t) ret < sizeof file_buf );
    TEST_ASSERT( mbedtls_x509_crt_parse_file( &chain, file_buf ) == 0 );

    /* Try to verify that chain */
    ret = mbedtls_x509_crt_verify( &chain, &trusted, NULL, NULL, &flags,
                                   NULL, NULL );
    TEST_ASSERT( ret == ret_chk );
    TEST_ASSERT( flags == (uint32_t) flags_chk );

exit:
    mbedtls_x509_crt_free( &chain );
    mbedtls_x509_crt_free( &trusted );
}

void test_mbedtls_x509_crt_verify_max_wrapper( void ** params )
{

    test_mbedtls_x509_crt_verify_max( (char *) params[0], (char *) params[1], *( (int *) params[2] ), *( (int *) params[3] ), *( (int *) params[4] ) );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#line 650 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_mbedtls_x509_crt_verify_chain(  char *chain_paths, char *trusted_ca,
                                     int flags_result, int result,
                                     char *profile_name, int vrfy_fatal_lvls )
{
    char* act;
    uint32_t flags;
    int res;
    mbedtls_x509_crt trusted, chain;
    const mbedtls_x509_crt_profile *profile = NULL;

    mbedtls_x509_crt_init( &chain );
    mbedtls_x509_crt_init( &trusted );

    while( ( act = mystrsep( &chain_paths, " " ) ) != NULL )
        TEST_ASSERT( mbedtls_x509_crt_parse_file( &chain, act ) == 0 );
    TEST_ASSERT( mbedtls_x509_crt_parse_file( &trusted, trusted_ca ) == 0 );

    if( strcmp( profile_name, "" ) == 0 )
        profile = &mbedtls_x509_crt_profile_default;
    else if( strcmp( profile_name, "next" ) == 0 )
        profile = &mbedtls_x509_crt_profile_next;
    else if( strcmp( profile_name, "suiteb" ) == 0 )
        profile = &mbedtls_x509_crt_profile_suiteb;
    else if( strcmp( profile_name, "rsa3072" ) == 0 )
        profile = &profile_rsa3072;
    else if( strcmp( profile_name, "sha512" ) == 0 )
        profile = &profile_sha512;

    res = mbedtls_x509_crt_verify_with_profile( &chain, &trusted, NULL, profile,
            NULL, &flags, verify_fatal, &vrfy_fatal_lvls );

    TEST_ASSERT( res == ( result ) );
    TEST_ASSERT( flags == (uint32_t)( flags_result ) );

exit:
    mbedtls_x509_crt_free( &trusted );
    mbedtls_x509_crt_free( &chain );
}

void test_mbedtls_x509_crt_verify_chain_wrapper( void ** params )
{

    test_mbedtls_x509_crt_verify_chain( (char *) params[0], (char *) params[1], *( (int *) params[2] ), *( (int *) params[3] ), (char *) params[4], *( (int *) params[5] ) );
}
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_X509_USE_C)
#line 691 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_oid_desc( data_t * buf, char * ref_desc )
{
    mbedtls_x509_buf oid;
    const char *desc = NULL;
    int ret;


    oid.tag = MBEDTLS_ASN1_OID;
    oid.p   = buf->x;
    oid.len   = buf->len;

    ret = mbedtls_oid_get_extended_key_usage( &oid, &desc );

    if( strcmp( ref_desc, "notfound" ) == 0 )
    {
        TEST_ASSERT( ret != 0 );
        TEST_ASSERT( desc == NULL );
    }
    else
    {
        TEST_ASSERT( ret == 0 );
        TEST_ASSERT( desc != NULL );
        TEST_ASSERT( strcmp( desc, ref_desc ) == 0 );
    }
exit:
    ;
}

void test_x509_oid_desc_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};

    test_x509_oid_desc( &data0, (char *) params[2] );
}
#endif /* MBEDTLS_X509_USE_C */
#if defined(MBEDTLS_X509_USE_C)
#line 719 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_oid_numstr( data_t * oid_buf, char * numstr, int blen, int ret )
{
    mbedtls_x509_buf oid;
    char num_buf[100];

    memset( num_buf, 0x2a, sizeof num_buf );

    oid.tag = MBEDTLS_ASN1_OID;
    oid.p   = oid_buf->x;
    oid.len   = oid_buf->len;

    TEST_ASSERT( (size_t) blen <= sizeof num_buf );

    TEST_ASSERT( mbedtls_oid_get_numeric_string( num_buf, blen, &oid ) == ret );

    if( ret >= 0 )
    {
        TEST_ASSERT( num_buf[ret] == 0 );
        TEST_ASSERT( strcmp( num_buf, numstr ) == 0 );
    }
exit:
    ;
}

void test_x509_oid_numstr_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};

    test_x509_oid_numstr( &data0, (char *) params[2], *( (int *) params[3] ), *( (int *) params[4] ) );
}
#endif /* MBEDTLS_X509_USE_C */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#if defined(MBEDTLS_X509_CHECK_KEY_USAGE)
#line 743 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_check_key_usage( char * crt_file, int usage, int ret )
{
    mbedtls_x509_crt crt;

    mbedtls_x509_crt_init( &crt );

    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == 0 );

    TEST_ASSERT( mbedtls_x509_crt_check_key_usage( &crt, usage ) == ret );

exit:
    mbedtls_x509_crt_free( &crt );
}

void test_x509_check_key_usage_wrapper( void ** params )
{

    test_x509_check_key_usage( (char *) params[0], *( (int *) params[1] ), *( (int *) params[2] ) );
}
#endif /* MBEDTLS_X509_CHECK_KEY_USAGE */
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_FS_IO)
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#if defined(MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE)
#line 759 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_check_extended_key_usage( char * crt_file, data_t * oid, int ret
                                    )
{
    mbedtls_x509_crt crt;

    mbedtls_x509_crt_init( &crt );


    TEST_ASSERT( mbedtls_x509_crt_parse_file( &crt, crt_file ) == 0 );

    TEST_ASSERT( mbedtls_x509_crt_check_extended_key_usage( &crt, (const char *)oid->x, oid->len ) == ret );

exit:
    mbedtls_x509_crt_free( &crt );
}

void test_x509_check_extended_key_usage_wrapper( void ** params )
{
    data_t data1 = {(uint8_t *) params[1], *( (uint32_t *) params[2] )};

    test_x509_check_extended_key_usage( (char *) params[0], &data1, *( (int *) params[3] ) );
}
#endif /* MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE */
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_FS_IO */
#if defined(MBEDTLS_X509_USE_C)
#line 777 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_get_time( int tag, char * time_str, int ret, int year, int mon,
                    int day, int hour, int min, int sec )
{
    mbedtls_x509_time time;
    unsigned char buf[21];
    unsigned char* start = buf;
    unsigned char* end = buf;

    memset( &time, 0x00, sizeof( time ) );
    *end = (unsigned char)tag; end++;
    *end = strlen( time_str );
    TEST_ASSERT( *end < 20 );
    end++;
    memcpy( end, time_str, (size_t)*(end - 1) );
    end += *(end - 1);

    TEST_ASSERT( mbedtls_x509_get_time( &start, end, &time ) == ret );
    if( ret == 0 )
    {
        TEST_ASSERT( year == time.year );
        TEST_ASSERT( mon  == time.mon  );
        TEST_ASSERT( day  == time.day  );
        TEST_ASSERT( hour == time.hour );
        TEST_ASSERT( min  == time.min  );
        TEST_ASSERT( sec  == time.sec  );
    }
exit:
    ;
}

void test_x509_get_time_wrapper( void ** params )
{

    test_x509_get_time( *( (int *) params[0] ), (char *) params[1], *( (int *) params[2] ), *( (int *) params[3] ), *( (int *) params[4] ), *( (int *) params[5] ), *( (int *) params[6] ), *( (int *) params[7] ), *( (int *) params[8] ) );
}
#endif /* MBEDTLS_X509_USE_C */
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#if defined(MBEDTLS_X509_RSASSA_PSS_SUPPORT)
#line 807 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_parse_rsassa_pss_params( data_t * params, int params_tag,
                                   int ref_msg_md, int ref_mgf_md,
                                   int ref_salt_len, int ref_ret )
{
    int my_ret;
    mbedtls_x509_buf buf;
    mbedtls_md_type_t my_msg_md, my_mgf_md;
    int my_salt_len;

    buf.p = params->x;
    buf.len = params->len;
    buf.tag = params_tag;

    my_ret = mbedtls_x509_get_rsassa_pss_params( &buf, &my_msg_md, &my_mgf_md,
                                                 &my_salt_len );

    TEST_ASSERT( my_ret == ref_ret );

    if( ref_ret == 0 )
    {
        TEST_ASSERT( my_msg_md == (mbedtls_md_type_t) ref_msg_md );
        TEST_ASSERT( my_mgf_md == (mbedtls_md_type_t) ref_mgf_md );
        TEST_ASSERT( my_salt_len == ref_salt_len );
    }

exit:
    ;;
}

void test_x509_parse_rsassa_pss_params_wrapper( void ** params )
{
    data_t data0 = {(uint8_t *) params[0], *( (uint32_t *) params[1] )};

    test_x509_parse_rsassa_pss_params( &data0, *( (int *) params[2] ), *( (int *) params[3] ), *( (int *) params[4] ), *( (int *) params[5] ), *( (int *) params[6] ) );
}
#endif /* MBEDTLS_X509_RSASSA_PSS_SUPPORT */
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#if defined(MBEDTLS_SELF_TEST)
#line 838 "/home/tdiallo/Bureau/mbedtls-2.16.9/tests/suites/test_suite_x509parse.function"
void test_x509_selftest(  )
{
    TEST_ASSERT( mbedtls_x509_self_test( 1 ) == 0 );
exit:
    ;
}

void test_x509_selftest_wrapper( void ** params )
{
    (void)params;

    test_x509_selftest(  );
}
#endif /* MBEDTLS_SELF_TEST */
#endif /* MBEDTLS_X509_CRT_PARSE_C */
#endif /* MBEDTLS_BIGNUM_C */


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

#if defined(MBEDTLS_BIGNUM_C)

        case 0:
            {
                *out_value = MBEDTLS_ERR_PEM_NO_HEADER_FOOTER_PRESENT;
            }
            break;
        case 1:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_EXTENSIONS + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 2:
            {
                *out_value = MBEDTLS_X509_BADCERT_MISSING;
            }
            break;
        case 3:
            {
                *out_value = MBEDTLS_X509_BADCERT_EXPIRED | MBEDTLS_X509_BADCRL_EXPIRED;
            }
            break;
        case 4:
            {
                *out_value = MBEDTLS_X509_BADCERT_OTHER | 0x80000000;
            }
            break;
        case 5:
            {
                *out_value = MBEDTLS_ERR_X509_CERT_VERIFY_FAILED;
            }
            break;
        case 6:
            {
                *out_value = MBEDTLS_X509_BADCERT_REVOKED | MBEDTLS_X509_BADCRL_EXPIRED;
            }
            break;
        case 7:
            {
                *out_value = MBEDTLS_X509_BADCERT_REVOKED | MBEDTLS_X509_BADCRL_FUTURE;
            }
            break;
        case 8:
            {
                *out_value = MBEDTLS_X509_BADCERT_REVOKED | MBEDTLS_X509_BADCRL_EXPIRED | MBEDTLS_X509_BADCERT_CN_MISMATCH;
            }
            break;
        case 9:
            {
                *out_value = MBEDTLS_X509_BADCERT_REVOKED | MBEDTLS_X509_BADCRL_FUTURE | MBEDTLS_X509_BADCERT_CN_MISMATCH;
            }
            break;
        case 10:
            {
                *out_value = MBEDTLS_X509_BADCRL_EXPIRED;
            }
            break;
        case 11:
            {
                *out_value = MBEDTLS_X509_BADCRL_FUTURE;
            }
            break;
        case 12:
            {
                *out_value = MBEDTLS_X509_BADCERT_REVOKED;
            }
            break;
        case 13:
            {
                *out_value = MBEDTLS_X509_BADCERT_REVOKED | MBEDTLS_X509_BADCERT_CN_MISMATCH;
            }
            break;
        case 14:
            {
                *out_value = MBEDTLS_X509_BADCERT_EXPIRED;
            }
            break;
        case 15:
            {
                *out_value = MBEDTLS_X509_BADCERT_FUTURE;
            }
            break;
        case 16:
            {
                *out_value = MBEDTLS_X509_BADCERT_NOT_TRUSTED;
            }
            break;
        case 17:
            {
                *out_value = MBEDTLS_X509_BADCERT_BAD_MD;
            }
            break;
        case 18:
            {
                *out_value = MBEDTLS_X509_BADCRL_BAD_MD | MBEDTLS_X509_BADCERT_BAD_MD;
            }
            break;
        case 19:
            {
                *out_value = MBEDTLS_X509_BADCERT_OTHER;
            }
            break;
        case 20:
            {
                *out_value = MBEDTLS_X509_BADCERT_CN_MISMATCH;
            }
            break;
        case 21:
            {
                *out_value = MBEDTLS_X509_BADCERT_CN_MISMATCH + MBEDTLS_X509_BADCERT_NOT_TRUSTED;
            }
            break;
        case 22:
            {
                *out_value = MBEDTLS_X509_BADCRL_NOT_TRUSTED;
            }
            break;
        case 23:
            {
                *out_value = MBEDTLS_X509_BADCERT_REVOKED|MBEDTLS_X509_BADCRL_FUTURE;
            }
            break;
        case 24:
            {
                *out_value = MBEDTLS_X509_BADCERT_BAD_MD|MBEDTLS_X509_BADCERT_BAD_PK|MBEDTLS_X509_BADCERT_BAD_KEY|MBEDTLS_X509_BADCRL_BAD_MD|MBEDTLS_X509_BADCRL_BAD_PK;
            }
            break;
        case 25:
            {
                *out_value = MBEDTLS_X509_BADCERT_BAD_PK;
            }
            break;
        case 26:
            {
                *out_value = MBEDTLS_X509_BADCERT_BAD_MD|MBEDTLS_X509_BADCRL_BAD_MD;
            }
            break;
        case 27:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_FORMAT;
            }
            break;
        case 28:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_FORMAT + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 29:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_FORMAT + MBEDTLS_ERR_ASN1_INVALID_LENGTH;
            }
            break;
        case 30:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_FORMAT + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 31:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_SERIAL + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 32:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_VERSION + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 33:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_VERSION + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 34:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_VERSION + MBEDTLS_ERR_ASN1_INVALID_LENGTH;
            }
            break;
        case 35:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_SERIAL + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 36:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_VERSION + MBEDTLS_ERR_ASN1_LENGTH_MISMATCH;
            }
            break;
        case 37:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_ALG + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 38:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_ALG + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 39:
            {
                *out_value = MBEDTLS_ERR_X509_UNKNOWN_VERSION;
            }
            break;
        case 40:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_ALG + MBEDTLS_ERR_ASN1_LENGTH_MISMATCH;
            }
            break;
        case 41:
            {
                *out_value = MBEDTLS_ERR_X509_UNKNOWN_SIG_ALG + MBEDTLS_ERR_OID_NOT_FOUND;
            }
            break;
        case 42:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_NAME + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 43:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_NAME + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 44:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_NAME+MBEDTLS_ERR_ASN1_LENGTH_MISMATCH;
            }
            break;
        case 45:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_DATE + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 46:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_DATE + MBEDTLS_ERR_ASN1_LENGTH_MISMATCH;
            }
            break;
        case 47:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_DATE;
            }
            break;
        case 48:
            {
                *out_value = MBEDTLS_ERR_PK_KEY_INVALID_FORMAT + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 49:
            {
                *out_value = MBEDTLS_ERR_PK_INVALID_ALG + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 50:
            {
                *out_value = MBEDTLS_ERR_PK_UNKNOWN_PK_ALG;
            }
            break;
        case 51:
            {
                *out_value = MBEDTLS_ERR_PK_INVALID_PUBKEY + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 52:
            {
                *out_value = MBEDTLS_ERR_PK_INVALID_PUBKEY + MBEDTLS_ERR_ASN1_INVALID_DATA;
            }
            break;
        case 53:
            {
                *out_value = MBEDTLS_ERR_PK_INVALID_PUBKEY + MBEDTLS_ERR_ASN1_LENGTH_MISMATCH;
            }
            break;
        case 54:
            {
                *out_value = MBEDTLS_ERR_PK_INVALID_PUBKEY + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 55:
            {
                *out_value = MBEDTLS_ERR_PK_INVALID_PUBKEY;
            }
            break;
        case 56:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_EXTENSIONS + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 57:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_EXTENSIONS + MBEDTLS_ERR_ASN1_LENGTH_MISMATCH;
            }
            break;
        case 58:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_EXTENSIONS + MBEDTLS_ERR_ASN1_INVALID_LENGTH;
            }
            break;
        case 59:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_EXTENSIONS;
            }
            break;
        case 60:
            {
                *out_value = MBEDTLS_ERR_X509_SIG_MISMATCH;
            }
            break;
        case 61:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_SIGNATURE + MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 62:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_SIGNATURE + MBEDTLS_ERR_ASN1_INVALID_DATA;
            }
            break;
        case 63:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_FORMAT + MBEDTLS_ERR_ASN1_LENGTH_MISMATCH;
            }
            break;
        case 64:
            {
                *out_value = MBEDTLS_ERR_X509_UNKNOWN_SIG_ALG;
            }
            break;
        case 65:
            {
                *out_value = MBEDTLS_ERR_ASN1_OUT_OF_DATA;
            }
            break;
        case 66:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_DATE + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 67:
            {
                *out_value = MBEDTLS_X509_MAX_INTERMEDIATE_CA;
            }
            break;
        case 68:
            {
                *out_value = MBEDTLS_X509_MAX_INTERMEDIATE_CA-1;
            }
            break;
        case 69:
            {
                *out_value = MBEDTLS_X509_MAX_INTERMEDIATE_CA+1;
            }
            break;
        case 70:
            {
                *out_value = MBEDTLS_ERR_X509_FATAL_ERROR;
            }
            break;
        case 71:
            {
                *out_value = -1;
            }
            break;
        case 72:
            {
                *out_value = MBEDTLS_ERR_X509_BAD_INPUT_DATA;
            }
            break;
        case 73:
            {
                *out_value = MBEDTLS_X509_BADCERT_BAD_MD|MBEDTLS_X509_BADCERT_BAD_PK|MBEDTLS_X509_BADCERT_BAD_KEY;
            }
            break;
        case 74:
            {
                *out_value = MBEDTLS_X509_BADCERT_BAD_PK|MBEDTLS_X509_BADCERT_BAD_KEY;
            }
            break;
        case 75:
            {
                *out_value = MBEDTLS_X509_BADCERT_BAD_MD|MBEDTLS_X509_BADCERT_BAD_KEY;
            }
            break;
        case 76:
            {
                *out_value = -2;
            }
            break;
        case 77:
            {
                *out_value = -4;
            }
            break;
        case 78:
            {
                *out_value = -3;
            }
            break;
        case 79:
            {
                *out_value = MBEDTLS_ERR_OID_BUF_TOO_SMALL;
            }
            break;
        case 80:
            {
                *out_value = MBEDTLS_X509_KU_DIGITAL_SIGNATURE|MBEDTLS_X509_KU_KEY_ENCIPHERMENT;
            }
            break;
        case 81:
            {
                *out_value = MBEDTLS_X509_KU_KEY_CERT_SIGN;
            }
            break;
        case 82:
            {
                *out_value = MBEDTLS_X509_KU_DIGITAL_SIGNATURE;
            }
            break;
        case 83:
            {
                *out_value = MBEDTLS_X509_KU_KEY_CERT_SIGN|MBEDTLS_X509_KU_CRL_SIGN;
            }
            break;
        case 84:
            {
                *out_value = MBEDTLS_X509_KU_KEY_ENCIPHERMENT|MBEDTLS_X509_KU_KEY_AGREEMENT;
            }
            break;
        case 85:
            {
                *out_value = MBEDTLS_X509_KU_DIGITAL_SIGNATURE|MBEDTLS_X509_KU_KEY_ENCIPHERMENT|MBEDTLS_X509_KU_DECIPHER_ONLY;
            }
            break;
        case 86:
            {
                *out_value = MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE;
            }
            break;
        case 87:
            {
                *out_value = MBEDTLS_MD_SHA1;
            }
            break;
        case 88:
            {
                *out_value = MBEDTLS_ASN1_SEQUENCE;
            }
            break;
        case 89:
            {
                *out_value = MBEDTLS_MD_SHA256;
            }
            break;
        case 90:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_ALG + MBEDTLS_ERR_ASN1_INVALID_DATA;
            }
            break;
        case 91:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_ALG + MBEDTLS_ERR_OID_NOT_FOUND;
            }
            break;
        case 92:
            {
                *out_value = MBEDTLS_ERR_X509_FEATURE_UNAVAILABLE + MBEDTLS_ERR_OID_NOT_FOUND;
            }
            break;
        case 93:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_ALG;
            }
            break;
        case 94:
            {
                *out_value = MBEDTLS_ERR_PK_KEY_INVALID_FORMAT + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 95:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_SIGNATURE + MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
            }
            break;
        case 96:
            {
                *out_value = MBEDTLS_ERR_PEM_INVALID_DATA + MBEDTLS_ERR_BASE64_INVALID_CHARACTER;
            }
            break;
        case 97:
            {
                *out_value = MBEDTLS_ASN1_UTC_TIME;
            }
            break;
        case 98:
            {
                *out_value = MBEDTLS_ASN1_GENERALIZED_TIME;
            }
            break;
        case 99:
            {
                *out_value = MBEDTLS_ASN1_CONTEXT_SPECIFIC;
            }
            break;
        case 100:
            {
                *out_value = MBEDTLS_ERR_X509_INVALID_DATE+MBEDTLS_ERR_ASN1_UNEXPECTED_TAG;
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

#if defined(MBEDTLS_BIGNUM_C)

        case 0:
            {
#if defined(MBEDTLS_PEM_PARSE_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 1:
            {
#if defined(MBEDTLS_RSA_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 2:
            {
#if defined(MBEDTLS_SHA1_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 3:
            {
#if defined(MBEDTLS_MD2_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 4:
            {
#if defined(MBEDTLS_MD4_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 5:
            {
#if defined(MBEDTLS_MD5_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 6:
            {
#if defined(MBEDTLS_SHA256_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 7:
            {
#if defined(MBEDTLS_SHA512_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 8:
            {
#if defined(MBEDTLS_X509_RSASSA_PSS_SUPPORT)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 9:
            {
#if defined(MBEDTLS_ECDSA_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 10:
            {
#if defined(MBEDTLS_ECP_DP_SECP256R1_ENABLED)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 11:
            {
#if defined(MBEDTLS_ECP_DP_SECP192R1_ENABLED)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 12:
            {
#if defined(MBEDTLS_X509_ALLOW_EXTENSIONS_NON_V3)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 13:
            {
#if defined(MBEDTLS_HAVE_TIME_DATE)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 14:
            {
#if defined(MBEDTLS_ECP_DP_SECP384R1_ENABLED)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 15:
            {
#if defined(MBEDTLS_PKCS1_V15)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 16:
            {
#if defined(MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_CERTIFICATES)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 17:
            {
#if !defined(MBEDTLS_TLS_DEFAULT_ALLOW_SHA1_IN_CERTIFICATES)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 18:
            {
#if defined(MBEDTLS_X509_CHECK_KEY_USAGE)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 19:
            {
#if defined(MBEDTLS_ECP_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 20:
            {
#if !defined(MBEDTLS_HAVE_TIME_DATE)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 21:
            {
#if defined(MBEDTLS_CERTS_C)
                ret = DEPENDENCY_SUPPORTED;
#else
                ret = DEPENDENCY_NOT_SUPPORTED;
#endif
            }
            break;
        case 22:
            {
#if defined(MBEDTLS_X509_USE_C)
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

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_x509_cert_info_wrapper,
#else
    NULL,
#endif
/* Function Id: 1 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRL_PARSE_C)
    test_mbedtls_x509_crl_info_wrapper,
#else
    NULL,
#endif
/* Function Id: 2 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRL_PARSE_C)
    test_mbedtls_x509_crl_parse_wrapper,
#else
    NULL,
#endif
/* Function Id: 3 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CSR_PARSE_C)
    test_mbedtls_x509_csr_info_wrapper,
#else
    NULL,
#endif
/* Function Id: 4 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_x509_verify_info_wrapper,
#else
    NULL,
#endif
/* Function Id: 5 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C) && defined(MBEDTLS_X509_CRL_PARSE_C) && defined(MBEDTLS_ECP_RESTARTABLE) && defined(MBEDTLS_ECDSA_C)
    test_x509_verify_restart_wrapper,
#else
    NULL,
#endif
/* Function Id: 6 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C) && defined(MBEDTLS_X509_CRL_PARSE_C)
    test_x509_verify_wrapper,
#else
    NULL,
#endif
/* Function Id: 7 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_x509_verify_callback_wrapper,
#else
    NULL,
#endif
/* Function Id: 8 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_mbedtls_x509_dn_gets_wrapper,
#else
    NULL,
#endif
/* Function Id: 9 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_mbedtls_x509_time_is_past_wrapper,
#else
    NULL,
#endif
/* Function Id: 10 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_mbedtls_x509_time_is_future_wrapper,
#else
    NULL,
#endif
/* Function Id: 11 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_CRT_PARSE_C) && defined(MBEDTLS_FS_IO)
    test_x509parse_crt_file_wrapper,
#else
    NULL,
#endif
/* Function Id: 12 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_x509parse_crt_wrapper,
#else
    NULL,
#endif
/* Function Id: 13 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_CRL_PARSE_C)
    test_x509parse_crl_wrapper,
#else
    NULL,
#endif
/* Function Id: 14 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_CSR_PARSE_C)
    test_mbedtls_x509_csr_parse_wrapper,
#else
    NULL,
#endif
/* Function Id: 15 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_mbedtls_x509_crt_parse_path_wrapper,
#else
    NULL,
#endif
/* Function Id: 16 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_mbedtls_x509_crt_verify_max_wrapper,
#else
    NULL,
#endif
/* Function Id: 17 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C)
    test_mbedtls_x509_crt_verify_chain_wrapper,
#else
    NULL,
#endif
/* Function Id: 18 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_USE_C)
    test_x509_oid_desc_wrapper,
#else
    NULL,
#endif
/* Function Id: 19 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_USE_C)
    test_x509_oid_numstr_wrapper,
#else
    NULL,
#endif
/* Function Id: 20 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C) && defined(MBEDTLS_X509_CHECK_KEY_USAGE)
    test_x509_check_key_usage_wrapper,
#else
    NULL,
#endif
/* Function Id: 21 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_FS_IO) && defined(MBEDTLS_X509_CRT_PARSE_C) && defined(MBEDTLS_X509_CHECK_EXTENDED_KEY_USAGE)
    test_x509_check_extended_key_usage_wrapper,
#else
    NULL,
#endif
/* Function Id: 22 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_USE_C)
    test_x509_get_time_wrapper,
#else
    NULL,
#endif
/* Function Id: 23 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_CRT_PARSE_C) && defined(MBEDTLS_X509_RSASSA_PSS_SUPPORT)
    test_x509_parse_rsassa_pss_params_wrapper,
#else
    NULL,
#endif
/* Function Id: 24 */

#if defined(MBEDTLS_BIGNUM_C) && defined(MBEDTLS_X509_CRT_PARSE_C) && defined(MBEDTLS_SELF_TEST)
    test_x509_selftest_wrapper,
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
    const char *default_filename = "./test_suite_x509parse.datax";
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

