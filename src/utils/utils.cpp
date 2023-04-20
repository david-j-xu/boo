/**
 * @file utils.cpp
 * @author David
 * @brief Utility functions
 * @version 0.1
 * @date 2023-04-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "utils.h"
namespace boo::utils {
u32 rotate_left(u32 val, u8 amount) {
    return (val << amount) | (val >> (32 - amount));
}

u32 rotate_right(u32 val, u8 amount) {
    return (val >> amount) | (val << (32 - amount));
}

int next_multiple(const int a, const int b) {
    if (a % b == 0) return a;
    return ((a / b) * b + b);
}
}  // namespace boo::utils
