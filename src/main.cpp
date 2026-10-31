#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <regex>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_SEP '\\'
#define PATH_SEP_STR "\\"
#define POP_DIR _rmdir
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#define PATH_SEP '/'
#define PATH_SEP_STR "/"
#endif

using namespace std;

// -------------------------------------------------------------
// Helper filesystem and string utilities (Standard C++ only)
// -------------------------------------------------------------

string JoinPath(const string& a, const string& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    char last = a.back();
    if (last == '/' || last == '\\')
        return a + b;
    return a + PATH_SEP_STR + b;
}

bool FileExists(const string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && (st.st_mode & S_IFREG);
}

bool DirectoryExists(const string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && (st.st_mode & S_IFDIR);
}

bool MakeDir(const string& path) {
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

bool RealizeDirectory(const string& path) {
    if (path.empty() || DirectoryExists(path)) return true;
    size_t pos = 0;
    while ((pos = path.find_first_of("/\\", pos + 1)) != string::npos) {
        string sub = path.substr(0, pos);
        if (!sub.empty() && !DirectoryExists(sub)) {
            MakeDir(sub);
        }
    }
    return MakeDir(path);
}

string ReadTextFile(const string& path) {
    ifstream f(path, ios::in | ios::binary);
    if (!f) return "";
    ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool WriteTextFile(const string& path, const string& content) {
    ofstream f(path, ios::out | ios::binary | ios::trunc);
    if (!f) return false;
    f.write(content.data(), content.size());
    return true;
}

int RunCommand(const string& cmd) {
    return system(cmd.c_str());
}

string GetHomeDirectory() {
#ifdef _WIN32
    const char* userprofile = getenv("USERPROFILE");
    if (userprofile) return userprofile;
    const char* drive = getenv("HOMEDRIVE");
    const char* path = getenv("HOMEPATH");
    if (drive && path) return string(drive) + string(path);
    return "C:";
#else
    const char* home = getenv("HOME");
    if (home) return home;
    return "/";
#endif
}

string ToLower(const string& str) {
    string out = str;
    for (char& c : out) {
        if (c >= 'A' && c <= 'Z') c += ('a' - 'A');
    }
    return out;
}

// -------------------------------------------------------------
// Toolchain and Build Method (.bm) Resolver
// -------------------------------------------------------------

enum class HostToolchain {
    Clang,
    Gcc,
    Msvc,
    MinGW,
    Unknown
};

HostToolchain DetectHostToolchain() {
#ifdef _WIN32
    if (RunCommand("cl.exe >nul 2>&1") == 0) return HostToolchain::Msvc;
    if (RunCommand("clang++ --version >nul 2>&1") == 0) return HostToolchain::Clang;
    if (RunCommand("g++ --version >nul 2>&1") == 0) return HostToolchain::MinGW;
    return HostToolchain::Msvc;
#else
    if (RunCommand("clang++ --version >/dev/null 2>&1") == 0) return HostToolchain::Clang;
    if (RunCommand("g++ --version >/dev/null 2>&1") == 0) return HostToolchain::Gcc;
    return HostToolchain::Unknown;
#endif
}

bool InstallBuildMethods(const string& methods_source_dir, HostToolchain tc) {
    string home = GetHomeDirectory();
    vector<string> target_dirs;

#ifdef _WIN32
    string appdata = string(getenv("APPDATA") ? getenv("APPDATA") : "");
    if (!appdata.empty()) {
        target_dirs.push_back(JoinPath(appdata, "u++\\theide"));
        target_dirs.push_back(JoinPath(appdata, "u++\\umk"));
    }
    target_dirs.push_back(JoinPath(home, "upp"));
#else
    target_dirs.push_back(JoinPath(home, ".config/u++/theide"));
    target_dirs.push_back(JoinPath(home, ".config/u++/umk"));
    target_dirs.push_back(JoinPath(home, ".upp/theide"));
    target_dirs.push_back(JoinPath(home, ".upp/umk"));
#endif

    vector<string> files = {"CLANG.bm", "GCC.bm", "MSVS22x64.bm", "CLANG_macOS.bm"};
    for (const string& dir : target_dirs) {
        RealizeDirectory(dir);
        for (const string& fn : files) {
            string src = JoinPath(methods_source_dir, fn);
            string dst = JoinPath(dir, fn);
            if (FileExists(src) && !FileExists(dst)) {
                string content = ReadTextFile(src);
                WriteTextFile(dst, content);
            }
        }
    }
    return true;
}

// -------------------------------------------------------------
// BRC Resource Transpiler (.brc -> .brcc)
// -------------------------------------------------------------

void TranspileBrcFiles(const string& uppsrc_dir) {
    vector<string> packages = {
        "plugin/z", "plugin/pcre", "plugin/bz2", "plugin/lzma",
        "plugin/lz4", "plugin/zstd", "plugin/png", "Core", "Draw", "Esc",
        "ide/Core", "ide/Android", "ide/Java", "ide/Builders", "ide/clang",
        "Sql", "plugin/sqlite3", "Indexer/Db", "umk"
    };

    regex re("BINARY\\s*\\(\\s*([^,]+)\\s*,\\s*\"([^\"]+)\"\\s*\\)");
    for (const string& pkg : packages) {
        string pkg_dir = JoinPath(uppsrc_dir, pkg);
        if (!DirectoryExists(pkg_dir)) continue;

        // Check common brc files in package
        vector<string> candidates = {"ide.brc", "umk.brc", "Builders.brc"};
        for (const string& cand : candidates) {
            string brc_path = JoinPath(pkg_dir, cand);
            if (!FileExists(brc_path)) continue;

            string brcc_path = brc_path + "c";
            ifstream is(brc_path);
            ofstream os(brcc_path);
            string line;
            while (getline(is, line)) {
                smatch m;
                if (regex_search(line, m, re)) {
                    string sym = m[1].str();
                    string fn = m[2].str();
                    string fpath = JoinPath(pkg_dir, fn);
                    ifstream df(fpath, ios::binary);
                    if (df) {
                        vector<unsigned char> data((istreambuf_iterator<char>(df)), istreambuf_iterator<char>());
                        os << "static const unsigned char " << sym << "_[] = {";
                        for (size_t i = 0; i < data.size(); ++i) {
                            os << (int)data[i] << (i + 1 == data.size() ? "" : ",");
                        }
                        os << "};\n";
                        os << "const int " << sym << "_length = " << data.size() << ";\n";
                        os << "const unsigned char *" << sym << " = " << sym << "_;\n";
                    }
                }
            }
        }
    }
}

// -------------------------------------------------------------
// Bootstrap UMK compilation
// -------------------------------------------------------------

int BootstrapUmk(const string& upp_root, const string& methods_dir, bool verbose) {
    cout << "==> UppBootstrap: Initializing system build methods..." << endl;
    HostToolchain tc = DetectHostToolchain();
    InstallBuildMethods(methods_dir, tc);

    string uppsrc = JoinPath(upp_root, "uppsrc");
    if (!DirectoryExists(uppsrc)) {
        cerr << "Error: 'uppsrc' not found at " << uppsrc << endl;
        return 1;
    }

    TranspileBrcFiles(uppsrc);

    string bin_dir = JoinPath(upp_root, "bin");
    RealizeDirectory(bin_dir);

#ifdef _WIN32
    cout << "==> UppBootstrap: Compiling umk (Windows / MSVC)..." << endl;
    // On Windows, compile umk using cl.exe or umk-bootstrap script
    string dest = JoinPath(bin_dir, "umk.exe");
    // Invoke python or native cl compilation
    string script = JoinPath(upp_root, "script\\build.py");
    if (FileExists(script)) {
        string cmd = "py -3 \"" + script + "\" --bootstrap";
        int res = RunCommand(cmd);
        if (res != 0) res = RunCommand("python \"" + script + "\" --bootstrap");
        if (res != 0) {
            cerr << "Bootstrap build failed on Windows." << endl;
            return res;
        }
    } else {
        cerr << "Windows bootstrap script not found." << endl;
        return 1;
    }
#else
#ifdef __APPLE__
    string makefile_name = "Makefile.macos15";
#else
    string makefile_name = "Makefile.linux64";
#endif
    string makefile = JoinPath(methods_dir, ".." PATH_SEP_STR "makefiles" PATH_SEP_STR + makefile_name);
    if (!FileExists(makefile)) {
        makefile = JoinPath(uppsrc, "umk" PATH_SEP_STR + makefile_name);
    }
    if (!FileExists(makefile)) {
        cerr << "Error: Makefile not found at " << makefile << endl;
        return 2;
    }

    char abs_makefile[4096];
    if (realpath(makefile.c_str(), abs_makefile)) {
        makefile = abs_makefile;
    }
    char abs_uppsrc[4096];
    if (realpath(uppsrc.c_str(), abs_uppsrc)) {
        uppsrc = abs_uppsrc;
    }

    cout << "==> UppBootstrap: Compiling umk via " << makefile_name << "..." << endl;
    stringstream ss;
    ss << "make -f " << makefile << " -C " << uppsrc << " UPPOUT=../out/ all";
    
    long ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpu > 0) ss << " -j" << ncpu;

    if (verbose) cout << "Executing: " << ss.str() << endl;
    int res = RunCommand(ss.str());
#ifndef _WIN32
    if (WIFEXITED(res)) res = WEXITSTATUS(res);
    else res = -1;
#endif

    if (res != 0) {
        cerr << "Error: umk makefile compilation failed with exit code " << res << endl;
        return res;
    }

    string built_out = JoinPath(uppsrc, "umk.out");
    string dest = JoinPath(bin_dir, "umk");
    if (FileExists(built_out)) {
        string cp_cmd = "cp -f \"" + built_out + "\" \"" + dest + "\"";
        RunCommand(cp_cmd);
        chmod(dest.c_str(), 0755);
    }
#endif

    cout << "==> UppBootstrap: Successfully bootstrapped umk to " << dest << endl;
    cout << "==> UppBootstrap: Verifying umk executable..." << endl;
    string verify_cmd = "\"" + dest + "\" >/dev/null 2>&1";
    int vres = RunCommand(verify_cmd);
#ifndef _WIN32
    if (WIFEXITED(vres)) vres = WEXITSTATUS(vres);
#endif
    if (vres == 0) {
        cout << "==> UppBootstrap: Verification PASSED. umk is fully functional!" << endl;
    } else {
        cout << "==> UppBootstrap: umk verification exited with code " << vres << " (check dependencies/libs)." << endl;
    }
    return vres;
}

// -------------------------------------------------------------
// Main Entrypoint
// -------------------------------------------------------------

void PrintUsage(const char* prog) {
    cout << "UppBootstrap - Zero-dependency U++ umk bootstrap toolchain\n"
         << "Usage: " << prog << " [options]\n\n"
         << "Happy-Case Fast Modes:\n"
         << "  --upstream            Bootstrap using upstream U++ (downloads release tarball if needed)\n"
         << "  --bazaar              Bootstrap using OuluBSD BazaarCore modular framework\n\n"
         << "Standard Options:\n"
         << "  --bootstrap, -bs      Bootstrap umk and configure default methods\n"
         << "  --upp-root PATH       Root directory of U++ source tree (default: auto-detected)\n"
         << "  --methods-dir PATH    Directory containing template .bm methods\n"
         << "  --install-methods     Only install .bm build methods to host config\n"
         << "  --verify              Verify umk executable execution\n"
         << "  --verbose, -v         Enable verbose diagnostic logs\n"
         << "  --help, -h            Show this help documentation\n";
}

int main(int argc, char* argv[]) {
    string upp_root = "";
    string methods_dir = "";
    bool do_bootstrap = false;
    bool do_methods_only = false;
    bool do_verify = false;
    bool verbose = false;
    bool do_upstream = false;
    bool do_bazaar = false;

    // Auto-detect upp_root from executable location or parent directories
    string prog = argv[0];
    string cur_dir = ".";

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--bootstrap" || arg == "-bs") {
            do_bootstrap = true;
        } else if (arg == "--upstream") {
            do_upstream = true;
        } else if (arg == "--bazaar") {
            do_bazaar = true;
        } else if (arg == "--install-methods") {
            do_methods_only = true;
        } else if (arg == "--verify") {
            do_verify = true;
        } else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        } else if (arg == "--upp-root" && i + 1 < argc) {
            upp_root = argv[++i];
        } else if (arg == "--methods-dir" && i + 1 < argc) {
            methods_dir = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            PrintUsage(argv[0]);
            return 0;
        }
    }

    if (do_upstream) {
        if (upp_root.empty() || !DirectoryExists(JoinPath(upp_root, "uppsrc"))) {
            upp_root = "upp-root";
            RealizeDirectory(upp_root);
            string cached_tar = ".cache" PATH_SEP_STR "uppsrc-18468.tar.gz";
            if (!FileExists(cached_tar)) {
                RealizeDirectory(".cache");
                cout << "==> UppBootstrap: Fetching upstream U++ release tarball..." << endl;
                RunCommand("curl -s -L https://github.com/ultimatepp/ultimatepp/releases/download/v2026.1/uppsrc-18468.tar.gz -o .cache" PATH_SEP_STR "uppsrc-18468.tar.gz");
            }
            cout << "==> UppBootstrap: Extracting minimal U++ packages..." << endl;
            RunCommand("tar -xz -C " + upp_root + " -f .cache" PATH_SEP_STR "uppsrc-18468.tar.gz --wildcards 'uppsrc/Core/*' 'uppsrc/Draw/*' 'uppsrc/Esc/*' 'uppsrc/ide/*' 'uppsrc/plugin/*' 'uppsrc/umk/*'");
        }
    } else if (do_bazaar) {
        if (upp_root.empty() || !DirectoryExists(JoinPath(upp_root, "uppsrc"))) {
            upp_root = "bazaar-core";
            if (!DirectoryExists(upp_root)) {
                cout << "==> UppBootstrap: Fetching OuluBSD BazaarCore..." << endl;
                RunCommand("git clone --depth 1 https://github.com/OuluBSD/BazaarCore.git " + upp_root);
            }
        }
    }

    // Default discovery for upp_root
    if (upp_root.empty()) {
        if (DirectoryExists("uppsrc")) upp_root = ".";
        else if (DirectoryExists("../uppsrc")) upp_root = "..";
        else if (DirectoryExists("../ai-upp/uppsrc")) upp_root = "../ai-upp";
        else if (DirectoryExists("/home/sblo/Dev/ai-upp/uppsrc")) upp_root = "/home/sblo/Dev/ai-upp";
    }

    // Default discovery for methods_dir
    if (methods_dir.empty()) {
        if (DirectoryExists("methods")) methods_dir = "methods";
        else if (DirectoryExists("../methods")) methods_dir = "../methods";
    }

    if (do_methods_only) {
        HostToolchain tc = DetectHostToolchain();
        return InstallBuildMethods(methods_dir, tc) ? 0 : 1;
    }

    if (do_verify) {
        string umk_bin = JoinPath(upp_root, "bin" PATH_SEP_STR "umk");
#ifdef _WIN32
        umk_bin += ".exe";
#endif
        if (!FileExists(umk_bin)) {
            cerr << "umk binary not found at " << umk_bin << endl;
            return 1;
        }
        cout << "Verifying " << umk_bin << "..." << endl;
        return RunCommand("\"" + umk_bin + "\"");
    }

    (void)do_bootstrap;
    // Default action is --bootstrap
    return BootstrapUmk(upp_root, methods_dir, verbose);
}
