#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_pid_t.h>
#include <sys/signal.h>

#define P_CAT(x, y) x##y
#define CAT(x, y)   P_CAT(x, y)

typedef unsigned long long ull;
typedef signed long long   ill;
typedef char              *string;

#include <immintrin.h>

uint64_t rdrand_random() {
    uint64_t rand_val;
    while (!_rdrand64_step(&rand_val)) {}
    return rand_val;
}

double num() {
    uint64_t rdrand_val = rdrand_random();
    double   _num       = ((double) rdrand_val) / (double) UINT64_MAX;

    return _num;
}

ull rand_int(ull min, ull max) { return (ull) ((num() * (max - min)) + min); }

#define length(x) ((((x))->size))

#define format(fmts, ...)                                                                          \
    ({                                                                                             \
        string buf    = NULL;                                                                      \
        int    result = asprintf(&buf, fmts, __VA_ARGS__);                                         \
        if (result < 0) memoryFailure(__FILE__, __LINE__);                                         \
        buf;                                                                                       \
    })

void memoryFailure(string file, int line) {
    printf(
        "----------------------------\n"
        " Memory allocation failure!\n"
        " At file %s\n"
        " Line %i\n"
        "----------------------------\n",
        file,
        line);
    exit(SIGABRT);
}

// sanity check for memory failures
#define sanity(x)                                                                                  \
    if (x == NULL) memoryFailure(__FILE__, __LINE__)

#define zmalloc(size)                                                                              \
    ({                                                                                             \
        void *data = malloc(size);                                                                 \
        sanity(data);                                                                              \
        data;                                                                                      \
    })

#define zrealloc(ptr, size)                                                                        \
    ({                                                                                             \
        void *data = realloc(ptr, size);                                                           \
        sanity(data);                                                                              \
        data;                                                                                      \
    })

#define create(type)                    CAT(CAT(create_, type), _list)()
#define push(type, list, item)          CAT(CAT(push_, type), _list)(list, item)
#define append(type, list, item, index) CAT(CAT(append_, type), _list)(list, index, item)
#define remove(type, list, index)       CAT(CAT(remove_, type), _list)(list, index)

#define __create_primitive(type, lname)                                                            \
    typedef struct lname {                                                                         \
        ull   size;                                                                                \
        type *arr;                                                                                 \
    } lname;                                                                                       \
                                                                                                   \
    lname *CAT(create_, lname)() {                                                                 \
        lname *ptr = zmalloc(sizeof(lname));                                                       \
        ptr->size  = 0;                                                                            \
        ptr->arr   = zmalloc(sizeof(type));                                                        \
        return ptr;                                                                                \
    }                                                                                              \
                                                                                                   \
    lname *CAT(push_, lname)(lname * list, type item) {                                            \
        length(list)++;                                                                            \
        list->arr                     = zrealloc(list->arr, sizeof(type) * (length(list) + 1));    \
        list->arr[ length(list) - 1 ] = item;                                                      \
        return list;                                                                               \
    }                                                                                              \
                                                                                                   \
    lname *CAT(append_, lname)(lname * list, ull index, type value) {                              \
        length(list)++;                                                                            \
        type *new_ptr = zmalloc(sizeof(type) * (length(list) + 1));                                \
        ull   exp     = 0;                                                                         \
        for (ull i = 0; i < length(list) + 1; i++)                                                 \
            if (i != index) new_ptr[ exp++ ] = list->arr[ i ];                                     \
            else                                                                                   \
                new_ptr[ exp++ ] = value;                                                          \
        free(list->arr);                                                                           \
        list->arr = new_ptr;                                                                       \
        return list;                                                                               \
    }                                                                                              \
                                                                                                   \
    lname *CAT(remove_, lname)(lname * list, ull index) {                                          \
        length(list)--;                                                                            \
        type *new_ptr = zmalloc(sizeof(type) * (length(list) + 1));                                \
        ull   exp     = 0;                                                                         \
        for (ull i = 0; i < length(list) + 1; i++)                                                 \
            if (i != index) new_ptr[ exp++ ] = list->arr[ i ];                                     \
        free(list->arr);                                                                           \
        list->arr = new_ptr;                                                                       \
        return list;                                                                               \
    }                                                                                              \
    typedef lname CAT(__prim_, CAT(lname, _prim__))

#define __create_list_type(type) __create_primitive(type, CAT(type, _list))

typedef void *var;

__create_list_type(var);

#define var() __auto_type

