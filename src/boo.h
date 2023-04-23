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

#include <chrono>
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
bool verbose = false;
/**
 * @brief Debug logging
 *
 * @param the log statement
 */
void debug_log(std::string s);

class BooContext {
   public:
    BooContext();

    /**
     * @brief Loads an existing Boo context, if it exists in this directory
     * or any ancestor directories
     *
     * @return true if able to load the context
     * @return false otherwise
     */
    bool load_existing_context();

    /**
     * @brief Create a Boo context
     *
     * @return true if created a new context
     * @return false otherwise
     */
    bool create_context();

    /**
     * @brief does a hash of each of the files, and a total hash for a commit
     * if it were to exist
     *
     */
    void calculate_current_hashes();

    /**
     * @brief Creates a commit. Contingent on hashes being calculated beforehand
     *
     * @param message the commit message
     * @return true if commit was successful
     * @return false otherwise
     */
    bool commit(std::string message);

    /**
     * @brief Logs a commit to the end of the log
     *
     * @param commit_hash
     * @param message
     */
    void log_commit(std::string commit_hash, std::string message);

    /**
     * @brief Get the meta filename of commit object
     *
     * @param commit the commit hash
     * @return std::string the path to the meta file
     */
    std::string get_meta_file_of_commit(std::string commit);

    /**
     * @brief Gets the filepath to the log file
     *
     * @return std::string the log filepath
     */
    std::string get_log_file();

   private:
    std::filesystem::path repo_dir;
    sha_obj commit_hash;  // the hash if we were to commit this
    std::unordered_map<std::string, std::string>
        file_hashes;  // the file hashes
};

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
    BooContext ctx;
};
}  // namespace boo

int main(int argc, char* argv[]);