
#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <unordered_map>

/*
## Utility macros

    Defined here because implementation of certain functions depend on them.

    - { EXPECT } is an assert which is a bit nicer to use.
    - { ERORR_IF } is essentially EXPECT negated, exists for semantic reasons.
    - { WARN_IF } raises a warning when the condition is fulfilled.
*/
#if PROJECT_BUILD_DEBUG
    #define EXPECT(condition, ...)   if (!(condition)) sphinx::terminate(__FILE__, __LINE__, #condition, __VA_ARGS__)
    #define ERROR_IF(condition, ...) if ((condition))  sphinx::terminate(__FILE__, __LINE__, #condition, __VA_ARGS__)
#else
    #define EXPECT(condition, ...)
    #define ERROR_IF(condition, ...)
#endif


template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

namespace sphinx {

    /*
    ## Aliases
    */
    typedef const char* cstr_t;
    typedef uint8_t     u8;
    typedef int16_t     s16;
    typedef int32_t     s32;
    typedef float       f32;
    typedef double      f64;

    /*
        Terminate the program with a message of where it happened, used by { EXPECT, ERROR_IF } macro defined above.
    */
    static inline void
    terminate(cstr_t file_name, int line, cstr_t condition_str, cstr_t message = nullptr) {

        std::printf(
            "[ERROR]\n* In: %s\n* Line: %d\n* Condition: %s\n",
            file_name,
            line,
            condition_str,
            message
        );

        if (message) {
            std::printf("* %s", message);
        }

        exit(1);
    }

    static inline int
    cstr_length(cstr_t str) {
        ERROR_IF(str == nullptr);

        int length = 0;
        while (str[length] != '\0') {
            ++length;
        }

        return length;
    }

} // sphinx
