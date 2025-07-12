#ifndef CLI_H
#define CLI_H

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <cstdio>

// Helper functions (moved outside class so they can be used globally)
std::string expand_path(const std::string& path) {
    if (path.empty() || path[0] != '~') {
        return path;
    }
    
    const char* home = std::getenv("HOME");
    if (!home) {
        return path;
    }
    
    if (path.length() == 1 || path[1] == '/') {
        return std::string(home) + path.substr(1);
    }
    
    return path;
}

std::string validate_and_expand_path(const std::string& path, bool must_exist = true, bool force_directory = false) {
    std::string expanded = expand_path(path);
    
    if (must_exist && !std::filesystem::exists(expanded)) {
        return ""; // Invalid path
    }
    
    if (force_directory && std::filesystem::exists(expanded) && std::filesystem::is_regular_file(expanded)) {
        // Convert file path to directory path
        expanded = std::filesystem::path(expanded).parent_path().string();
        std::cout << "Note: Using parent directory: " << expanded << std::endl;
    }
    
    return expanded;
}

std::string get_password_input() {
    std::cout << "Enter password: ";
    std::cout.flush();
    
    std::string password;
    struct termios old_termios, new_termios;
    
    // Turn off echo
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios);
    
    std::getline(std::cin, password);
    
    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_termios);
    
    std::cout << std::endl;
    return password;
}

class InteractiveCLI {
private:
    struct termios old_termios;
    
    void enable_raw_mode() {
        tcgetattr(STDIN_FILENO, &old_termios);
        struct termios raw = old_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
    
    void disable_raw_mode() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_termios);
    }
    
    std::vector<std::string> get_completions(const std::string& partial_path) {
        std::vector<std::string> completions;
        std::string expanded_path = expand_path(partial_path);
        std::filesystem::path path(expanded_path);
        std::string dir_path, filename_prefix;
        
        if (expanded_path.empty() || expanded_path.back() == '/') {
            dir_path = expanded_path.empty() ? "." : expanded_path;
            filename_prefix = "";
        } else {
            dir_path = path.parent_path().string();
            if (dir_path.empty()) dir_path = ".";
            filename_prefix = path.filename().string();
        }
        
        try {
            for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                std::string name = entry.path().filename().string();
                
                // Skip hidden files unless explicitly requested
                if (name[0] == '.' && filename_prefix.empty()) continue;
                
                if (name.substr(0, filename_prefix.length()) == filename_prefix) {
                    std::string full_path;
                    if (dir_path == ".") {
                        full_path = name;
                    } else {
                        full_path = dir_path + "/" + name;
                    }
                    
                    if (entry.is_directory()) {
                        full_path += "/";
                    }
                    
                    // Convert back to tilde format if original used ~ (C++17 compatible)
                    if (!partial_path.empty() && partial_path[0] == '~') {
                        std::string home = std::getenv("HOME");
                        if (!home.empty() && full_path.length() >= home.length() && 
                            full_path.substr(0, home.length()) == home) {
                            full_path = "~" + full_path.substr(home.length());
                        }
                    }
                    
                    completions.push_back(full_path);
                }
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Directory doesn't exist or can't be accessed
        }
        
        std::sort(completions.begin(), completions.end());
        return completions;
    }
    
    std::string read_line_with_completion(const std::string& prompt) {
        std::cout << prompt;
        std::cout.flush();
        
        std::string input;
        int ch;
        
        enable_raw_mode();
        
        while (true) {
            ch = getchar();
            
            if (ch == '\n' || ch == '\r') {  // Enter
                std::cout << std::endl;
                break;
            } else if (ch == '\t') {  // Tab for autocompletion
                auto completions = get_completions(input);
                
                if (completions.size() == 1) {
                    // Single completion - auto-complete
                    std::cout << "\r" << std::string(prompt.length() + input.length(), ' ');
                    input = completions[0];
                    std::cout << "\r" << prompt << input;
                } else if (completions.size() > 1) {
                    // Multiple completions - show options
                    std::cout << std::endl;
                    std::cout << "Options:" << std::endl;
                    for (const auto& comp : completions) {
                        std::cout << "  " << comp << std::endl;
                    }
                    std::cout << prompt << input;
                }
                std::cout.flush();
            } else if (ch == 127 || ch == '\b') {  // Backspace
                if (!input.empty()) {
                    input.pop_back();
                    std::cout << "\b \b";
                    std::cout.flush();
                }
            } else if (ch == 27) {  // Escape sequence (arrow keys, etc.)
                getchar(); // Skip '['
                getchar(); // Skip the actual key
            } else if (ch >= 32 && ch <= 126) {  // Printable characters
                input += ch;
                std::cout << (char)ch;
                std::cout.flush();
            }
        }
        
        disable_raw_mode();
        return input;
    }
    
    std::string get_password() {
        return get_password_input(); // Use the global function
    }

