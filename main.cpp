#include "internal/directory.h"
#include "internal/encryption.h"
#include "cmd/cli.h"
#include "internal/zip.h"

int main(int argc, char* argv[]) {
    const int length = 32;
    const int iterations = 100000;
    std::string password, mode, input, output;

    // Get CLI parameters
    if (cli(argc, argv, input, output, password, mode) == -1) {
        return -1;
    }

    try {
        if (mode == "enc") {
            // Validate input exists
            if (!std::filesystem::exists(input)) {
                std::cerr << "Error: Input file/folder does not exist: " << input << std::endl;
                return -1;
            }

            // Create output filename
            output = output + std::filesystem::path(input).extension().string() + ".enc";
            
            // Check if output already exists
            if (std::filesystem::exists(output)) {
                std::cout << "Warning: Output file already exists: " << output << std::endl;
                std::cout << "Continue? (y/N): ";
                char confirm;
                std::cin >> confirm;
                if (confirm != 'y' && confirm != 'Y') {
                    std::cout << "Operation cancelled." << std::endl;
                    return 0;
                }
            }

            int file_or_folder = is_file_or_folder(input);
            if (file_or_folder == -1) {
                std::cerr << "Error: Invalid input type" << std::endl;
                return -1;
            }

            std::string temp_zip = input + ".tmp.zip";

            // Zip the input
            bool zip_success = false;
            if (file_or_folder == 1) {
                zip_success = zip_file(input, temp_zip);
            } else if (file_or_folder == 0) {
                zip_success = zip_folder(input, temp_zip);
            }

            if (!zip_success) {
                std::cerr << "Error: Failed to create zip file" << std::endl;
                return -1;
            }

            // Encrypt
            std::cout << "Encrypting..." << std::endl;
            std::vector<uint8_t> encrypted_data = final_encrypt(password, iterations, length, temp_zip);
            
            // Clear password from memory
            secure_clear(password);
            
            if (encrypted_data.empty()) {
                std::cerr << "Error: Encryption failed" << std::endl;
                delete_zip(temp_zip);
                return -1;
            }

            // Save encrypted file
            create_new_file(output, encrypted_data);
            delete_zip(temp_zip);
            
            std::cout << "Encryption completed successfully: " << output << std::endl;

        } else if (mode == "dec") {
            // Validate encrypted file exists
            if (!std::filesystem::exists(input)) {
                std::cerr << "Error: Encrypted file does not exist: " << input << std::endl;
                return -1;
            }

            std::cout << "Decrypting..." << std::endl;
            std::vector<uint8_t> to_be_decrypted = read_a_file(input);
            if (to_be_decrypted.empty()) {
                std::cerr << "Error: Failed to read encrypted file" << std::endl;
                return -1;
            }

            std::vector<uint8_t> decrypted = decrypt_aes_256(to_be_decrypted, password, iterations);
            
            // Clear password from memory
            secure_clear(password);
            
            if (decrypted.empty()) {
                std::cerr << "Error: Decryption failed - wrong password or corrupted file" << std::endl;
                return -1;
            }

            std::string temp_zip = input + ".tmp.unzipped";
            create_new_file(temp_zip, decrypted);
            
            // Extract
            if (!unzip_file(temp_zip, output)) {
                std::cerr << "Error: Failed to extract files" << std::endl;
                delete_zip(temp_zip);
                return -1;
            }

            delete_zip(temp_zip);
            fix_extracted_directory(output);
            
            std::cout << "Decryption completed successfully: " << output << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}