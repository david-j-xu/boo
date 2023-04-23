/**
 * @file boo.cpp
 * @author David Xu
 * @brief toy version control system
 * @version 0.1
 * @date 2023-04-20
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "boo.h"

using namespace boo;
using namespace std;

#define INIT "init"
#define COMMIT "commit"
#define RESET "reset"
#define LOG "log"
#define STATUS "status"

#define BOO_DIR ".boo"
#define LOG_FILE_NAME "log"
#define META_FILE_NAME "meta"

namespace boo {
BooContext::BooContext() : repo_dir() {}

bool BooContext::load_existing_context() {
    debug_log("Attempting to load an existing Boo context");
    using namespace std::filesystem;
    auto curr_dir = absolute(current_path());

    unordered_set<string> visited;

    while (!visited.count(curr_dir.string())) {
        debug_log("Searching for boo instances in " + curr_dir.string());
        if (exists(curr_dir / BOO_DIR) && is_directory(curr_dir / BOO_DIR)) {
            debug_log("Found a boo instance at " +
                      (curr_dir / BOO_DIR).string());
            repo_dir = curr_dir;

            return true;
        }
        visited.insert(curr_dir.string());
        curr_dir = curr_dir.parent_path();
    }

    return false;
}

void BooContext::calculate_current_hashes() {
    namespace fs = std::filesystem;
    auto boo_dir = repo_dir / BOO_DIR;
    for (auto const& dir_entry : fs::recursive_directory_iterator(repo_dir)) {
        if (dir_entry.path().string().starts_with(boo_dir.string())) {
            continue;
        }

        if (dir_entry.is_regular_file()) {
            ifstream file(dir_entry.path());
            stringstream buffer;
            sha_obj file_hash;
            if (file) {
                buffer << file.rdbuf();
                commit_hash.update(buffer.str());
                file_hash.update(buffer.str());

                file_hashes[dir_entry.path().string()] =
                    file_hash.get_hash_string();
                debug_log("Hashed " + dir_entry.path().string() + " to " +
                          file_hash.get_hash_string());
            }
        }
    }
    debug_log("Commit hash: " + commit_hash.get_hash_string());
}

bool BooContext::create_context() {
    debug_log("Creating a new Boo context in the pwd");
    using namespace std::filesystem;
    repo_dir = current_path();
    path boo_path = repo_dir / BOO_DIR;
    if (exists(boo_path)) {
        // context already exists!
        return false;
    }

    if (create_directory(boo_path)) {
        // create commit log
        ofstream infoFile(get_log_file());
        infoFile.close();
        return true;
    }
    return false;
}

bool BooContext::commit(string message) {
    namespace fs = std::filesystem;
    if (repo_dir.empty()) {
        debug_log("Unable to commit, was this context initialized?");
        return false;
    }
    auto boo_dir = repo_dir / BOO_DIR;
    // update for current time (if you wanna commit again)
    commit_hash.update(
        to_string(chrono::system_clock::now().time_since_epoch().count()));

    fs::path commit_dir = boo_dir / commit_hash.get_hash_string();
    if (!fs::create_directory(commit_dir)) {
        debug_log("Couldn't create commit directory...");
        return false;
    }

    log_commit(commit_hash.get_hash_string(), message);
    ofstream meta(get_meta_file_of_commit(commit_hash.get_hash_string()));

    // write metadata
    for (auto const& dir_entry : fs::recursive_directory_iterator(repo_dir)) {
        // ignore boo data
        if (dir_entry.path().string().starts_with(boo_dir.string())) {
            continue;
        }

        if (dir_entry.is_regular_file()) {
            meta << fs::absolute(dir_entry.path()) << endl;
            meta << file_hashes[dir_entry.path().string()] << endl << endl;
        }
    }

    meta.close();

    // copy commit data
    for (auto const& dir_entry : fs::directory_iterator(repo_dir)) {
        // ignore boo data
        if (dir_entry.path().string().starts_with(boo_dir.string())) {
            continue;
        }

        string rel_path_str = fs::absolute(dir_entry.path())
                                  .string()
                                  .substr(repo_dir.string().length() + 1);
        fs::copy(dir_entry.path(), commit_dir / rel_path_str);
        debug_log("Copying from " + dir_entry.path().string() + " to " +
                  (commit_dir / rel_path_str).string());
    }

    return true;
}

void BooContext::log_commit(string hash, string message) {
    ofstream log(get_log_file(), ios_base::app);
    log << hash << endl;
    log << message.length() << endl;
    log << message << endl << endl;
    log.close();
}

string BooContext::get_meta_file_of_commit(string commit) {
    return repo_dir / BOO_DIR / (META_FILE_NAME + commit);
}

string BooContext::get_log_file() { return repo_dir / BOO_DIR / LOG_FILE_NAME; }

const unordered_set<string> Boo::commands{INIT, COMMIT, RESET, LOG, STATUS};
unordered_map<string, string> Boo::command_descriptions{
    {INIT, "Initializes a repository here"},
    {COMMIT, "Commits to this repository, if it exists"},
    {RESET, "Reset to a commit"},
    {LOG, "See previous commits"},
    {STATUS, "See current repository status"},
};

Boo::Boo()
    : command_handlers{
          {INIT,
           bind(&Boo::handle_init, this, placeholders::_1, placeholders::_2)},
          {COMMIT,
           bind(&Boo::handle_commit, this, placeholders::_1, placeholders::_2)},
          {RESET,
           bind(&Boo::handle_reset, this, placeholders::_1, placeholders::_2)},
          {LOG,
           bind(&Boo::handle_log, this, placeholders::_1, placeholders::_2)},
          {STATUS,
           bind(&Boo::handle_status, this, placeholders::_1, placeholders::_2)},
      }, ctx() {}

void Boo::handle_init(int argc, char* argv[]) {
    debug_log("Handling INIT function");

    if (!ctx.create_context()) {
        cout << "Failed to create empty repository at this location. Is there "
                "already an open repository?"
             << endl;
    } else {
        cout << "Successfully initialized repository at the current directory"
             << endl;
    }
}

void Boo::handle_commit(int argc, char* argv[]) {
    debug_log("Handling COMMIT function");
    if (!ctx.load_existing_context()) {
        cout << "Unable to load repository in this or any parent directories. "
                "Have you initialized a Boo repository?"
             << endl;
        exit(-1);
    }
    ctx.calculate_current_hashes();
    ctx.commit("hi");
}

void Boo::handle_reset(int argc, char* argv[]) {
    debug_log("Handling RESET function");
}

void Boo::handle_log(int argc, char* argv[]) {
    debug_log("Handling LOG function");
}

void Boo::handle_status(int argc, char* argv[]) {
    debug_log("Handling STATUS function");
}

void debug_log(string s) {
    if (verbose) {
        cerr << "[DEBUG] " << s << endl;
    }
}

cxxopts::Options Boo::createOptions() {
    return cxxopts::Options("boo", "a minimalist version control system")
        .allow_unrecognised_options();
}

void Boo::handle_args(int argc, char* argv[]) {
    cxxopts::Options options = createOptions();
    options.add_options()("command", "The command to execute",
                          cxxopts::value<string>()->default_value(""))(
        "v, verbose", "Verbose mode",
        cxxopts::value<bool>()->default_value("false"))("n, boon", "boon!")(
        "h, help", "Print usage");

    options.parse_positional({"command"});
    auto result = options.parse(argc, argv);

    if (result["verbose"].as<bool>()) {
        verbose = true;
    }

    // Help command
    if (result["help"].count()) {
        debug_log("Received help command");
        cout << options.help() << endl;
        cout << "Available arguments are: " << endl;

        print_available_commands();
        exit(0);
    }

    if (result["boon"].as<bool>()) {
        debug_log("boon mode activated >:)");
        cout << "You right. Boon the goat!" << endl;
        exit(0);
    }

    string command = result["command"].as<string>();
    debug_log("Received argument: " + command);

    if (command.empty() || !commands.count(command)) {
        // no argument was passed
        debug_log("No or unrecognized command was passed");
        cout << "No command or unrecognized command was passed. Available "
                "commands are: "
             << endl;
        print_available_commands();

        exit(0);
    }

    // call the handlers for the command
    if (command_handlers.count(command)) {
        command_handlers[command](argc, argv);
    } else {
        cout << "Command not implemented. Available commands are: " << endl;
        print_available_commands();
    }
};

void Boo::print_available_commands() {
    for (auto itr = commands.begin(); itr != commands.end(); itr++) {
        string cmd = *itr;
        cout << cmd << "\t" << command_descriptions[cmd] << endl;
    }
    exit(0);
}

}  // namespace boo

int main(int argc, char* argv[]) {
    Boo boo;
    boo.handle_args(argc, argv);
}