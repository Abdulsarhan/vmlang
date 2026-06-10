#ifndef DS_H
#define DS_H

#include <stdarg.h> /* for variadics */
#include <limits.h> /* for types */

#if defined(_WIN32)
  #if defined(__TINYC__)
    #define __declspec(x) __attribute__((x))
  #endif
  #if defined(DS_BUILD_SHARED)
    #define DSAPI __declspec(dllexport) /* We are building the library as a Win32 shared library (.dll) */
  #elif defined(DS_USE_SHARED)
    #define DSAPI __declspec(dllimport) /* We are using the library as a Win32 shared library (.dll) */
  #endif
#else
  #if defined(DS_BUILD_SHARED)
    #define DSAPI __attribute__((visibility("default"))) /* We are building as a Unix shared library (.so/.dylib) */
  #endif
#endif

#ifdef DS_STATIC
  #define DSAPI static
#else
  #define DSAPI extern
#endif

// : Clang OS/Arch Cracking
#if defined(__clang__)

# define COMPILER_CLANG 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__) || defined(__linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error This compiler/OS combo is not supported.
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define ARCH_X86 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# elif defined(__arm__)
#  define ARCH_ARM32 1
# else
#  error Architecture not supported.
# endif

// : MSVC OS/Arch Cracking

#elif defined(_MSC_VER)

# define COMPILER_MSVC 1

# if _MSC_VER >= 1920
#  define COMPILER_MSVC_YEAR 2019
# elif _MSC_VER >= 1910
#  define COMPILER_MSVC_YEAR 2017
# elif _MSC_VER >= 1900
#  define COMPILER_MSVC_YEAR 2015
# elif _MSC_VER >= 1800
#  define COMPILER_MSVC_YEAR 2013
# elif _MSC_VER >= 1700
#  define COMPILER_MSVC_YEAR 2012
# elif _MSC_VER >= 1600
#  define COMPILER_MSVC_YEAR 2010
# elif _MSC_VER >= 1500
#  define COMPILER_MSVC_YEAR 2008
# elif _MSC_VER >= 1400
#  define COMPILER_MSVC_YEAR 2005
# else
#  define COMPILER_MSVC_YEAR 0
# endif

# if defined(_WIN32)
#  define OS_WINDOWS 1
# else
#  error This compiler/OS combo is not supported.
# endif

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_IX86)
#  define ARCH_X86 1
# elif defined(_M_ARM64)
#  define ARCH_ARM64 1
# elif defined(_M_ARM)
#  define ARCH_ARM32 1
# else
#  error Architecture not supported.
# endif

// : GCC OS/Arch Cracking

#elif defined(__GNUC__) || defined(__GNUG__)

# define COMPILER_GCC 1

# if defined(__gnu_linux__) || defined(__linux__)
#  define OS_LINUX 1
# else
#  error This compiler/OS combo is not supported.
# endif

# if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#  define ARCH_X64 1
# elif defined(i386) || defined(__i386) || defined(__i386__)
#  define ARCH_X86 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# elif defined(__arm__)
#  define ARCH_ARM32 1
# else
#  error Architecture not supported.
# endif

#else
# error Compiler not supported.
#endif

// : Arch Cracking

#if defined(ARCH_X64)
# define ARCH_64BIT 1
#elif defined(ARCH_X86)
# define ARCH_32BIT 1
#endif

#if ARCH_ARM32 || ARCH_ARM64 || ARCH_X64 || ARCH_X86
# define ARCH_LITTLE_ENDIAN 1
#else
# error Endianness of this architecture not understood by context cracker.
#endif

// : Language Cracking

#if defined(__cplusplus)
# define LANG_CPP 1
#else
# define LANG_C 1
#endif

// : Build Option Cracking

#if !defined(BUILD_DEBUG)
# define BUILD_DEBUG 1
#endif

#if !defined(BUILD_SUPPLEMENTARY_UNIT)
# define BUILD_SUPPLEMENTARY_UNIT 0
#endif

#if !defined(BUILD_ENTRY_DEFINING_UNIT)
# define BUILD_ENTRY_DEFINING_UNIT 1
#endif

#if !defined(BUILD_CONSOLE_INTERFACE)
# define BUILD_CONSOLE_INTERFACE 0
#endif

#if !defined(BUILD_VERSION_MAJOR)
# define BUILD_VERSION_MAJOR 0
#endif

#if !defined(BUILD_VERSION_MINOR)
# define BUILD_VERSION_MINOR 0
#endif

#if !defined(BUILD_VERSION_PATCH)
# define BUILD_VERSION_PATCH 1
#endif

#define BUILD_VERSION_STRING_LITERAL stringify(BUILD_VERSION_MAJOR) "." stringify(BUILD_VERSION_MINOR) "." stringify(BUILD_VERSION_PATCH)
#if BUILD_DEBUG
# define BUILD_MODE_STRING_LITERAL_APPEND " [Debug]"
#else
# define BUILD_MODE_STRING_LITERAL_APPEND ""
#endif
#if defined(BUILD_GIT_HASH)
# define BUILD_GIT_HASH_STRING_LITERAL_APPEND " [" BUILD_GIT_HASH "]"
#else
# define BUILD_GIT_HASH_STRING_LITERAL_APPEND ""
#endif

#if !defined(BUILD_TITLE)
# define BUILD_TITLE "Untitled"
#endif

#if !defined(BUILD_RELEASE_PHASE_STRING_LITERAL)
# define BUILD_RELEASE_PHASE_STRING_LITERAL "ALPHA"
#endif

#define BUILD_TITLE_STRING_LITERAL BUILD_TITLE " (" BUILD_VERSION_STRING_LITERAL " " BUILD_RELEASE_PHASE_STRING_LITERAL ") - " __DATE__ "" BUILD_GIT_HASH_STRING_LITERAL_APPEND BUILD_MODE_STRING_LITERAL_APPEND

// : Zero All Undefined Options

#if !defined(ARCH_32BIT)
# define ARCH_32BIT 0
#endif
#if !defined(ARCH_64BIT)
# define ARCH_64BIT 0
#endif
#if !defined(ARCH_X64)
# define ARCH_X64 0
#endif
#if !defined(ARCH_X86)
# define ARCH_X86 0
#endif
#if !defined(ARCH_ARM64)
# define ARCH_ARM64 0
#endif
#if !defined(ARCH_ARM32)
# define ARCH_ARM32 0
#endif
#if !defined(COMPILER_MSVC)
# define COMPILER_MSVC 0
#endif
#if !defined(COMPILER_GCC)
# define COMPILER_GCC 0
#endif
#if !defined(COMPILER_CLANG)
# define COMPILER_CLANG 0
#endif
#if !defined(OS_WINDOWS)
# define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
# define OS_LINUX 0
#endif
#if !defined(OS_MAC)
# define OS_MAC 0
#endif
#if !defined(LANG_CPP)
# define LANG_CPP 0
#endif
#if !defined(LANG_C)
# define LANG_C 0
#endif

// : Unsupported Errors

#if ARCH_X86
# error You tried to build in x86 (32 bit) mode, but currently, only building in x64 (64 bit) mode is supported.
#endif
#if !ARCH_X64
# error You tried to build with an unsupported architecture. Currently, only building in x64 mode is supported.
#endif

