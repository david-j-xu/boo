/**
 * @file boo.h
 * @author David Xu
 * @brief Toy version control system
 * @version 0.1
 * @date 2023-04-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <unistd.h>

#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "include/cxxopts.hpp"
#include "utils/sha.h"
#include "utils/utils.h"

namespace boo {
class Boo {
   public:
    static const std::unordered_set<std::string> commands;
    static std::unordered_map<std::string, std::string> command_descriptions;
    std::unordered_map<std::string, std::function<void(int, char*[])>>
        command_handlers;

    /**
     * @brief Construct a new Boo object
     *
     */
    Boo();

    /**
     * @brief Create a default Options object with no options
     *
     * @return cxxopts::Options default options
     */
    cxxopts::Options createOptions();

    /**
     * @brief Handle arguments
     *
     * @param argc the argument count
     * @param argv the argument values
     */
    void handle_args(int argc, char* argv[]);

    /**
     * @brief Handle the init function
     *
     * @param argc
     * @param argv
     */
    void handle_init(int argc, char* argv[]);

    /**
     * @brief Handle the commit function
     *
     * @param argc
     * @param argv
     */
    void handle_commit(int argc, char* argv[]);

    /**
     * @brief Handle the reset function
     *
     * @param argc
     * @param argv
     */
    void handle_reset(int argc, char* argv[]);

    /**
     * @brief Handle the log function
     *
     * @param argc
     * @param argv
     */
    void handle_log(int argc, char* argv[]);

    /**
     * @brief Handle the status function
     *
     * @param argc
     * @param argv
     */
    void handle_status(int argc, char* argv[]);

    /**
     * @brief Prints the available arguments
     *
     */
    void print_available_commands();

   private:
    bool verbose;
    void debug_log(std::string s);
};
}  // namespace boo

int main(int argc, char* argv[]);