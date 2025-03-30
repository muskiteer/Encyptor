#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
 



std::string find_deepest_unnecessary_root(const std::string& extract_dir) {
    std::string current_dir = extract_dir;
    while (true) {
        std::vector<std::string> top_level_entries;
        std::string single_folder;

        // Check all entries in the current directory
        for (const auto& entry : fs::directory_iterator(current_dir)) {
            if (entry.is_directory()) {
                if (!single_folder.empty()) return current_dir; // More than one folder, stop
                single_folder = entry.path().string();
            }
            top_level_entries.push_back(entry.path().string());
        }

        // If there's only one folder and no top-level files, keep going deeper
        if (!single_folder.empty() && top_level_entries.size() == 1) {
            current_dir = single_folder;
        } else {
            return current_dir; // Stop when multiple files/folders exist
        }
    }
}

// Function to remove empty folders from bottom up
void remove_empty_folders(std::string dir) {
    while (dir != "." && dir != "/") {
        if (fs::is_empty(dir)) {
            fs::remove(dir);  // Remove empty folder
            dir = fs::path(dir).parent_path().string();  // Move up one level
        } else {
            break;  // Stop if folder is not empty
        }
    }
}

// Function to fix extracted structure if unnecessary nesting exists
void fix_extracted_directory(const std::string& extract_dir) {
    std::string nested_root = find_deepest_unnecessary_root(extract_dir);
    if (nested_root == extract_dir) {
        std::cout << "No unnecessary nesting detected.\n";
        return;
    }

    std::cout << "Detected unnecessary nesting in: " << nested_root << "\n";

    // Move all contents from nested_root to extract_dir
    for (const auto& entry : fs::directory_iterator(nested_root)) {
        fs::rename(entry.path(), extract_dir + "/" + entry.path().filename().string());
    }

    // Remove empty nested folders
    remove_empty_folders(nested_root);
    
    std::cout << "Fixed extracted structure.\n";
}