#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <filesystem> 
#include <fstream>
#include <memory>


std::vector<uint8_t> generated_salt_and_IV(int length){
    std::vector<uint8_t> random(length);
    if(!RAND_bytes(random.data(), length)){
        std::cerr << "Error: Failed to generate cryptographically secure random bytes. "
                  << "OpenSSL RAND_bytes() failed." << std::endl;
        return {};
    }
    return random;
}

std::vector<uint8_t> key_gene(const std::string& password, const std::vector<uint8_t>& salt, int length, int iterations, int keysize){
    std::vector<uint8_t> key(keysize);
    if(!PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), length, iterations, EVP_sha256(), keysize, key.data())){
        std::cerr << "Error: PBKDF2 key derivation failed. "
                  << "Password: " << password.length() << " chars, "
                  << "Iterations: " << iterations << ", "
                  << "Key size: " << keysize << " bytes" << std::endl;
        return {};
    }
    return key;   
}

std::vector<uint8_t> encryption_aes_256(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& IV){
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Error: Failed to create encryption context." << std::endl;
        return {};
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), IV.data()) != 1) {
        std::cerr << "Error: Encryption initialization failed." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    std::vector<uint8_t> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);

    int len;
    int ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
        std::cerr << "Error: Encryption update failed." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + ciphertext_len, &len) != 1) {
        std::cerr << "Error: Encryption finalization failed." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;


}

std::vector<uint8_t> read_a_file(const std::string& file_path){
    std::ifstream file(file_path, std::ios::binary);
    if(!file){
        std::cerr << "Error: file not found";
        return {};
    }
    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> read(file_size);


    file.read(reinterpret_cast<char* >(read.data()),file_size);

       return read;
}

void create_new_file(const std::string& path_file, std::vector<uint8_t> data) {
    std::string new_path = path_file;
    int count = 1;

    while (std::filesystem::exists(new_path)) {
        new_path = path_file + "_" + std::to_string(count);
        count++;
    }

    std::ofstream file(new_path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to create file: " + new_path);
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}




std::vector<uint8_t> decrypt_aes_256(const std::vector<uint8_t>& encrypted_data, const std::string& password, int iterations) {
    if (encrypted_data.size() < 32) {
        std::cerr << "Error: Encrypted data is too short." << std::endl;
        return {};
    }

    std::vector<uint8_t> salt(encrypted_data.begin(), encrypted_data.begin() + 16);
    std::vector<uint8_t> iv(encrypted_data.begin() + 16, encrypted_data.begin() + 32);
    std::vector<uint8_t> ciphertext(encrypted_data.begin() + 32, encrypted_data.end());

    std::vector<uint8_t> key(32);
    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), iterations, EVP_sha256(), key.size(), key.data())) {
        std::cerr << "Error: Key derivation failed." << std::endl;
        return {};
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Error: Failed to create context." << std::endl;
        return {};
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        std::cerr << "Error: Decryption initialization failed." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    std::vector<uint8_t> plaintext(ciphertext.size() + EVP_MAX_BLOCK_LENGTH);
    int len;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
        std::cerr << "Error: Decryption failed." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintext_len, &len) != 1) {
        std::cerr << "Error: Final decryption step failed." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(plaintext_len);

    // Convert plaintext to string for delimiter search
    std::string decrypted_text(plaintext.begin(), plaintext.end());
    std::string delimiter = "::END::";

    std::size_t pos = decrypted_text.find(delimiter);
    if (pos == std::string::npos) {
        std::cerr << "Error: Decryption verification failed." << std::endl;
        return {};
    }

    // Remove the delimiter and return the original plaintext
    decrypted_text = decrypted_text.substr(0, pos);
    return std::vector<uint8_t>(decrypted_text.begin(), decrypted_text.end());
}


std::vector<uint8_t> final_encrypt(const std::string& password, int iterations, int keysize, const std::string& input) {
    std::vector<uint8_t> salt = generated_salt_and_IV(16);
    if (salt.empty()) {
        std::cerr << "Error: Failed to generate salt" << std::endl;
        return {};
    }
    
    std::vector<uint8_t> derived_key = key_gene(password, salt, salt.size(), iterations, keysize);
    if (derived_key.empty()) {
        std::cerr << "Error: Failed to derive key" << std::endl;
        return {};
    }

    std::vector<uint8_t> IV = generated_salt_and_IV(16);
    if (IV.empty()) {
        std::cerr << "Error: Failed to generate IV" << std::endl;
        return {};
    }
    
    std::vector<uint8_t> plaintext = read_a_file(input);
    if (plaintext.empty()) {
        std::cerr << "Error: Failed to read input file or file is empty" << std::endl;
        return {};
    }

    // Append a known delimiter and string to the plaintext
    std::string delimiter = "::END::";
    plaintext.insert(plaintext.end(), delimiter.begin(), delimiter.end());

    std::vector<uint8_t> ciphertext = encryption_aes_256(plaintext, derived_key, IV);
    if (ciphertext.empty()) {
        std::cerr << "Error: Encryption failed" << std::endl;
        return {};
    }

    std::vector<uint8_t> final_output;
    final_output.insert(final_output.end(), salt.begin(), salt.end());
    final_output.insert(final_output.end(), IV.begin(), IV.end());
    final_output.insert(final_output.end(), ciphertext.begin(), ciphertext.end());

    return final_output;
}

// Add secure string clearing function
inline void secure_clear(std::string& str) {
    std::fill(str.begin(), str.end(), '\0');
    str.clear();
}

inline void secure_clear(std::vector<uint8_t>& vec) {
    std::fill(vec.begin(), vec.end(), 0);
    vec.clear();
}

// Create RAII wrapper for OpenSSL context
class EVPContext {
private:
    EVP_CIPHER_CTX* ctx_;
public:
    EVPContext() : ctx_(EVP_CIPHER_CTX_new()) {}
    ~EVPContext() { if (ctx_) EVP_CIPHER_CTX_free(ctx_); }
    EVP_CIPHER_CTX* get() { return ctx_; }
    bool valid() const { return ctx_ != nullptr; }
};

#endif // ENCRYPTION_H