public:
    int run_interactive_cli(std::string& input, std::string& output, std::string& password, std::string& mode) {
        std::cout << "=== File Encryption Tool ===" << std::endl;
        std::cout << "Tab for autocompletion, Enter to confirm" << std::endl << std::endl;
        
        // Get operation mode
        std::cout << "Choose operation:" << std::endl;
        std::cout << "1. Encrypt (e)" << std::endl;
        std::cout << "2. Decrypt (d)" << std::endl;
        std::cout << "Enter choice (e/d): ";
        
        char choice;
        std::cin >> choice;
        std::cin.ignore(); // Clear the newline
        
        if (choice == 'e' || choice == 'E') {
            mode = "enc";
        } else if (choice == 'd' || choice == 'D') {
            mode = "dec";
        } else {
            std::cerr << "Invalid choice. Please enter 'e' for encrypt or 'd' for decrypt." << std::endl;
            return -1;
        }
        
        // Get input file/folder
        input = read_line_with_completion("Input file/folder: ");
        if (input.empty()) {
            std::cerr << "Input file/folder is required." << std::endl;
            return -1;
        }
        
        // Validate and expand input path
        std::string validated_input = validate_and_expand_path(input, true, false);
        if (validated_input.empty()) {
            std::cerr << "Error: Input file/folder does not exist: " << input << std::endl;
            return -1;
        }
        input = validated_input;
        
        // Get output path
        output = read_line_with_completion("Output directory: ");
        if (output.empty()) {
            std::cerr << "Output directory is required." << std::endl;
            return -1;
        }
        
        // Validate and expand output path (force directory)
        output = validate_and_expand_path(output, false, true);
        
        // Get password
        password = get_password();
        if (password.empty()) {
            std::cerr << "Password is required." << std::endl;
            return -1;
        }
        
        // Confirm password for encryption
        if (mode == "enc") {
            std::cout << "Confirm password: ";
            std::string confirm_password = get_password();
            if (password != confirm_password) {
                std::cerr << "Passwords do not match!" << std::endl;
                return -1;
            }
        }
        
        std::cout << std::endl << "Operation: " << (mode == "enc" ? "Encrypt" : "Decrypt") << std::endl;
        std::cout << "Input: " << input << std::endl;
        std::cout << "Output: " << output << std::endl;
        std::cout << "Proceeding..." << std::endl << std::endl;
        
        return 0;
    }
};

// Main CLI function that can handle both interactive and command-line modes
int cli(int argc, char* argv[], std::string& input, std::string& output, std::string& password, std::string& mode) {
    // If no arguments or just the program name, run interactive mode
    if (argc == 1) {
        InteractiveCLI interactive;
        return interactive.run_interactive_cli(input, output, password, mode);
    }
    
    // Handle help flag
    if (argc == 2 && std::string(argv[1]) == "-h") {
        std::cout << "File Encryption Tool\n\n"
                  << "Usage:\n"
                  << "  " << argv[0] << "                          # Interactive mode\n"
                  << "  " << argv[0] << " -i <input> -o <output> -p <password> (-e | -d)  # Command line mode\n\n"
                  << "Interactive mode:\n"
                  << "  Run without arguments for guided setup with tab autocompletion\n\n"
                  << "Command line options:\n"
                  << "  -i <input>     Input file or folder\n"
                  << "  -o <output>    Output directory\n"
                  << "  -p <password>  Password for encryption/decryption\n"
                  << "  -e             Encrypt mode\n"
                  << "  -d             Decrypt mode\n"
                  << "  -h             Show this help\n\n"
                  << "Examples:\n"
                  << "  " << argv[0] << " -i document.txt -o ~/encrypted_output -p mypassword -e\n"
                  << "  " << argv[0] << " -i ~/encrypted_doc.txt.enc -o ~/decrypted_output -p mypassword -d\n";
        return -1;
    }
    
    // Command line mode - existing logic but improved
    if (argc != 8) {
        std::cerr << "Error: Incorrect number of parameters.\n"
                  << "Use '" << argv[0] << " -h' for help or run without arguments for interactive mode." << std::endl;
        return -1;
    }
    
    bool has_input = false, has_output = false, has_password = false, has_mode = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-i" && i + 1 < argc) {
            input = argv[++i];
            has_input = true;
        } else if (arg == "-o" && i + 1 < argc) {
            output = argv[++i];
            has_output = true;
        } else if (arg == "-p" && i + 1 < argc) {
            password = argv[++i];
            has_password = true;
        } else if (arg == "-e") {
            mode = "enc";
            has_mode = true;
        } else if (arg == "-d") {
            mode = "dec";
            has_mode = true;
        }
    }
    
    if (!has_input || !has_output || !has_password || !has_mode) {
        std::cerr << "Error: Missing required parameters.\n"
                  << "Use '" << argv[0] << " -h' for help." << std::endl;
        return -1;
    }
    
    // Expand and validate input exists
    std::string expanded_input = expand_path(input);
    if (!std::filesystem::exists(expanded_input)) {
        std::cerr << "Error: Input file/folder does not exist: " << input << std::endl;
        return -1;
    }
    input = expanded_input;
    
    // Expand and validate output path (force directory)
    output = validate_and_expand_path(output, false, true);
    
    return 0;
}

#endif // CLI_H