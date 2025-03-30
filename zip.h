#include <openssl/evp.h>
#include <openssl/rand.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <filesystem> 
#include <fstream>
#include <zip.h>


using namespace std; 
namespace fs = std::filesystem;


int is_file_or_folder(const string& path){
    if(fs::is_regular_file(path)){
        return 1;
    }
    else if(fs::is_directory(path)){
        return 0;
    }
    else{
        cerr<<"file is neigher a file nor a folder";
        return -1;
    }
}

void delete_zip(const string& path) {
    try {
        if (fs::exists(path)) {
            // Recursively remove if it's a folder, or just remove if it's a file
            size_t removed_count = fs::remove_all(path);
            if (removed_count == 0) {
                cerr << "zip was not deleted\n";
            }
        } else {
            cerr << "File or directory does not exist: " << path << "\n";
        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Error deleting file/folder: " << e.what() << "\n";
    }
}




bool zip_file(const std::string& input_file, const std::string& zip_path) {
    int error = 0;

    // Open the ZIP archive (create if not exists)
    zip_t* zip = zip_open(zip_path.c_str(), ZIP_CREATE, &error);
    if (!zip) {
        std::cerr << "Error: Could not create or open ZIP archive.\n";
        return false;
    }

    // Create a zip source from the input file
    zip_source_t* source = zip_source_file(zip, input_file.c_str(), 0, 0);
    if (!source) {
        std::cerr << "Error: Could not read input file.\n";
        zip_close(zip);
        return false;
    }

    // Add file to ZIP archive
    if (zip_file_add(zip, input_file.c_str(), source, ZIP_FL_OVERWRITE) < 0) {
        std::cerr << "Error: Could not add file to ZIP archive.\n";
        zip_source_free(source);
        zip_close(zip);
        return false;
    }

    // Close the ZIP archive
    if (zip_close(zip) != 0) {
        std::cerr << "Error: Could not close ZIP archive.\n";
        return false;
    }

    std::cout << "File successfully zipped into: " << zip_path << std::endl;
    return true;
}




bool zip_folder(const std::string& folder_path, const std::string& zip_path) {
    int error = 0;

    // Open or create a ZIP archive
    zip_t* zip = zip_open(zip_path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!zip) {
        std::cerr << "Error: Could not create ZIP archive.\n";
        return false;
    }

    // Iterate through all files and subdirectories recursively
    for (const auto& entry : fs::recursive_directory_iterator(folder_path)) {
        if (entry.is_regular_file()) {
            std::string file_path = entry.path().string();
            
            // Get relative path inside the ZIP archive
            std::string relative_path = fs::relative(entry.path(), folder_path).string();

            // Create a zip source from the file
            zip_source_t* source = zip_source_file(zip, file_path.c_str(), 0, 0);
            if (!source) {
                std::cerr << "Error: Could not read file: " << file_path << "\n";
                continue;
            }

            // Add file to the ZIP archive with its relative path
            if (zip_file_add(zip, relative_path.c_str(), source, ZIP_FL_OVERWRITE) < 0) {
                std::cerr << "Error: Could not add file to ZIP archive: " << file_path << "\n";
                zip_source_free(source);
                continue;
            }
        }
    }

    // Close the ZIP archive
    if (zip_close(zip) != 0) {
        std::cerr << "Error: Could not close ZIP archive.\n";
        return false;
    }

    std::cout << "Folder successfully zipped: " << zip_path << std::endl;
    return true;
}



// the first one
bool unzip_file(const std::string& zip_path, const std::string& output_folder) {
    int error = 0;

    // Open the ZIP archive for reading
    zip_t* zip = zip_open(zip_path.c_str(), ZIP_RDONLY, &error);
    if (!zip) {
        std::cerr << "Error: Could not open ZIP archive.\n";
        return false;
    }

    // Get the number of entries (files/folders) in the ZIP
    zip_int64_t num_entries = zip_get_num_entries(zip, 0);
    if (num_entries < 0) {
        std::cerr << "Error: Could not read ZIP archive entries.\n";
        zip_close(zip);
        return false;
    }

    // Ensure the output folder exists
    fs::create_directories(output_folder);

    // Iterate over each entry in the ZIP file
    for (zip_int64_t i = 0; i < num_entries; i++) {
        struct zip_stat file_stat;
        if (zip_stat_index(zip, i, 0, &file_stat) != 0) {
            std::cerr << "Error: Could not retrieve file info.\n";
            continue;
        }
// -------- just trying
        std::string filename = file_stat.name;
        std::string output_path = output_folder + "/" + filename;

        // If it's a directory, create it
        if (!filename.empty() && filename.back() == '/') {
            fs::create_directories(output_path);
            continue;
        }
// -------- just trying

// std::string filename = file_stat.name;
// fs::path output_path = fs::path(output_folder) / filename;

// // Ensure parent directories exist
// fs::create_directories(output_path.parent_path());

// // If it's a directory, just create it
// if (filename.back() == '/') {
//     fs::create_directories(output_path);
//     continue;
// }


        // Open the file inside the ZIP
        zip_file_t* file = zip_fopen_index(zip, i, 0);
        if (!file) {
            std::cerr << "Error: Could not open file inside ZIP: " << filename << "\n";
            continue;
        }

        // Ensure the parent directory exists
        fs::create_directories(fs::path(output_path).parent_path());

        // Extract the file
        std::ofstream out_file(output_path, std::ios::binary);
        if (!out_file) {
            std::cerr << "Error: Could not create output file: " << output_path << "\n";
            zip_fclose(file);
            continue;
        }

        char buffer[4096];
        zip_int64_t bytes_read;
        while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
            out_file.write(buffer, bytes_read);
        }

        // Close file streams
        zip_fclose(file);
        out_file.close();
    }

    // Close the ZIP archive
    zip_close(zip);

    std::cout << "Extraction complete: " << output_folder << std::endl;
    return true;
}