typedef struct mark {
    var       bottom_of_stack;
    var_list *found_on_stack;
    var_list *allocated;

    ull last_alloc_cycle;
} mark;

#define expand(...)         __VA_ARGS__
#define iter_prim(ind, arr) expand(for (ull ind = 0; ind < arr->size; ind++))
#define iterate(f)          expand(iter_prim(f))
#define item(ind, ar)       (ar->arr[ ind ])
#define through             ,

static const var null = NULL;

#ifdef _WIN32
    #include <intrin.h>
// Returns the current CPU clock cycle.
uint64_t rdtsc() { return __rdtsc(); }
#else
// Returns the current CPU clock cycle.
uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t) hi << 32) | lo;
}
#endif

void markc_start(mark *gc, var bottom_of_stack) {
    gc->bottom_of_stack  = bottom_of_stack;
    gc->found_on_stack   = create(var);
    gc->allocated        = create(var);
    gc->last_alloc_cycle = rdtsc();
}

void markc_stop(mark *gc) {
    // clean up any memory allocated with us
    iterate(mem_ind through gc->allocated) {
        var ptr = item(mem_ind, gc->allocated);
        free(ptr);
    }

    // now clean shit up
    free(gc->allocated->arr);
    free(gc->allocated);
    free(gc->found_on_stack->arr);
    free(gc->found_on_stack);
}

// #define __BRAVE__

#ifndef __BRAVE__
    #define courage(x) ({})
#else
    #define courage(x) ({ x; })
#endif

var __markc_malloc(mark *gc, size_t size, ull depth, string __FILE, ull __LINE) {
    var addr = malloc(size);
    if (addr == NULL) {
        if (depth > 10) {
            printf(
                "[ Failed to allocate memory. ]\n[ How much? %lu ]\n[ Where? %s ]\n[ Why? I have "
                "no idea! ]\n[ Which "
                "line? %llu ]\nHope that helps! :P\n\n",
                size,
                __FILE,
                __LINE);
            exit(SIGSEGV);
        } else
            return __markc_malloc(gc, size, depth + 1, __FILE, __LINE);
    }

    var() g              = rdtsc();
    gc->last_alloc_cycle = g;

    push(var, gc->allocated, addr);

    return addr;
}

var __markc_realloc(mark *gc, var ptr, size_t size, ull depth, string __FILE, ull __LINE) {
    var addr = realloc(ptr, size);
    if (addr == NULL) {
        if (depth > 10) {
            printf(
                "[ Failed to reallocate memory. ]\n[ How much? %lu ]\n[ Where? %s ]\n[ Why? I have "
                "no idea! ]\n[ Which "
                "line? %llu ]\nHope that helps! :P\n\n",
                size,
                __FILE,
                __LINE);
            exit(SIGSEGV);
        } else
            return __markc_realloc(gc, ptr, size, depth + 1, __FILE, __LINE);
    }

    // remove old ptr
    iterate(pointer_i through gc->allocated) {
        if (item(pointer_i, gc->allocated) == ptr) remove(var, gc->allocated, pointer_i);
    }

    var() g              = rdtsc();
    gc->last_alloc_cycle = g;

    // push new ptr
    push(var, gc->allocated, addr);

    return addr;
}

#define markc_malloc(gc, size)       __markc_malloc(gc, size, 0, __FILE__, __LINE__)
#define markc_realloc(gc, ptr, size) __markc_realloc(gc, ptr, size, 0, __FILE__, __LINE__)

#define smalloc(size)       markc_malloc(gc, size)
#define srealloc(ptr, size) markc_realloc(gc, ptr, size)

void markc_free(mark *gc, var ptr) {
    ill index = -1;
    iterate(i through gc->allocated) {
        var it = item(i, gc->allocated);
        if (it == ptr) {
            index = i;
            break;
        }
    }

    if (index == -1)
        courage({
            printf("MarkC Error: Use after free in pointer %p\n", ptr);
            var _ptr = malloc(1);
            free(ptr);
            free(ptr);
        });

    var _ptr = gc->allocated->arr[ index ];

    remove(var, gc->allocated, index);
    free(_ptr);
}

typedef unsigned char byte;
typedef byte bool;

#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

const bool false = 0;
const bool true  = 1;

