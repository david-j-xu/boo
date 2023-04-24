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
#define HEAD_FILE_NAME "head"

namespace boo {
commit_t::commit_t(string hash, string message)
    : message(message), hash(hash) {}

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

void BooContext::set_head(string commit) {
    ofstream head(get_head_file(), ios::trunc);
    head << commit << endl;
}

string BooContext::get_head() {
    namespace fs = filesystem;
    if (fs::exists(get_head_file())) {
        ifstream head_f(get_head_file());
        string head;
        head_f >> head;
        return head;
    } else {
        // no head currently exists!
        return "";
    }
}

string BooContext::get_head_file() {
    return (repo_dir / BOO_DIR / HEAD_FILE_NAME).string();
}

bool BooContext::reset(string commit, bool force) {
    namespace fs = filesystem;
    if (!exists_commit(commit)) {
        return false;
    }

    auto commit_dir = get_commit_folder(commit);
    auto commit_hashes = parse_meta_file(commit);
    auto current_hashes = calculate_current_hashes();
    if (!force) {
        auto head_hashes = parse_meta_file(get_head());
        auto [h1, h2, h3] = calculate_diffs(head_hashes, current_hashes);
        // if any file has been created, modified, or deleted, we abort
        if (h1.size() || h2.size() || h3.size()) {
            return false;
        }
    }

    auto [add, mod, del] = calculate_diffs(current_hashes, commit_hashes);

    auto replace_file = [commit_dir, this](string file) {
        string rel_path = file.substr(repo_dir.string().size() + 1);
        fs::path to_del = file;
        fs::path to_copy = commit_dir / rel_path;

        debug_log("Replacing " + to_del.string() + " with " + to_copy.string());
        if (fs::exists(to_del)) fs::remove(to_del);
        if (fs::exists(to_copy)) fs::copy(to_copy, to_del);
    };

    for (auto const& file : add) replace_file(file);
    for (auto const& file : mod) replace_file(file);
    for (auto const& file : del) replace_file(file);
    set_head(commit);

    return true;
}

bool BooContext::exists_commit(string commit) {
    namespace fs = filesystem;
    fs::path commit_folder = get_commit_folder(commit);

    return fs::exists(commit_folder) && fs::is_directory(commit_folder);
}

unordered_map<string, string> BooContext::calculate_current_hashes() {
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

                file_hashes[fs::absolute(dir_entry.path()).string()] =
                    file_hash.get_hash_string();
                debug_log("Hashed " + fs::absolute(dir_entry.path()).string() +
                          " to " + file_hash.get_hash_string());
            }
        }
    }
    debug_log("Commit hash: " + commit_hash.get_hash_string());
    return file_hashes;
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
        return true;
    }
    return false;
}

tuple<unordered_set<string>, unordered_set<string>, unordered_set<string>>
BooContext::calculate_diffs(unordered_map<string, string> from_hash,
                            unordered_map<string, string> to_hash) {
    unordered_set<string> new_files;
    unordered_set<string> modified_files;
    unordered_set<string> deleted_files;

    for (const auto& curr : to_hash) {
        string curr_path = curr.first;
        string curr_hash = curr.second;

        if (!from_hash.contains(curr_path)) {
            // only exists currently, so this must be a new file
            new_files.insert(curr_path);
        } else {
            if (curr_hash != from_hash[curr_path]) {
                // the hashes don't match, but they exist both now and in the
                // past, so this must be modified
                debug_log("Detected modified hash for " + curr_path + " from " +
                          from_hash[curr_path] + " to " + curr_hash);
                modified_files.insert(curr_path);
            }
            // remove from head hashes
            from_hash.erase(curr_path);
        }
    }

    // all remaining elements in head hashes must be deleted files
    for (const auto& curr : from_hash) {
        deleted_files.insert(curr.first);
    }

    return make_tuple(new_files, modified_files, deleted_files);
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
    set_head(commit_hash.get_hash_string());

    // write metadata
    for (auto const& dir_entry : fs::recursive_directory_iterator(repo_dir)) {
        // ignore boo data
        if (dir_entry.path().string().starts_with(boo_dir.string())) {
            continue;
        }

        if (dir_entry.is_regular_file()) {
            meta << fs::absolute(dir_entry.path()).string() << endl;
            meta << file_hashes[dir_entry.path().string()] << endl << endl;
        }
    }

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
}

filesystem::path BooContext::get_meta_file_of_commit(string commit) {
    return repo_dir / BOO_DIR / (META_FILE_NAME + commit);
}

filesystem::path BooContext::get_commit_folder(string commit) {
    return repo_dir / BOO_DIR / commit;
}

unordered_map<string, string> BooContext::parse_meta_file(string commit) {
    debug_log("Parsing metafile for commit " + commit);
    namespace fs = filesystem;
    fs::path meta_path = get_meta_file_of_commit(commit);
    unordered_map<string, string> parsed;

    if (fs::exists(meta_path) && fs::is_regular_file(meta_path)) {
        ifstream meta_file(meta_path);

        while (!meta_file.eof() && !meta_file.bad()) {
            string filepath, hash;
            getline(meta_file, filepath);
            getline(meta_file, hash);
            meta_file.ignore(1);

            if (filepath.empty() || hash.empty()) continue;

            debug_log("Parsed filepath: " + filepath + " with hash " + hash);
            parsed[filepath] = hash;
        }
    }

    return parsed;
}

