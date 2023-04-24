/**
 * @file sha.cpp
 * @author David Xu
 * @brief SHA-1 hash
 * @version 0.1
 * @date 2023-04-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "sha.h"
#define BLOCK_SIZE 512
#define WORD_SIZE 32
#define WORD_COUNT 80

using namespace std;
using namespace boo;

namespace boo {
sha_obj::sha_obj() {
    hash = 0;
    h0 = 0x67452301;
    h1 = 0xEFCDAB89;
    h2 = 0x98BADCFE;
    h3 = 0x10325476;
    h4 = 0xC3D2E1F0;
}

/* takes in a block of length 512 and updates the hash value */
void sha_obj::update_block(string block) {
    char bl[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        if (i < block.size()) {
            bl[i] = block.c_str()[i];
        } else {
            bl[i] = 0;
        }
    }

    u32 w[WORD_COUNT];

    for (int i = 0; i < BLOCK_SIZE / WORD_SIZE; ++i) {
        for (int j = 0; j < WORD_SIZE / 8; ++j) {
            w[i] = w[i] << 8;
            w[i] += bl[i * (WORD_SIZE / 8) + j];
        }
    }

    for (int i = BLOCK_SIZE / WORD_SIZE; i < WORD_COUNT; ++i) {
        w[i] = bit_utils::rotate_left(
            w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    u32 a = h0;
    u32 b = h1;
    u32 c = h2;
    u32 d = h3;
    u32 e = h4;
    u32 f = 0;
    u32 k = 0;

    for (int i = 0; i < WORD_COUNT; ++i) {
        if (i < WORD_COUNT / 4) {
            f = (b & c) | (~b & d);
            k = 0x5A827999;
        } else if (i < 2 * WORD_COUNT / 4) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 3 * WORD_COUNT / 4) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        u32 temp = bit_utils::rotate_left(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = bit_utils::rotate_left(b, 30);
        b = a;
        a = temp;
    }

    h0 = h0 + a;
    h1 = h1 + b;
    h2 = h2 + c;
    h3 = h3 + d;
    h4 = h4 + e;
}

u64 sha_obj::get_hash() {
    return ((u64)h0 << 48) | ((u64)h1 << 36) | ((u64)h2 << 24) |
           ((u64)h3 << 12) | (u64)h4;
}

string sha_obj::get_hash_string() { return to_string(get_hash()); }

void sha_obj::update(string line) {
    for (int i = 0;
         i <
         bit_utils::next_multiple((int)line.length(), BLOCK_SIZE) / BLOCK_SIZE;
         ++i) {
        update_block(line.substr(i * BLOCK_SIZE, BLOCK_SIZE));
    }
}
}  // namespace boo