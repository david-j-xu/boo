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

/**
 * @brief representation of a boo commit
 *
 */
struct commit_t {
    std::string message;
    std::string hash;

    /**
     * @brief Construct a new commit object
     *
     * @param message
     * @param hash
     */
    commit_t(std::string message, std::string hash);
};

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
     * @return std::unordered_map<std::string, std::string> a map of the current
     * hashes
     *
     */
    std::unordered_map<std::string, std::string> calculate_current_hashes();

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
     * @return std::filesystem::path the path to the meta file
     */
    std::filesystem::path get_meta_file_of_commit(std::string commit);

    /**
     * @brief Gets the filepath to the log file
     *
     * @return std::string the log filepath
     */
    std::string get_log_file();

    /**
     * @brief Parses the log file into a list of commits arranged
     * chronologically
     *
     * @return std::vector<commit> the commits
     */
    std::vector<commit_t> parse_log();

    /**
     * @brief Sets the head to the specified
     *
     * @param commit
     */
    void set_head(std::string commit);

    /**
     * @brief Gets the commit hash of the head
     *
     * @return std::string the head commit
     */
    std::string get_head();

    /**
     * @brief Get the head file name
     *
     * @return std::string the file name
     */
    std::string get_head_file();

    /**
     * @brief Gets the path to the commit folder
     *
     * @param commit the commit hash
     * @return filesystem::path the path to the folder
     */
    std::filesystem::path get_commit_folder(std::string commit);

    /**
     * @brief Parses a meta file for a commit, if it exists (if it doesn't,
     * returns empty map).
     *
     * @param commit the commit hash
     * @return std::unordered_map<std::string, std::string> a map from
     * filepath to hash
     */
    std::unordered_map<std::string, std::string> parse_meta_file(
        std::string commit);

    /**
     * @brief Returns whether a commit exists
     *
     * @param commit the commit hash
     * @return true if the commit exists
     * @return false otherwise
     */
    bool exists_commit(std::string commit);

    /**
     * @brief Calculates the difference between a from set of hashes (from
     * path to hash) and a to set of hashes
     *
     * @param from_hash the original hashes
     * @param to_hash the changed hashes
     * @return std::tuple<std::unordered_set<std::string>,
     * std::unordered_set<std::string>, std::unordered_set<std::string>> a
     * tuple of created_files, modified_files, and deleted_files
     */
    std::tuple<std::unordered_set<std::string>, std::unordered_set<std::string>,
               std::unordered_set<std::string>>
    calculate_diffs(std::unordered_map<std::string, std::string> from_hash,
                    std::unordered_map<std::string, std::string> to_hash);

    /**
     * @brief resets to a previous commit
     *
     * @param commit the commit to reset to
     * @param force to overwrite any staged changes
     * @return true if the reset was successful
     * @return false otherwise
     */
    bool reset(std::string commit, bool force);

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