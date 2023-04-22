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

namespace boo {
const unordered_set<string> Boo::commands{INIT, COMMIT, RESET, LOG, STATUS};
unordered_map<string, string> Boo::command_descriptions{
    {INIT, "Initializes a repository here"},
    {COMMIT, "Commits to this repository, if it exists"},
    {RESET, "Reset to a commit"},
    {LOG, "See previous commits"},
    {STATUS, "See current repository status"},
};

Boo::Boo() : verbose(false) {
    command_handlers[INIT] =
        bind(&Boo::handle_init, this, placeholders::_1, placeholders::_2);
    command_handlers[COMMIT] =
        bind(&Boo::handle_commit, this, placeholders::_1, placeholders::_2);
    command_handlers[RESET] =
        bind(&Boo::handle_reset, this, placeholders::_1, placeholders::_2);
    command_handlers[LOG] =
        bind(&Boo::handle_log, this, placeholders::_1, placeholders::_2);
    command_handlers[STATUS] =
        bind(&Boo::handle_status, this, placeholders::_1, placeholders::_2);
}

void Boo::handle_init(int argc, char* argv[]) {
    debug_log("Handling INIT function");
}

void Boo::handle_commit(int argc, char* argv[]) {
    debug_log("Handling COMMIT function");
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

bool Boo::load_existing_context() {
    debug_log("Attempting to load an existing Boo context");
    return false;
}

bool Boo::create_context() {
    debug_log("Creating a new Boo context");
    return false;
}

void Boo::debug_log(string s) {
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