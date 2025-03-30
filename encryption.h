#include <openssl/evp.h>
#include <openssl/rand.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <filesystem> 
#include <fstream>


using namespace std;
vector<uint8_t> generated_salt_and_IV(int length){
    vector<uint8_t> random(length);
    if(!RAND_bytes(random.data(),length)){
        cerr << "Error!!!!!!!!!";
        return {};
    }
    return random;
}

vector<uint8_t> key_gene(const string& password,const vector<uint8_t>& salt,int length,int iterations,int keysize){
    vector<uint8_t> key(keysize);
     if(!PKCS5_PBKDF2_HMAC(password.c_str(),password.length(),salt.data(),length,iterations,EVP_sha256(),keysize,key.data())){
        cerr << "Error";
        return {};
     }
    return key;   
}

vector <uint8_t> encryption_aes_256(const vector<uint8_t>& plaintext,const vector<uint8_t>& key,const vector<uint8_t>& IV){
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    EVP_EncryptInit_ex(ctx,EVP_aes_256_cbc(),nullptr,key.data(),IV.data());

    vector<uint8_t> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);

    int len;
    int ciphertext_len = 0;

    EVP_EncryptUpdate(ctx,ciphertext.data(),&len,plaintext.data(),plaintext.size());

    ciphertext_len = len;


    EVP_EncryptFinal_ex(ctx,ciphertext.data() + ciphertext_len,&len);

    ciphertext_len+=len;

    ciphertext.resize(ciphertext_len);

    EVP_CIPHER_CTX_free(ctx);

    return ciphertext;


}

vector<uint8_t > read_a_file(const string& file_path){
ifstream file(file_path,ios::binary);
if(!file){
    cerr << "Error: file not found";
    return {};
}
file.seekg(0,ios::end);
streamsize file_size = file.tellg();
file.seekg(0,ios::beg);

vector <uint8_t> read(file_size);


file.read(reinterpret_cast<char* >(read.data()),file_size);

   return read;
}

void create_new_file(const string& path_file, vector<uint8_t> data) {
    string new_path = path_file;
    int count = 1;

    // Check if file exists and create a unique filename
    while (fs::exists(new_path)) {
        new_path = path_file + "_" + to_string(count);
        count++;
    }

    // Open a new file for writing
    ofstream file(new_path, ios::binary);
    if (!file) {
        throw runtime_error("Failed to create file: " + new_path);
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}




vector<uint8_t> decrypt_aes_256(const vector<uint8_t>& encrypted_data, const string& password, int iterations) {
    if (encrypted_data.size() < 32) {
        cerr << "Error: Encrypted data is too short." << endl;
        return {};
    }

    vector<uint8_t> salt(encrypted_data.begin(), encrypted_data.begin() + 16);
    vector<uint8_t> iv(encrypted_data.begin() + 16, encrypted_data.begin() + 32);
    vector<uint8_t> ciphertext(encrypted_data.begin() + 32, encrypted_data.end());

    vector<uint8_t> key(32);
    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), iterations, EVP_sha256(), key.size(), key.data())) {
        cerr << "Error: Key derivation failed." << endl;
        return {};
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        cerr << "Error: Failed to create context." << endl;
        return {};
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
        cerr << "Error: Decryption initialization failed." << endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    vector<uint8_t> plaintext(ciphertext.size() + EVP_MAX_BLOCK_LENGTH);
    int len;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size()) != 1) {
        cerr << "Error: Decryption failed." << endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintext_len, &len) != 1) {
        cerr << "Error: Final decryption step failed." << endl;
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(plaintext_len);

    // Convert plaintext to string for delimiter search
    string decrypted_text(plaintext.begin(), plaintext.end());
    string delimiter = "::END::";

    size_t pos = decrypted_text.find(delimiter);
    if (pos == string::npos) {
        cerr << "Error: Decryption verification failed." << endl;
        return {};
    }

    // Remove the delimiter and return the original plaintext
    decrypted_text = decrypted_text.substr(0, pos);
    return vector<uint8_t>(decrypted_text.begin(), decrypted_text.end());
}


vector<uint8_t> final_encrypt(const string& password, int iterations, int keysize,const string& input) {
    vector<uint8_t> salt = generated_salt_and_IV(16);
    vector<uint8_t> derived_key = key_gene(password, salt, salt.size(), iterations, keysize);

    vector<uint8_t> IV = generated_salt_and_IV(16);
    vector<uint8_t> plaintext = read_a_file(input);

    // Append a known delimiter and string to the plaintext
    string delimiter = "::END::";
    plaintext.insert(plaintext.end(), delimiter.begin(), delimiter.end());

    vector<uint8_t> ciphertext = encryption_aes_256(plaintext, derived_key, IV);

    vector<uint8_t> final_output;
    final_output.insert(final_output.end(), salt.begin(), salt.end());
    final_output.insert(final_output.end(), IV.begin(), IV.end());
    final_output.insert(final_output.end(), ciphertext.begin(), ciphertext.end());

    return final_output;
}