// : Codebase Keywords

#define internal      static // internal linkage
#define global        static
#define local_persist static

#if COMPILER_MSVC || (COMPILER_CLANG && OS_WINDOWS)
# pragma section(".rdata$", read)
# define read_only __declspec(allocate(".rdata$"))
#elif (COMPILER_CLANG && OS_LINUX)
# define read_only __attribute__((section(".rodata")))
#else
// NOTE(rjf): I don't know of a useful way to do this in GCC land.
// __attribute__((section(".rodata"))) looked promising, but it introduces a
// strange warning about malformed section attributes, and it doesn't look
// like writing to that section reliably produces access violations, strangely
// enough. (It does on Clang)
# define read_only
#endif

#if COMPILER_MSVC
# define thread_static __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
# define thread_static __thread
#else
# error thread_static not defined for this compiler.
#endif

#if COMPILER_MSVC
# define force_inline __forceinline
#elif COMPILER_CLANG || COMPILER_GCC
# define force_inline __attribute__((always_inline))
#else
# error force_inline not defined for this compiler.
#endif

#if COMPILER_MSVC
# define no_inline __declspec(noinline)
#elif COMPILER_CLANG || COMPILER_GCC
# define no_inline __attribute__((noinline))
#else
# error no_inline not defined for this compiler.
#endif

// : Linkage Keyword Macros

#if OS_WINDOWS
# define shared_function C_LINKAGE __declspec(dllexport)
#else
# define shared_function C_LINKAGE
#endif

#if LANG_CPP
# define C_LINKAGE_BEGIN extern "C" {
# define C_LINKAGE_END }
# define C_LINKAGE extern "C"
#else
# define C_LINKAGE_BEGIN
# define C_LINKAGE_END
# define C_LINKAGE
#endif

// : Optimization Settings

#if COMPILER_MSVC
# define OPTIMIZE_BEGIN _Pragma("optimize(\"\", on)")
# define OPTIMIZE_END _Pragma("optimize(\"\", off)")
#elif COMPILER_CLANG
# define OPTIMIZE_BEGIN _Pragma("clang optimize on")
# define OPTIMIZE_END _Pragma("clang optimize off")
#elif COMPILER_GCC
# define OPTIMIZE_BEGIN _Pragma("GCC push_options") _Pragma("GCC optimize(\"O2\")")
# define OPTIMIZE_END _Pragma("GCC pop_options")
#else
# define OPTIMIZE_BEGIN
# define OPTIMIZE_END
#endif

#if COMPILER_MSVC && !BUILD_DEBUG
# define NO_OPTIMIZE_BEGIN _Pragma("optimize(\"\", off)")
# define NO_OPTIMIZE_END _Pragma("optimize(\"\", on)")
#elif COMPILER_CLANG && !BUILD_DEBUG
# define NO_OPTIMIZE_BEGIN _Pragma("clang optimize off")
# define NO_OPTIMIZE_END _Pragma("clang optimize on")
#elif COMPILER_GCC && !BUILD_DEBUG
# define NO_OPTIMIZE_BEGIN _Pragma("GCC push_options") _Pragma("GCC optimize(\"O0\")")
# define NO_OPTIMIZE_END _Pragma("GCC pop_options")
#else
# define NO_OPTIMIZE_BEGIN
# define NO_OPTIMIZE_END
#endif

////////////////////////////////
// : Versions

#define version(major, minor, patch) (u64)((((u64)(major) & 0xffff) << 32) | ((((u64)(minor) & 0xffff) << 16)) | ((((u64)(patch) & 0xffff) << 0)))
#define majorfromversion(version) (((version) & 0xffff00000000ull) >> 32)
#define minorfromversion(version) (((version) & 0x0000ffff0000ull) >> 16)
#define patchfromversion(version) (((version) & 0x00000000ffffull) >> 0)

/* : units */

#define kibibytes(n) ((size_t)(n) << 10)
#define mebibytes(n) ((size_t)(n) << 20)
#define gibibytes(n) ((size_t)(n) << 30)

#define Thousand(n)   ((n)*1000)
#define Million(n)    ((n)*1000000)
#define Billion(n)    ((n)*1000000000)

/* : Branch Predictor Hints */

#if defined(__clang__)
# define expect(expr, val) __builtin_expect((expr), (val))
#else
# define expect(expr, val) (expr)
#endif

#define likely(expr)            expect(expr,1)
#define unlikely(expr)          expect(expr,0)

/* :clamp, min and max */
#define Min(A,B) (((A)<(B))?(A):(B))
#define Max(A,B) (((A)>(B))?(A):(B))
#define clamp_top(A,X) Min(A,X)
#define clamp_bottom(X,B) Max(X,B)
#define clamp(low,X,high) (((X)<(low))?(low):((X)>(high))?(high):(X))

/* : Type -> Alignment */

#if COMPILER_MSVC
# define align_of(T) __alignof(T)
#elif COMPILER_CLANG
# define align_of(T) __alignof(T)
#elif COMPILER_GCC
# define align_of(T) __alignof__(T)
#else
# error alignof not defined for this compiler.
#endif

#if COMPILER_MSVC
# define align_type(x) __declspec(align(x))
#elif COMPILER_CLANG || COMPILER_GCC
# define align_type(x) __attribute__((aligned(x)))
#else
# error AlignType not defined for this compiler.
#endif


/* : Member Offsets */

#define member(T,m)                   (((T*)0)->m)
#define offset_of(T,m)                offsetof(T, m)
#define member_from_offset(T,ptr,off) (T)((((u8 *)ptr)+(off)))
#define member_from_ptr(T,ptr,m)      (void*)((((u8 *)ptr)+offset_of(T,m)))
#define cast_from_member(T,m,ptr)     (T*)(((u8*)ptr) - offset_of(T,m))

/* : for loop constructs */

#define defer_loop(begin, end)         for(int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define defer_loop_checked(begin, end) for(int _i_ = 2 * !(begin); (_i_ == 2 ? ((end), 0) : !_i_); _i_ += 1, (end))

#define each_index(it, count)    (u64 it = 0; it < (count); it += 1)
#define each_element(it, array)  (u64 it = 0; it < array_count(array); it += 1)
#define each_enum_val(type, it)  (type it = (type)0; it < type##_COUNT; it = (type)(it+1))
#define each_non_zero_enum_val   (type, it) (type it = (type)1; it < type##_COUNT; it = (type)(it+1))
#define each_in_range(it, range) (u64 it = (range).min; it < (range).max; it += 1)
#define each_node(it, T, first)  (T *it = first; it != 0; it = it->next)
#define each_bit(it, flags)      (U64 (_i_) = (flags), it = (flags) & -(flags); (_i_) != 0; (_i_) &= ((_i_) - 1), it = (flags) & -(flags))

// : Memory Operation Macros

#define memory_copy(dst, src, size)    memmove((dst), (src), (size))
#define memory_set(dst, byte, size)    memset((dst), (byte), (size))
#define memory_compare(a, b, size)     memcmp((a), (b), (size))

