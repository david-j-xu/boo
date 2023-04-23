/**
 * @file sha.h
 * @author David Xu
 * @brief SHA-1 Hash
 * @version 0.1
 * @date 2023-04-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "utils.h"

namespace boo {
class sha_obj {
   private:
    u32 h0;
    u32 h1;
    u32 h2;
    u32 h3;
    u32 h4;
    u64 hash;

   public:
    sha_obj(void);
    void update_block(std::string block);
    void update(std::string line);
    u64 get_hash();

    std::string get_hash_string();
};
}  // namespace boo
