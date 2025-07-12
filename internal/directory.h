#ifndef dIRECTORY_H
#define dIRECTORY_H


#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// Function declarations
std::string find_deepest_unnecessary_root(const std::string& extract_dir);
void fix_extracted_directory(const std::string& extract_dir);
void remove_empty_folders(const std::string& dir);

// Inline implementations (small functions only)
inline void remove_empty_folders(const std::string& dir) {
    std::string current_dir = dir;
    while (current_dir != "." && current_dir != "/") {
        if (fs::is_empty(current_dir)) {
            fs::remove(current_dir);
            current_dir = fs::path(current_dir).parent_path().string();
        } else {
            break;
        }
    }
}

std::string find_deepest_unnecessary_root(const std::string& extract_dir) {
    std::string current_dir = extract_dir;
    while (true) {
        std::vector<std::string> top_level_entries;
        std::string single_folder;

        try {
            for (const auto& entry : fs::directory_iterator(current_dir)) {
                if (entry.is_directory()) {
                    if (!single_folder.empty()) return current_dir;
                    single_folder = entry.path().string();
                }
                top_level_entries.push_back(entry.path().string());
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error accessing directory: " << e.what() << std::endl;
            return current_dir;
        }

        if (!single_folder.empty() && top_level_entries.size() == 1) {
            current_dir = single_folder;
        } else {
            return current_dir;
        }
    }
}

void fix_extracted_directory(const std::string& extract_dir) {
    std::string nested_root = find_deepest_unnecessary_root(extract_dir);
    if (nested_root == extract_dir) {
        std::cout << "No unnecessary nesting detected.\n";
        return;
    }

    std::cout << "Detected unnecessary nesting in: " << nested_root << "\n";

    try {
        for (const auto& entry : fs::directory_iterator(nested_root)) {
            std::string new_path = extract_dir + "/" + entry.path().filename().string();
            fs::rename(entry.path(), new_path);
        }
        remove_empty_folders(nested_root);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error fixing directory structure: " << e.what() << std::endl;
    }
}

#endif // UNNECESSARY_DIRECTORY_H