#define memory_copy_struct(d,s)  memory_copy((d),(s),sizeof(*(d)))
#define memory_copy_array(d,s)   memory_copy((d),(s),sizeof(d))
#define memory_copy_typed(d,s,c) memory_copy((d),(s),sizeof(*(d))*(c))
#define memory_copy_str8(dst, s) memory_copy(dst, (s).data, (s).length)

#define memory_zero(s,z)        memset((s),0,(z))
#define memory_zero_struct(s)   memory_zero((s),sizeof(*(s)))
#define memory_zero_array(a)    memory_zero((a),sizeof(a))
#define memory_zero_typed(m,c)  memory_zero((m),sizeof(*(m))*(c))

#define memory_match(a,b,z)     (memory_compare((a),(b),(z)) == 0)
#define memory_match_struct(a,b)  memory_match((a),(b),sizeof(*(a)))
#define memory_match_array(a,b)   memory_match((a),(b),sizeof(a))

#define memory_is_zero_struct(ptr) memory_is_zero((ptr), sizeof(*(ptr)))

// : Asserts

#if COMPILER_MSVC
# define trap() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GCC
# define trap() __builtin_trap()
#else
# error Unknown trap intrinsic for this compiler.
#endif

#define assert_always(x) do{if(!(x)) {trap();}}while(0)
#if BUILD_DEBUG
# define assert(x) assert_always(x)
#else
# define assert(x) (void)(x)
#endif

#define unused(value) (void)(value)
#define invalid_path      assert(!"Invalid Path!")
#define unimplemented     assert(!"unimplemented!")
#define unreachable       assert(!"unreachable!")
#define no_op             ((void)0)
#define static_assert(C, ID) global u8 glue(ID, __LINE__)[(C)?1:-1]

/* : misc helper macros */

#define BIT(x) (1 << x)
#define stringify_(s) #s
#define stringify(s) stringify_(s)

#define glue_(a,b) a##b
#define glue(a,b) glue_(a,b)

#define array_count(a) (sizeof(a) / sizeof((a)[0]))

#define ceil_integer_div(a,b) (((a) + (b) - 1)/(b))

#define swap(T,a,b) do{T t__ = a; a = b; b = t__;}while(0)

/* Arena things */
#define ARENA_PUSH_ARRAY_ALIGN(arena, T, n, align) (T *)arena_push(arena, sizeof(T) * n, align, 1)
#define ARENA_PUSH_ARRAY_ALIGN_NO_ZERO(arena, T, n, align) (T *)arena_push(arena, sizeof(T) * n, align, 0)
#define ARENA_PUSH_ARRAY(arena, T, n) ARENA_PUSH_ARRAY_ALIGN(arena, T, n, align_of(T))
#define ARENA_PUSH_ARRAY_NO_ZERO(arena, T, n) ARENA_PUSH_ARRAY_ALIGN_NO_ZERO(arena, T, n, align_of(T))

/* strings */
#define STR8_LIT(x) (string8){.data = (u8*)x, .length = sizeof(x) - 1}

/* booleans */
#define true 1
#define false 0

#if UCHAR_MAX == 0xFF
typedef unsigned char u8;
typedef signed char i8;
#else
#error "No 8-bit type available on this platform."
#endif
#if USHRT_MAX == 0xFFFF
typedef unsigned short u16;
typedef signed short i16;
#elif UINT_MAX == 0xFFFF
typedef unsigned int u16;
typedef signed int i16;
#else
#error "No 16-bit type available on this platform."
#endif
#if UINT_MAX == 0xFFFFFFFFUL
typedef unsigned int u32;
typedef signed int i32;
#elif ULONG_MAX == 0xFFFFFFFFUL
typedef unsigned long u32;
typedef signed long i32;
#else
#error "No 32-bit type available on this platform."
#endif
#if ULONG_MAX == 0xFFFFFFFFFFFFFFFFUL
typedef unsigned long u64;
typedef signed long int64_t;
#elif defined(ULLONG_MAX) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFFULL
typedef unsigned long long u64;
typedef signed long long i64;
#else
#error "No 64-bit type available on this platform."
#endif

typedef i8 b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

typedef float f32;
typedef double f64;

typedef struct mem_arena mem_arena;
struct mem_arena {
    u64 pos;
    u64 committed_size;
    u64 page_size;
    u64 reserved_size;
};

typedef struct da_header da_header;
struct da_header {
    u64 length;
    u64 capacity;
};

typedef struct aa_header aa_header;
struct aa_header {
    mem_arena *arena;
    u64 length;
};

typedef struct string8 string8;
struct string8 {
    u8 *data;
    u64 length;
};

typedef struct string16 string16;
struct string16 {
    u16 *data;
    u64 length;
};

typedef struct string32 string32;
struct string32 {
    u32 *data;
    u64 length;
};

typedef struct string_builder string_builder;
struct string_builder {
    mem_arena *arena;
    u64 length;   /* bytes written so far   */
    u8 *base_pointer;
};

typedef u64 (*hash_func_t)(string8 data);

