#ifndef ZIP_H
#define ZIP_H

#include <vector>
#include <iostream>
#include <filesystem> 
#include <fstream>
#include <zip.h>

namespace fs = std::filesystem;

int is_file_or_folder(const std::string& path) {
    try {
        if (fs::is_regular_file(path)) {
            return 1;  // File
        } else if (fs::is_directory(path)) {
            return 0;  // Directory
        } else {
            std::cerr << "Error: Path is neither a file nor a folder: " << path << std::endl;
            return -1;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error checking path: " << e.what() << std::endl;
        return -1;
    }
}

void delete_zip(const std::string& path) {
    try {
        if (fs::exists(path)) {
            std::size_t removed_count = fs::remove_all(path);
            if (removed_count == 0) {
                std::cerr << "Warning: No files were deleted at: " << path << std::endl;
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error deleting file/folder: " << e.what() << std::endl;
    }
}

// Path sanitization to prevent directory traversal attacks
bool is_safe_path(const std::string& path) {
    return path.find("..") == std::string::npos && 
           path.find("//") == std::string::npos &&
           !path.empty() && 
           (path[0] != '/' || path.find_first_not_of('/') != std::string::npos);
}

bool zip_file(const std::string& input_file, const std::string& zip_path) {
    int error = 0;

    zip_t* zip = zip_open(zip_path.c_str(), ZIP_CREATE, &error);
    if (!zip) {
        std::cerr << "Error: Could not create ZIP archive: " << zip_path << std::endl;
        return false;
    }

    // Use only the filename, not the full path, to avoid unnecessary directories
    std::string filename = fs::path(input_file).filename().string();
    
    zip_source_t* source = zip_source_file(zip, input_file.c_str(), 0, 0);
    if (!source) {
        std::cerr << "Error: Could not read input file: " << input_file << std::endl;
        zip_close(zip);
        return false;
    }

    // Add file with just the filename (no path)
    if (zip_file_add(zip, filename.c_str(), source, ZIP_FL_OVERWRITE) < 0) {
        std::cerr << "Error: Could not add file to ZIP archive." << std::endl;
        zip_source_free(source);
        zip_close(zip);
        return false;
    }

    if (zip_close(zip) != 0) {
        std::cerr << "Error: Could not close ZIP archive." << std::endl;
        return false;
    }

    std::cout << "File successfully zipped: " << zip_path << std::endl;
    return true;
}

bool zip_folder(const std::string& folder_path, const std::string& zip_path) {
    int error = 0;

    zip_t* zip = zip_open(zip_path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip) {
        std::cerr << "Error: Could not create ZIP archive: " << zip_path << std::endl;
        return false;
    }

    try {
        // Get the folder name to preserve directory structure
        std::string folder_name = fs::path(folder_path).filename().string();
        
        // First, add all directories (including empty ones)
        for (const auto& entry : fs::recursive_directory_iterator(folder_path)) {
            if (entry.is_directory()) {
                // Get relative path from the parent of folder_path
                fs::path relative_path = fs::relative(entry.path(), fs::path(folder_path).parent_path());
                std::string dir_path = relative_path.string() + "/";
                
                // Add directory entry
                if (zip_dir_add(zip, dir_path.c_str(), ZIP_FL_ENC_UTF_8) < 0) {
                    std::cerr << "Warning: Could not add directory: " << dir_path << std::endl;
                }
            }
        }
        
        // Then add all files
        for (const auto& entry : fs::recursive_directory_iterator(folder_path)) {
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                
                // Get relative path from the parent of folder_path to preserve structure
                fs::path relative_path = fs::relative(entry.path(), fs::path(folder_path).parent_path());
                std::string archive_path = relative_path.string();

                zip_source_t* source = zip_source_file(zip, file_path.c_str(), 0, 0);
                if (!source) {
                    std::cerr << "Warning: Could not read file: " << file_path << std::endl;
                    continue;
                }

                if (zip_file_add(zip, archive_path.c_str(), source, ZIP_FL_OVERWRITE) < 0) {
                    std::cerr << "Warning: Could not add file to ZIP: " << file_path << std::endl;
                    zip_source_free(source);
                    continue;
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error iterating directory: " << e.what() << std::endl;
        zip_close(zip);
        return false;
    }

    if (zip_close(zip) != 0) {
        std::cerr << "Error: Could not close ZIP archive." << std::endl;
        return false;
    }

    std::cout << "Folder successfully zipped: " << zip_path << std::endl;
    return true;
}

bool unzip_file(const std::string& zip_path, const std::string& output_folder) {
    int error = 0;

    zip_t* zip = zip_open(zip_path.c_str(), ZIP_RDONLY, &error);
    if (!zip) {
        std::cerr << "Error: Could not open ZIP archive: " << zip_path << std::endl;
        return false;
    }

    zip_int64_t num_entries = zip_get_num_entries(zip, 0);
    if (num_entries < 0) {
        std::cerr << "Error: Could not read ZIP archive entries." << std::endl;
        zip_close(zip);
        return false;
    }

    // Create output directory
    try {
        fs::create_directories(output_folder);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: Could not create output directory: " << e.what() << std::endl;
        zip_close(zip);
        return false;
    }

    // Extract all entries
    for (zip_int64_t i = 0; i < num_entries; i++) {
        struct zip_stat file_stat;
        if (zip_stat_index(zip, i, 0, &file_stat) != 0) {
            std::cerr << "Warning: Could not retrieve info for entry " << i << std::endl;
            continue;
        }

        std::string filename = file_stat.name;
        
        // Security check - prevent directory traversal
        if (!is_safe_path(filename)) {
            std::cerr << "Warning: Skipping unsafe path: " << filename << std::endl;
            continue;
        }

        // Use proper path handling
        fs::path output_path = fs::path(output_folder) / filename;

        // Handle directories
        if (!filename.empty() && filename.back() == '/') {
            try {
                fs::create_directories(output_path);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Warning: Could not create directory: " << e.what() << std::endl;
            }
            continue;
        }

        // Handle files
        zip_file_t* file = zip_fopen_index(zip, i, 0);
        if (!file) {
            std::cerr << "Warning: Could not open file in ZIP: " << filename << std::endl;
            continue;
        }

        // Ensure parent directories exist
        try {
            fs::create_directories(output_path.parent_path());
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Warning: Could not create parent directories: " << e.what() << std::endl;
            zip_fclose(file);
            continue;
        }

        // Extract the file
        std::ofstream out_file(output_path, std::ios::binary);
        if (!out_file) {
            std::cerr << "Warning: Could not create output file: " << output_path << std::endl;
            zip_fclose(file);
            continue;
        }

        // Read and write file data
        char buffer[8192];  // Increased buffer size for better performance
        zip_int64_t bytes_read;
        while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            out_file.write(buffer, bytes_read);
            if (!out_file) {
                std::cerr << "Error: Failed to write to output file: " << output_path << std::endl;
                break;
            }
        }

        zip_fclose(file);
        out_file.close();

        if (bytes_read < 0) {
            std::cerr << "Warning: Error reading file from ZIP: " << filename << std::endl;
        }
    }

    zip_close(zip);
    std::cout << "Extraction completed: " << output_folder << std::endl;
    return true;
}

#endif // ZIP_H