string BooContext::get_log_file() { return repo_dir / BOO_DIR / LOG_FILE_NAME; }

vector<commit_t> BooContext::parse_log() {
    debug_log("Parsing config file...");
    vector<commit_t> commits;
    ifstream log(get_log_file());

    string commit_hash, message;
    int message_length;
    while (!log.eof()) {
        log >> commit_hash >> message_length;

        // if the line was empty, just return
        if (log.eof()) break;

        // ignore new line
        log.ignore(1);

        char message_arr[message_length + 1];
        log.read(message_arr, message_length);

        message = string(message_arr, message_length);

        debug_log("Found commit " + commit_hash + " with message <" + message +
                  "> (" + to_string(message_length) + " bytes)");
        commits.push_back(commit_t(commit_hash, message));
        log.ignore(2);
    }

    return commits;
}

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
    auto options = createOptions();

    options.add_options()(
        "m, message", "Commit message",
        cxxopts::value<string>()->default_value("No message provided"))(
        "h, help", "Provide help");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << endl;
        exit(0);
    }

    ctx.calculate_current_hashes();
    ctx.commit(result["message"].as<string>());
}

void Boo::handle_reset(int argc, char* argv[]) {
    debug_log("Handling RESET function");
    auto options = createOptions();

    options.add_options()("c, commit", "Commit hash", cxxopts::value<string>())(
        "h, help", "Provide help")(
        "f, force", "Force reset (overwrite staged changes)",
        cxxopts::value<bool>()->default_value("false"));

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help() << endl;
        exit(0);
    }

    if (!result.count("commit")) {
        cout << "Reset requires a commit hash, passsed with the -c or --commit "
                "argument"
             << endl;
        exit(-1);
    }

    string commit = result["commit"].as<string>();
    bool force = result["force"].as<bool>();

    if (!ctx.load_existing_context()) {
        cout << "Unable to load repository in this or any parent directories. "
                "Have you initialized a Boo repository?"
             << endl;
        exit(-1);
    }

    if (ctx.reset(commit, force)) {
        cout << "Successfully reset to commit " + commit << endl;
    } else {
        cout << "Reset unsuccessful. You may be overwriting staged changes, "
                "for which you would need the -f tag. Otherwise, are you sure "
                "the commit exists?"
             << endl;
    }
}

void Boo::handle_log(int argc, char* argv[]) {
    debug_log("Handling LOG function");
    if (!ctx.load_existing_context()) {
        cout << "Unable to load repository in this or any parent directories. "
                "Have you initialized a Boo repository?"
             << endl;
        exit(-1);
    }
    auto commits = ctx.parse_log();
    string head_commit = ctx.get_head();

    for (auto itr = commits.rbegin(); itr != commits.rend(); ++itr) {
        commit_t commit = *itr;
        string head_msg =
            commit.hash == ctx.get_head() ? "\033[1;31m(HEAD)\033[0m" : "";
        cout << "Commit: " << commit.hash << "\t" << head_msg << "\n"
             << "Message: " << commit.message << "\n"
             << endl;
    }
}

void Boo::handle_status(int argc, char* argv[]) {
    if (!ctx.load_existing_context()) {
        cout << "Unable to load repository in this or any parent directories. "
                "Have you initialized a Boo repository?"
             << endl;
        exit(-1);
    }
    debug_log("Handling STATUS function");

    string head = ctx.get_head();
    auto current_hashes = ctx.calculate_current_hashes();
    auto head_hashes = ctx.parse_meta_file(head);

    auto [new_files, modified_files, deleted_files] =
        ctx.calculate_diffs(head_hashes, current_hashes);

    cout << "These are the current distances from the HEAD commit (" << head
         << ")" << endl;
    if (new_files.size()) {
        cout << "\033[1mNew Files:\033[0m"
             << "\n";
        cout << "\033[1;32m";
        for (const auto& file : new_files) {
            cout << "+\t" << file << endl;
        }
        cout << "\033[0m";
    }

    if (modified_files.size()) {
        cout << "\033[1mModified Files:\033[0m"
             << "\n";
        cout << "\033[1;33m";
        for (const auto& file : modified_files) {
            cout << "+/-\t" << file << endl;
        }
        cout << "\033[0m";
    }

    if (deleted_files.size()) {
        cout << "\033[1mDeleted Files:\033[0m"
             << "\n";
        cout << "\033[1;31m";
        for (const auto& file : deleted_files) {
            cout << "-\t" << file << endl;
        }
        cout << "\033[0m";
    }

    cout << "Run boo commit to commit these changes (they have been "
            "automatically staged)"
         << endl;
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

    if (result["boon"].as<bool>()) {
        debug_log("boon mode activated >:)");
        cout << "You right. Boon the goat!" << endl;
        exit(0);
    }

    string command = result["command"].as<string>();
    debug_log("Received argument: " + command);

    if (command.empty() || !commands.count(command)) {
        if (result["help"].count()) {
            debug_log("Received help command");
            cout << options.help() << endl;
            cout << "Available arguments are: " << endl;

            print_available_commands();
            exit(0);
        }
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
    } else if (result["help"].count()) {
        debug_log("Received help command");
        cout << options.help() << endl;
        cout << "Available arguments are: " << endl;

        print_available_commands();
        exit(0);
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

int main(int argc, char* argv[]) { Boo().handle_args(argc, argv); }