typedef struct hm_header hm_header;
struct hm_header {
    u64          count;       // live entries
    u64          capacity;    // must be a power of two
    u32          elem_size;   // key_size + value_size
    u32          key_size;
    hash_func_t  hash_func;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* Arenas */
DSAPI mem_arena *arena_init(size_t size);
DSAPI void *arena_push(mem_arena *arena, size_t size, size_t alignment, b32 zero_out_the_memory);
DSAPI void arena_pop(mem_arena *arena, size_t size);
DSAPI void arena_pop_to(mem_arena *arena, size_t pos);
DSAPI void arena_clear(mem_arena *arena);
DSAPI void arena_destroy(mem_arena *arena);
DSAPI int arena_reset_region(const mem_arena *arena, void *region_start, size_t region_size);
DSAPI mem_arena *arena_get_scratch();

/* string slicing */
DSAPI string8 str_from_to(string8 str, size_t from, size_t to);
DSAPI string8 str_copy(mem_arena *arena, string8 str);
DSAPI string8 str_format(mem_arena *arena, string8 fmt, ...);
DSAPI string8 str_chop_from_right(string8 str, u64 n);
DSAPI string8 str_chop_from_left(string8 str, u64 n);
DSAPI string8 str_trim_from_right(string8 str);
DSAPI string8 str_trim_from_left(string8 str);
DSAPI string8 str_trim_from_both_sides(string8 str);
DSAPI string8 str_postfix(string8 str, u64 length);
DSAPI string8 str_prefix(string8 str, u64 length);

/* string compares */
DSAPI b32 str_are_strings_equal(string8 a, string8 b);

/* convert numbers to string */
DSAPI string8 str_from_char(mem_arena *arena, u8 ch);
DSAPI string8 str_from_i32(mem_arena *arena, i32 value);
DSAPI string8 str_from_i64(mem_arena *arena, i64 value);
DSAPI string8 str_from_f32(mem_arena *arena, f32 value);
DSAPI string8 str_from_f64(mem_arena *arena, f64 value);

/* convert string to number */
DSAPI i32 str_to_i32(string8 str);
DSAPI i64 str_to_i64(string8 str);
DSAPI f64 str_to_f64(string8 str);

/* convert a string to a c string */
DSAPI const char *str_to_cstr(mem_arena *arena, string8 str);

DSAPI string_builder sb_create(mem_arena *arena);
DSAPI string8 sb_append(string_builder *sb, string8 str);
DSAPI string8 sb_appendf(string_builder *sb, const char *fmt, ...);
DSAPI string8 sb_build(string_builder *sb);
DSAPI void sb_reset(string_builder *sb);
DSAPI void sb_chop_from_right(string_builder *sb, u64 n);
/* Hash Tables */
#define hm_get_header(hm)   ((hm_header*)(hm) - 1)
#define hm_count(hm)        (hm_get_header(hm)->count)
#define hm_capacity(hm)     (hm_get_header(hm)->capacity)

// hm_init(hm, KeyType, ValueType, hash_func)
//   hm must be typed as a pointer to your value struct/type.
#define hm_init(hm, key_type, value_type, hash_func) \
    ((hm) = hm_init_impl(sizeof(key_type), sizeof(value_type), (hash_func)))

// hm_push(hm, key, value)  –– inserts or overwrites
#define hm_push(hm, key, value)  \
    (hm_grow_if_needed(hm, 80), hm_push_impl((void*)(hm), (void*)&(key), (void*)&(value)))

// hm_get(hm, key)  –– returns pointer to value, or NULL if not found
#define hm_get(hm, key) (hm_get_impl((hm), (void*)&(key)))

// hm_delete(hm, key)  –– marks slot as deleted (tombstone)
#define hm_delete(hm, key) (hm_delete_impl((hm), (void*)&(key)))

// hm_grow(hm)  –– unconditionally resize to the next power of two
#define hm_grow(hm) ((hm) = hm_grow_internal(hm))

// hm_grow_if_needed(hm, load_factor)  –– resize only if load factor (0-100) is exceeded
#define hm_grow_if_needed(hm, load_factor) \
    (hm_count(hm) * 100 >= hm_capacity(hm) * (load_factor) ? hm_grow(hm) : (void)0)

#define hm_foreach(hm, key_type, value_type)                                             \
    for (key_type *key = NULL; value_type *value = NULL; u64 _it = 0;                    \
         hm_next_impl((hm), &_it, (void**)&key, (void**)&value); )

#define hm_free(hm) (free(hm_get_header(hm)))

internal u64 hm_fnv1a_hash(string8 string);
internal u64 hm_knuth_hash(string8 string);
internal u64 hm_fnv1_hash(string8 string);
internal u64 hm_djb2_hash(string8 string);
internal u64 hm_sdbm_hash(string8 string);
internal u64 hm_id_hash(string8 string);

#define get_aa_header(t) ((aa_header *)(t) - 1)
#define aa_create(T, size) (T *)aa_create_impl(size * sizeof(T))
#define aa_push(a, v)                                                                 \
    do {                                                                              \
        (void)arena_push(get_aa_header(a)->arena, sizeof(a[0]), 1, 1);                \
        (a)[get_aa_header(a)->length++] = (v);                                        \
    } while (0)
#define aa_swapback(a, i)                                 \
    do {                                                  \
        (a)[(i)] = aa_last(a);                            \
        arena_pop(get_aa_header(a)->arena, sizeof(a[0])); \
        get_aa_header(a)->length--;                       \
    } while (0)
#define aa_pop(a) (arena_pop(get_aa_header(a)->arena, sizeof(a[0])), a[--get_aa_header(a)->length])
#define aa_length(a) (get_aa_header(a)->length)
#define aa_last(a) (a[get_aa_header(a)->length - 1])
#define aa_capacity(a) (get_aa_header(a)->arena->reserved_size / sizeof(a[0]))

#define aa_clear(a)                                               \
    do {                                                          \
        aa_header *_h = get_aa_header(a);                         \
        _h->arena->pos = ARENA_BASE_POS + sizeof(aa_header);      \
        _h->length = 0;                                           \
    } while (0)

#define header(t) ((da_header *)(t) - 1)
#define da_reserve(a, n) (da_grow(a, 0, n)) /* reserve n bytes for the entire array. You can do this to avoid calling realloc() multiple times */
#define da_set_len(a, n) ((da_cap(a) < (size_t)(n) ? da_reserve((a), (size_t)(n)), 0 : 0), (a) ? header(a)->length = (size_t)(n) : 0)
#define da_cap(a) ((a) ? header(a)->capacity : 0)
#define da_len(a) ((a) ? (ptrdiff_t)header(a)->length : 0)
#define da_lenu(a) ((a) ? header(a)->length : 0)
#define da_push(a, v) (da_maybe_grow(a, 1), (a)[header(a)->length++] = (v))
#define da_push_many(a, v, n)                  \
    do {                                       \
        da_maybe_grow(a, n);                   \
        for (int i = 0; i < (n); i++) {        \
            (a)[header(a)->length++] = (v)[i]; \
        }                                      \
    } while (0)
#define da_pop(a) (header(a)->length--, (a)[header(a)->length])
#define da_expand_ptr(a, n) (da_maybe_grow(a, n), (n) ? (header(a)->length += (n), &(a)[header(a)->length - (n)]) : (a))
#define da_expand_index(a, n) (da_maybe_grow(a, n), (n) ? (header(a)->length += (n), header(a)->length - (n)) : da_len(a))
#define da_last(a) ((a)[header(a)->length - 1])
#define da_free(a) ((void)((a) ? ds_free(NULL, header(a)) : (void)0), (a) = NULL)
#define da_delete(a, i) da_delete_many(a, i, 1)
#define da_delete_many(a, i, n) (memmove(&(a)[i], &(a)[(i) + (n)], sizeof(a[0]) * (header(a)->length - (n) - (i))), header(a)->length -= (n))
#define da_swapback(a, i) ((a)[i] = da_last(a), header(a)->length -= 1)
#define da_ins_n(a, i, n) ((void)da_expand_index((a), (n)), memmove(&(a)[(i) + (n)], &(a)[i], sizeof(a[0]) * (header(a)->length - (n) - (i))))
#define da_ins(a, i, v) (da_ins_n((a), (i), 1), (a)[i] = (v))

#define da_maybe_grow(a, n) ((!(a) || header(a)->length + (n) > header(a)->capacity) ? (da_grow(a, n, 0), 0) : 0)

#define da_grow(a, b, c) ((a) = da_arrgrowf_wrapper((a), sizeof(a[0]), (b), (c)))


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// in C we use implicit assignment from these void*-returning functions to T*.
// in C++ these templates make the same code work
template <class T>
static T *da_arrgrowf_wrapper(T *a, size_t elemsize, size_t addlen, size_t min_cap) {
    return (T *)da_arrgrowf((void *)a, elemsize, addlen, min_cap);
}
#else
#define da_arrgrowf_wrapper da_arrgrowf
#endif

#endif // DS_H

#ifdef DS_IMPLEMENTATION
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Not to be confused with da_free */
#if defined(ds_realloc) && !defined(ds_free) || !defined(ds_realloc) && defined(ds_free)
#error "You must define both da_realloc and da_free, or neither."
#endif
#if !defined(ds_realloc) && !defined(ds_free)
#include <stdlib.h>
#define ds_realloc(c, p, s) realloc(p, s)
#define ds_free(c, p) free(p)
#endif

#define ALIGN_UP_POW2(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

#define ARENA_BASE_POS sizeof(mem_arena)

#define hm_slot_states_internal(header) \
    ((u8*)((header) + 1))

#define hm_entries_internal(header) \
    ((u8*)((header) + 1) + (header)->capacity * sizeof(u8))

#define hm_entry_internal(header, idx) \
    (hm_entries_internal(header) + (u64)(idx) * (header)->elem_size)


typedef enum hm_slot_state {
    HM_SLOT_EMPTY    = 0,   // never used
    HM_SLOT_OCCUPIED = 1,   // live entry
    HM_SLOT_DELETED  = 2,   // tombstone (lazy deletion)
}hm_slot_state;

static void *ds_memset(void *buf, int value, size_t count) {
    unsigned char *p = buf;
    unsigned char v = (unsigned char)value;
    size_t i = 0;

    for (; i < count; i++) {
        p[i] = v;
    }

    return buf;
}

void *ds_memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    size_t i = 0;