// Goofily genius amounts of trickery is used here to detect wether or not we own a pointer.
bool is_ptr(var data) {
    // Allocate a shared memory space for the parent and child process.
    bool *glob_var
        = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    pid_t p = fork();
    if (p < 0) { // Failed to fork. Cannot recover!!
        perror("MarkC Error: Fork failed");
        exit(SIGABRT);
    } else if (p == 0) {
        // Child process. Try to deallocate the pointer and then change the variable to true.
        *glob_var = false;        // Set *glob_var to false.
        int *x    = (int *) data; // Cast.
        bool data = *x;           // Deallocate.
        *glob_var = true;
        // If we deallocated it, We own it and it is a pointer. If not, We don't own that pointer.

        exit(0);
    } else { /* Parent process. Wait until the child process exits and then read the status,
             Deallocate the memory and return the output. */
        wait(NULL);
        bool output = *glob_var;

        munmap(glob_var, sizeof(bool));

        return output;
    }
}

#define cast(type, x) ((type) x)

#define deref(x) (*cast(var *, x))

void markc_check_ptr(mark *gc, var ptr) { //
    if (ptr == gc || ptr == gc->bottom_of_stack || ptr == gc->found_on_stack
        || ptr == gc->allocated)
        return;
    iterate(u through gc->found_on_stack) if (item(u, gc->found_on_stack) == ptr) return;

    var copy = ptr;
    while (is_ptr(copy)) {
        if (copy == gc || copy == gc->bottom_of_stack || gc->found_on_stack || copy == ptr)
            goto __LAST_LABEL;
        markc_check_ptr(gc, copy);

        if (is_ptr(copy)) push(var, gc->found_on_stack, copy);

    __LAST_LABEL:
        copy += sizeof(var);
    }
}

void markc_collect(mark *gc) {
    courage(printf("Running cleanup\n"));
    int __top_of_stack;
    var() top_addr    = cast(ull, &__top_of_stack);
    var() bottom_addr = cast(ull, gc->bottom_of_stack);

    if (top_addr < bottom_addr) { // Swap values if the stack direction is different
        var() temp  = top_addr;
        top_addr    = bottom_addr;
        bottom_addr = temp;
    }

    // top_addr--;
    // bottom_addr++;

    gc->found_on_stack = create(var);

    for (ull STACK_INDEX = top_addr; STACK_INDEX > bottom_addr; STACK_INDEX -= sizeof(var)) {
        if (!is_ptr(cast(var, STACK_INDEX))) continue;
        if (STACK_INDEX == cast(ull, gc) || STACK_INDEX == cast(ull, gc->found_on_stack)
            || STACK_INDEX == cast(ull, gc->allocated))
            continue;

        var STACK_VALUE = deref(STACK_INDEX);
        if (is_ptr(STACK_VALUE)) push(var, gc->found_on_stack, STACK_VALUE);

        markc_check_ptr(gc, STACK_VALUE);

        // iterate(g through thming) push(var, gc->found_on_stack, item(g, thming));
    }

    iterate(h through gc->allocated) {
        var  x             = item(h, gc->allocated);
        bool should_delete = true;
        iterate(f through gc->found_on_stack) {
            var comp = item(f, gc->found_on_stack);
            if (comp == x) { // shouldn't delete
                should_delete = false;
                break;
            }
        }

        if (should_delete) {
            courage(printf("Freed: %p\n", cast(var, h)));
            gc->allocated = remove(var, gc->allocated, h);
            free(x);
        } else {
            courage(printf("Not freeing: %p\n", x));
        }
    }

    // cleanup
    gc->last_alloc_cycle = rdtsc();
    // These are freed for some reason??? How???
    // free(gc->found_on_stack->arr);
    // free(gc->found_on_stack);
}

#include <math.h>
bool should_collect_memory(mark *gc) {
    float x = 0;
    x += rdtsc() - gc->last_alloc_cycle;
    ull log  = logf(x);
    ull log2 = log;
    for (ull i = 0; i < 5; i++) log2 *= log;

    bool out = floor(num() * log2) == 0;

    if (out) printf("Yo]\n");
    return out;
}

#ifdef MARKC_AUTO_COLLECT
    #define new(type, value)                                                                       \
        ({                                                                                         \
            if (should_collect_memory(&gc)) { markc_collect(&gc); }                                \
            var ptr            = markc_malloc(&gc, sizeof(type));                                  \
            *cast(type *, ptr) = value;                                                            \
            (type *) ptr;                                                                          \
        })
#else
    #define new(type, value)                                                                       \
        ({                                                                                         \
            var ptr            = markc_malloc(&gc, sizeof(type));                                  \
            *cast(type *, ptr) = value;                                                            \
            (type *) ptr;                                                                          \
        })
#endif
