/**
 * @file utils.h
 * @author David Xu
 * @brief utility functions
 * @version 0.1
 * @date 2023-04-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once
#include <iostream>

#define u8 u_int8_t
#define u16 u_int16_t
#define u32 u_int32_t
#define u64 u_int64_t

namespace boo {
namespace bit_utils {
/* rotates a 64 bit value left by amount bits */
u32 rotate_left(u32 val, u8 amount);
/* rotates a 64 bit value right by amount bits */
u32 rotate_right(u32 val, u8 amount);
/* gets the next multiple of b larger than a */
int next_multiple(const int a, const int b);
}  // namespace bit_utils
}  // namespace boo