    for (; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

static char *ds_strcpy(char *dst, const char *src) {
    char *start = dst;
    while ((*start++ = *src++)) {
    };
    return dst;
}

/* ===========================================================
   ===============   ARENA IMPLEMENTATION   =================
   =========================================================== */
#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

DSAPI mem_arena *arena_init(size_t size) {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    size_t page_size = sys_info.dwPageSize;
    size = ALIGN_UP_POW2(size, page_size);

    mem_arena *arena = (mem_arena *)VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
    assert(arena && "Error: VirtualAlloc reserve failed!");

    size_t initial_commit = Max(page_size, ARENA_BASE_POS);
    initial_commit = ALIGN_UP_POW2(initial_commit, page_size);

    void *commit = VirtualAlloc(arena, initial_commit, MEM_COMMIT, PAGE_READWRITE);
    if (!commit) {
        fprintf(stderr, "VirtualAlloc initial commit failed: %lu\n", GetLastError());
        VirtualFree(arena, 0, MEM_RELEASE);
        exit(EXIT_FAILURE);
    }

    arena->page_size = page_size;
    arena->reserved_size = size;
    arena->pos = ARENA_BASE_POS;
    arena->committed_size = initial_commit;

    return arena;
}

DSAPI void *arena_push(mem_arena *arena, size_t size, size_t alignment, b32 zero_out_the_memory) {
    size_t base = (size_t)arena + arena->pos;
    uintptr_t aligned = ALIGN_UP_POW2(base, alignment);
    size_t padding = aligned - base;
    size_t total_size = padding + size;
    size_t required = arena->pos + total_size;

    assert(required < arena->reserved_size && "Error: Allocation exceeds arena reserved size!");

    if (required > arena->committed_size) {
        size_t new_commit_end = ALIGN_UP_POW2(required, arena->page_size);
        size_t commit_amount = new_commit_end - arena->committed_size;

        void *result = VirtualAlloc((char *)arena + arena->committed_size, commit_amount, MEM_COMMIT, PAGE_READWRITE);
        assert(result && "Error: VirtualAlloc commit failed!");

        arena->committed_size = new_commit_end;
    }

    arena->pos += total_size;

    if (zero_out_the_memory) {
        ds_memset((void *)aligned, 0, total_size);
    }

    return (void *)aligned;
}

DSAPI void arena_pop(mem_arena *arena, size_t size) {
    if (arena->pos - ARENA_BASE_POS < size) {
        arena->pos = ARENA_BASE_POS;
    } else {
        arena->pos -= size;
    }
}

DSAPI void arena_pop_to(mem_arena *arena, size_t pos) {
    pos = Max(pos, ARENA_BASE_POS);
    if (pos < arena->pos) {
        arena->pos = pos;
    }
}

DSAPI void arena_clear(mem_arena *arena) {
    if (arena) {
        size_t reset_size = arena->committed_size - ARENA_BASE_POS;
        if (reset_size > 0) {
            VirtualAlloc((char *)arena + ARENA_BASE_POS, reset_size, MEM_RESET, PAGE_READWRITE);
        }
        arena->pos = ARENA_BASE_POS;
    }
}

DSAPI void arena_destroy(mem_arena *arena) {
    if (arena) {
        arena->pos = 0;
        arena->committed_size = 0;
        arena->reserved_size = 0;
        arena->page_size = 0;
        VirtualFree(arena, 0, MEM_RELEASE);
    }
}

DSAPI int arena_reset_region(const mem_arena *arena, void *region_start, size_t region_size) {
    uintptr_t arena_start = (uintptr_t)arena + ARENA_BASE_POS;
    uintptr_t arena_end = (uintptr_t)arena + arena->reserved_size;
    uintptr_t region_addr = (uintptr_t)region_start;

    if (region_addr >= arena_start && region_addr + region_size <= arena_end) {
        ds_memset(region_start, 0, region_size);
        return 0;
    }
    return -1;
}

global u8 g_scratch_mem[16384];
DSAPI mem_arena *arena_get_scratch() {
    mem_arena *arena = (mem_arena*)g_scratch_mem;
    arena->pos = ARENA_BASE_POS;
    arena->committed_size = 16384;
    arena->reserved_size = 16384;
    arena->page_size = 4096;
    return arena;
}

#else

#include <sys/mman.h>
#include <unistd.h>

DSAPI mem_arena *arena_init(size_t size) {
    size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
    size = ALIGN_UP_POW2(size, page_size);

    mem_arena *arena = (mem_arena *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (arena == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    arena->page_size = page_size;
    arena->reserved_size = size;
    arena->pos = ARENA_BASE_POS;
    arena->committed_size = size; // mmap commits all at once on most systems

    return arena;
}

DSAPI void *arena_push(mem_arena *arena, size_t size, size_t alignment, ds_bool zero_out_the_memory) {
    size_t base = (size_t)arena + arena->pos;
    uintptr_t aligned = ALIGN_UP_POW2(base, alignment);
    size_t padding = aligned - base;
    size_t total_size = padding + size;
    size_t required = arena->pos + total_size;

    if (required > arena->reserved_size) {
        fprintf(stderr, "ERROR: Allocation exceeds arena reserved_size!\n");
        return NULL;
    }

    arena->pos += total_size;

    if (zero_out_the_memory) {
        ds_memset((void *)aligned, 0, total_size);
    }

    return (void *)aligned;
}

DSAPI void arena_pop(mem_arena *arena, size_t size) {
    if (arena->pos - ARENA_BASE_POS < size) {
        arena->pos = ARENA_BASE_POS;
    } else {
        arena->pos -= size;
    }
}

DSAPI void arena_pop_to(mem_arena *arena, size_t pos) {
    pos = ARENA_MAX(pos, ARENA_BASE_POS);
    if (pos < arena->pos) {
        arena->pos = pos;
    }
}

DSAPI void arena_clear(mem_arena *arena) {
    if (arena) {
        size_t reset_size = arena->pos - ARENA_BASE_POS;
        if (reset_size > 0) {
            madvise((char *)arena + ARENA_BASE_POS, reset_size, MADV_DONTNEED);
        }
        arena->pos = ARENA_BASE_POS;
    }
}

DSAPI void arena_destroy(mem_arena *arena) {
    if (arena) {
        munmap(arena, arena->reserved_size);
    }
}

DSAPI int arena_reset_region(const mem_arena *arena, void *region_start, size_t region_size) {
    uintptr_t arena_start = (uintptr_t)arena + ARENA_BASE_POS;
    uintptr_t arena_end = (uintptr_t)arena + arena->reserved_size;
    uintptr_t region_addr = (uintptr_t)region_start;

    if (region_addr >= arena_start && region_addr + region_size <= arena_end) {
        ds_memset(region_start, 0, region_size);
        return 0;
    }
    return -1;
}

DSAPI mem_arena arena_get_scratch(mem_arena *source) {
    mem_arena scratch = *source;
    return scratch;
}
#endif // _WIN32 || _WIN64


/* ===========================================================
   =======   STRINGS AND STRING BUILDER IMPLEMENTATION   =====
   =========================================================== */

DSAPI u8 ds_eat_char(u8 **at) {
    if (**at != '\0') {
        u8 c = **at;
        (*at)++;
        return c;
    }
    return '\0';
}

DSAPI u8 consume_char(u8 **at, u8 *end) {
    u8 orig = **at;
    if(*at != end) {
        at++;
        return orig;
    }
    return 0;
}

DSAPI b32 is_space(u8 ch) {
    return ch == '\t' ||
           ch == '\v' ||
           ch == '\f' ||
           ch == '\r' ||
           ch == '\n' ||
           ch == ' ';
}

DSAPI string8 str_from_to(string8 str, size_t from, size_t to) {
    if (from < 0) {
        from += (int)str.length;
    }
    if (to < 0) {
        to += (int)str.length;
    }

    if (from < 0) {
        from = 0;
    }
    if (to < 0) {
        to = 0;
    }
    if ((size_t)from > str.length) {
        from = (int)str.length;
    }
    if ((size_t)to > str.length) {
        to = (int)str.length;
    }

    string8 result = {0};

    if (from >= to) {
        result.data = str.data + str.length;
        result.length = 0;
    } else {
        result.data = str.data + from;
        result.length = to - from;
    }

    return result;
}

DSAPI string8 str_copy(mem_arena *arena, string8 str) {
    string8 result;
    result.data = (u8 *)arena_push(arena, str.length, 1, 0);
    result.length = str.length;
    memory_copy_str8(result.data, str);
    return result;
}

// STRING SLICING:

DSAPI string8 str_chop_from_right(string8 str, u64 n) {
    n = clamp_top(n, str.length);
    str.length -= n;
    return str;
}

DSAPI string8 str_chop_from_left(string8 str, u64 n) {
    n = clamp_top(n, str.length);
    str.data += n;
    str.length -= n;
    return str;
}

DSAPI string8 str_trim_from_left(string8 str) {
    while(str.length > 0 && is_space(str.data[0])) {
        str = str_chop_from_left(str, 1);
    }
    return str;
}

DSAPI string8 str_trim_from_right(string8 str) {
    while(str.length > 0 && is_space(str.data[str.length - 1])) {
        str = str_chop_from_right(str, 1);
    }
    return str;
}

DSAPI string8 str_trim_from_both_sides(string8 str) {
    str = str_trim_from_right(str);
    str = str_trim_from_left(str);
    return str;
}

DSAPI string8 str_postfix(string8 str, u64 length) {
  length = clamp_top(length, str.length);
  str.data = (str.data + str.length) - length;
  str.length = length;
  return str;
}

DSAPI string8 str_prefix(string8 str, u64 length) {
  str.length = clamp_top(length, str.length);
  return str;
}

DSAPI b32 str_are_strings_equal(string8 a, string8 b) {
    if (a.length != b.length) {
        return false;
    }
    for (size_t i = 0; i < a.length; i++) {
        if (a.data[i] != b.data[i]) {
            return false;
        }
    }
    return true;
}

DSAPI string8 str_from_char(mem_arena *arena, u8 ch) {
    string8 str;
    str.data = (u8 *)arena_push(arena, 1, 1, 0);
    str.length = 1;
    str.data[0] = ch;
    return str;
}

DSAPI string8 str_from_i32(mem_arena *arena, i32 value) {
    u8 tmp[32] = {0};
    int length = snprintf((char*)tmp, 32, "%d", value);

    string8 str;
    str.data = (u8 *)arena_push(arena, length, 1, 0);
    str.length = length;
    ds_memcpy(str.data, tmp, length);
    return str;
}

DSAPI string8 str_from_i64(mem_arena *arena, i64 value) {
    u8 tmp[64] = {0};
    int length = snprintf((char*)tmp, 64, "%lld", value);

    string8 str;
    str.data = (u8 *)arena_push(arena, length, 1, 0);
    str.length = length;
    ds_memcpy(str.data, tmp, length);
    return str;
}

DSAPI string8 str_from_f32(mem_arena *arena, f32 value) {
    u8 tmp[32] = {0};
    int length = snprintf((char*)tmp, 32, "%f", value);

    string8 str;
    str.data = (u8 *)arena_push(arena, length, 1, 0);
    str.length = length;
    ds_memcpy(str.data, tmp, length);
    return str;
}

DSAPI string8 str_from_f64(mem_arena *arena, f64 value) {
    u8 tmp[32] = {0};
    int length = snprintf((char*)tmp, 32, "%lf", value);

    string8 str;
    str.data = (u8 *)arena_push(arena, length, 1, 0);
    str.length = length;
    ds_memcpy(str.data, tmp, length);
    return str;
}

DSAPI i32 str_to_i32(string8 str) {
    i32 k = 0;
    u64 i = 0;
    i32 sign = 1;

    if (str.length > 0 && str.data[0] == '-') {
        sign = -1;
        i = 1;
    }

    for (; i < str.length; i++) {
        k = k * 10 + (str.data[i] - '0');
    }

    return sign * k;
}

DSAPI i64 str_to_i64(string8 str) {
    i64 k = 0;
    u64 i = 0;
    i32 sign = 1;

    if (str.length > 0 && str.data[0] == '-') {
        sign = -1;
        i = 1;
    }

    for (; i < str.length; i++) {
        k = k * 10 + (str.data[i] - '0');
    }

    return sign * k;
}

DSAPI f64 str_to_f64(string8 str) {
    // Find the decimal point
    u64 whole_part_len = str.length; // default: no decimal point
    for (u64 i = 0; i < str.length; i++) {
        if (str.data[i] == '.') {
            whole_part_len = i;
            break;
        }
    }

    string8 whole_part = (string8){ .data = str.data, .length = whole_part_len };
    i64 whole = str_to_i64(whole_part);

    f64 result = (f64)whole;

    // Only parse fractional part if a '.' was found
    if (whole_part_len < str.length) {
        u64 fractional_len = str.length - whole_part_len - 1;
        string8 fractional_part = str_chop_from_right(str, fractional_len);

        // Convert fractional digits as an integer, then scale down
        i64 frac_int = str_to_i64(fractional_part);
        f64 scale = 1.0;
        for (u64 i = 0; i < fractional_len; i++) scale *= 10.0;

        f64 fraction = (f64)frac_int / scale;

        // Match sign of whole part
        result += (whole < 0 || (whole_part_len == 1 && str.data[0] == '-'))
                  ? -fraction
                  : fraction;
    }

    return result;
}

DSAPI const char *str_to_cstr(mem_arena *arena, string8 str) {
    char *c_string = (char *)arena_push(arena, str.length + 1, 1, 1);
    ds_memcpy(c_string, str.data, str.length);
    return c_string;
}

DSAPI string8 str_format(mem_arena *arena, string8 fmt, ...) {
    va_list ap;
    u8 *at = fmt.data;
    u8 *end = fmt.data + fmt.length;
    u8 *start = (u8 *)arena + arena->pos;

    va_start(ap, fmt);

    while (at < end) {
        char c = ds_eat_char(&at);
        if (c == '%') {
            char spec = ds_eat_char(&at);
            switch (spec) {

                case 's': {
                    string8 s = va_arg(ap, string8);
                    ds_memcpy((char *)arena + arena->pos, s.data, s.length);
                    arena->pos += s.length;
                } break;

                case 'd': {
                    int d = va_arg(ap, int);
                    u8 tmp[32];
                    int written = snprintf((char*)tmp, 32, "%d", d);
                    ds_memcpy((char *)arena + arena->pos, tmp, written);
                    arena->pos += written;
                } break;

                case 'z': {
                    char next = ds_eat_char(&at);
                    if (next == 'u') {
                        size_t zu = va_arg(ap, size_t);
                        char tmp[32];
                        int written = sprintf(tmp, "%zu", zu);
                        ds_memcpy((char *)arena + arena->pos, tmp, written);
                        arena->pos += written;
                    }
                } break;

                default:
                    *((char *)arena + arena->pos) = spec;
                    arena->pos += 1;
                    break;
            }
        } else {
            *((char *)arena + arena->pos) = c;
            arena->pos += 1;
        }
    }

    va_end(ap);

    string8 result;
    result.data = start;
    result.length = (u8 *)arena + arena->pos - start;
    return result;
}

/* string builder */
DSAPI string_builder sb_create(mem_arena *arena) {
    string_builder sb = {0};
    sb.arena = arena;
    sb.length = 0;
    return sb;
}

DSAPI string8 sb_append(string_builder *sb, string8 str) {
    string8 out = {0};
    u8 *string_to_append = ARENA_PUSH_ARRAY_ALIGN(sb->arena, u8, str.length, 1);
    memory_copy_str8(string_to_append, str);
    sb->length += str.length;
    if(!sb->base_pointer) {
        sb->base_pointer = string_to_append;
    }
    out.data = sb->base_pointer;
    out.length = sb->length;
    return out;
}

DSAPI string8 sb_appendf(string_builder *sb, const char *fmt, ...) {
    string8 result = {0};
    if (!sb || !fmt) {
        return result;
    }

    va_list args;
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) {
        return result;
    }

    u8 *buf = ARENA_PUSH_ARRAY_ALIGN(sb->arena, u8, (size_t)needed, 1);

    if (!sb->base_pointer) {
        sb->base_pointer = buf;
    }

    va_start(args, fmt);
    vsnprintf((char *)buf, (size_t)needed + 1, fmt, args);
    va_end(args);

    sb->length += (size_t)needed;

    result.data = sb->base_pointer;
    result.length = sb->length;
    return result;
}

DSAPI string8 sb_build(string_builder *sb) {
    string8 out = {0};
    out.data = sb->base_pointer;
    out.length = sb->length;

    return out;
}

DSAPI void sb_chop_from_right(string_builder *sb, u64 n) {
    if (sb) {
        n = clamp_top(n, sb->length);
        arena_pop(sb->arena, n);
        sb->length -= n;
    }
}

DSAPI void sb_reset(string_builder *sb) {
    if (sb) {
        arena_pop(sb->arena, sb->length);
        sb->length = 0;
        sb->base_pointer = NULL;
    }
}

internal u64 hm_pos_hash(string8 pos) {
    f32 pos_x = 0;
    f32 pos_y = 0;
    memory_copy(&pos_x, pos.data, 4);
    memory_copy(&pos_y, pos.data, 4);

    u64 hash = ((u64)(u32)pos_x << 32) | (u32)pos_y;
    hash = (~hash) + (hash << 21);
    hash ^= hash >> 24;
    hash = (hash + (hash << 3)) + (hash << 8);
    hash ^= hash >> 14;
    hash = (hash + (hash << 2)) + (hash << 4);
    hash ^= hash >> 28;
    hash += hash << 31;
    return hash;
}

internal u64 hm_fnv1a_hash(string8 string) {
    u64 hash = 14695981039346656037ULL;
    for (u64 i = 0; i < string.length; i++) {
        hash ^= string.data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

internal u64 hm_knuth_hash(string8 string) {
    u64 hash = string.length;
    for (u64 i = 0; i < string.length; i++) {
        hash ^= string.data[i];
        hash *= 2654435761ULL;
    }
    return hash;
}

internal u64 hm_fnv1_hash(string8 string) {
    u64 hash = 14695981039346656037ULL;
    for (u64 i = 0; i < string.length; i++) {
        hash *= 1099511628211ULL;
        hash ^= string.data[i];
    }
    return hash;
}

internal u64 hm_djb2_hash(string8 string) {
    u64 hash = 5381;
    for (u64 i = 0; i < string.length; i++) {
        hash = ((hash << 5) + hash) + string.data[i]; // hash * 33 + c
    }
    return hash;
}

internal u64 hm_sdbm_hash(string8 string) {
    u64 hash = 0;
    for (u64 i = 0; i < string.length; i++) {
        hash = string.data[i] + (hash << 6) + (hash << 16) - hash; // hash * 65599 + c
    }
    return hash;
}

internal u64 hm_id_hash(string8 string) {
    u64 hash = 0;
    for (u64 i = 0; i < string.length && i < 8; i++) {
        hash |= (u64)string.data[i] << (i * 8);
    }
    return hash;
}
// ---------------------------------------------------------------------------
// hm_init_impl
// ---------------------------------------------------------------------------
internal void *hm_init_impl(u32 key_size, u32 value_size, hash_func_t hash_func) {
    u32 elem_size  = key_size + value_size;
    u64 capacity   = 256;
    u64 alloc_size = sizeof(hm_header)
                   + capacity * sizeof(u8)
                   + capacity * elem_size;

    u8 *mem = (u8*)malloc(alloc_size);
    memory_zero(mem, alloc_size);

    hm_header *header  = (hm_header*)mem;
    header->capacity   = capacity;
    header->count      = 0;
    header->elem_size  = elem_size;
    header->key_size   = key_size;
    header->hash_func  = hash_func;

    // Return pointer to just past the header; macros subtract 1 to get back.
    return (void*)(header + 1);
}

// ---------------------------------------------------------------------------
// hm_find_slot_internal
//   Returns the index of:
//     - the existing occupied slot with a matching key, OR
//     - the best empty/deleted slot to insert into.
//   Sets *out_found = true if an exact key match was located.
// ---------------------------------------------------------------------------
internal u64 hm_find_slot_internal(hm_header *header, void *key, b32 *out_found) {
    string8 key_str      = { (u8*)key, header->key_size };
    u64 hash             = header->hash_func(key_str);
    u64 capacity         = header->capacity;
    u64 mask             = capacity - 1;
    u8  *slot_states     = hm_slot_states_internal(header);
    u64  first_tombstone = (u64)-1;   // track first usable deleted slot

    for (u64 i = 0; i < capacity; i++) {
        u64 idx   = (hash + i) & mask;   // linear probing with wrap
        u8  state = slot_states[idx];

        if (state == HM_SLOT_EMPTY) {
            // Key definitely not present; use tombstone if we passed one.
            *out_found = false;
            return (first_tombstone != (u64)-1) ? first_tombstone : idx;
        }

        if (state == HM_SLOT_DELETED) {
            if (first_tombstone == (u64)-1) {
                first_tombstone = idx;
            }
            continue;   // keep probing — the key mighm be further along
        }

        // state == HM_SLOT_OCCUPIED: compare keys
        u8 *entry_key = hm_entry_internal(header, idx);
        if (memcmp(entry_key, key, header->key_size) == 0) {
            *out_found = true;
            return idx;
        }
    }

    // Table is full of occupied + deleted slots with no match.
    *out_found = false;
    return (first_tombstone != (u64)-1) ? first_tombstone : (u64)-1;
}

// ---------------------------------------------------------------------------
// hm_grow_internal  –– rebuild at double capacity, skipping tombstones
// ---------------------------------------------------------------------------
internal void *hm_grow_internal(void *hm_ptr) {
    hm_header *old_header = (hm_header*)hm_ptr - 1;
    u64 old_capacity      = old_header->capacity;
    u64 new_capacity      = old_capacity * 2;
    u32 elem_size         = old_header->elem_size;

    u64 alloc_size = sizeof(hm_header)
                   + new_capacity * sizeof(u8)
                   + new_capacity * elem_size;

    u8 *mem = (u8*)malloc(alloc_size);
    memory_zero(mem, alloc_size);

    hm_header *new_header  = (hm_header*)mem;
    new_header->capacity   = new_capacity;
    new_header->count      = 0;
    new_header->elem_size  = elem_size;
    new_header->key_size   = old_header->key_size;
    new_header->hash_func  = old_header->hash_func;

    u8 *old_slot_states = hm_slot_states_internal(old_header);

    // Re-insert every live entry into the new table.
    for (u64 i = 0; i < old_capacity; i++) {
        if (old_slot_states[i] != HM_SLOT_OCCUPIED){
            continue;
        }

        u8 *old_entry = hm_entry_internal(old_header, i);

        b32 found = false;
        u64 new_idx = hm_find_slot_internal(new_header, old_entry, &found);
        assert(!found); // no duplicates during rehash

        u8 *new_slot_states      = hm_slot_states_internal(new_header);
        new_slot_states[new_idx] = HM_SLOT_OCCUPIED;
        memory_copy(hm_entry_internal(new_header, new_idx), old_entry, elem_size);
        new_header->count++;
    }

    free(old_header);
    return (void*)(new_header + 1);
}

// ---------------------------------------------------------------------------
// hm_push_impl  –– insert or overwrite
// ---------------------------------------------------------------------------
internal void hm_push_impl(void *hm_ptr, void *key, void *value) {
    hm_header *header = (hm_header*)hm_ptr - 1;

    b32 found = false;
    u64 idx = hm_find_slot_internal(header, key, &found);
    assert(idx != (u64)-1);

    u8 *slot_states = hm_slot_states_internal(header);
    u8 *entry       = hm_entry_internal(header, idx);

    if (!found) {
        memory_copy(entry, key, header->key_size);
        header->count++;
    }
    // Overwrite value portion (key is unchanged for existing entries).
    memory_copy(entry + header->key_size, value, header->elem_size - header->key_size);
    slot_states[idx] = HM_SLOT_OCCUPIED;
}

// ---------------------------------------------------------------------------
// hm_get_impl  –– lookup; returns pointer to stored value or NULL
// ---------------------------------------------------------------------------
internal void *hm_get_impl(void *hm_ptr, void *key) {
    hm_header *header = (hm_header*)hm_ptr - 1;
    b32 found = false;
    u64 idx = hm_find_slot_internal(header, key, &found);
    if (!found) {
        return NULL;
    }
    return hm_entry_internal(header, idx) + header->key_size;
}

internal b32 hm_next_impl(void *hm_ptr, u64 *iter, void **out_key, void **out_value) {
    hm_header *header  = hm_get_header(hm_ptr);
    u8 *slot_states    = hm_slot_states_internal(header);

    while (*iter < header->capacity) {
        u64 i = (*iter)++;
        if (slot_states[i] == HM_SLOT_OCCUPIED) {
            u8 *entry  = hm_entry_internal(header, i);
            *out_key   = entry;
            *out_value = entry + header->key_size;
            return true;
        }
    }
    return false;
}
// ---------------------------------------------------------------------------
// hm_delete_impl  –– lazy deletion via tombstone
// ---------------------------------------------------------------------------
internal void hm_delete_impl(void *hm_ptr, void *key) {
    hm_header *header = (hm_header*)hm_ptr - 1;
    b32 found = false;
    u64 idx = hm_find_slot_internal(header, key, &found);
    if (!found) {
        return;
    }

    u8 *slot_states  = hm_slot_states_internal(header);
    slot_states[idx] = HM_SLOT_DELETED;
    header->count--;
}


/* dynamic arrays */
internal inline void *aa_create_impl(size_t reserve) {
    mem_arena *arena = arena_init(reserve);
    aa_header *h = (aa_header *)arena_push(arena, sizeof(aa_header), 16, 1);
    h->arena = arena;
    h->length = 0;
    return (void *)(h + 1);
}

void *da_arrgrowf(void *array, size_t elemsize, size_t addlen, size_t min_cap) {
    da_header temp = {0}; // force debugging
    void *realloced_array;
    size_t min_len = da_len(array) + addlen;
    (void)sizeof(temp);

    /* compute the minimum capacity needed */
    if (min_len > min_cap) {
        min_cap = min_len;
    }

    if (min_cap <= da_cap(array)) {
        return array;
    }

    /* increase needed capacity to guarantee O(1) amortized */
    if (min_cap < 2 * da_cap(array)) {
        min_cap = 2 * da_cap(array);
    } else if (min_cap < 4) {
        min_cap = 4;
    }

    realloced_array = ds_realloc(NULL, (array) ? header(array) : 0, elemsize * min_cap + sizeof(da_header));
    /* if (num_prev < 65536) prev_allocs[num_prev++] = (int *) (char *) b; */
    realloced_array = (char *)realloced_array + sizeof(da_header);
    if (array == NULL) {
        header(realloced_array)->length = 0;
    }
    header(realloced_array)->capacity = min_cap;

    return realloced_array;
}

#endif /* DS_IMPLEMENTATION */
