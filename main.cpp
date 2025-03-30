#include "temporarily.h"
#include "encryption.h"
#include "cli.h"
#include "temp.h"


int main(int argc ,char* argv[]){
    int length= 32;
    int iterations = 1000;
    string password , mode , input,output;

    if(cli(argc,argv,input,output,password,mode)==-1){
        return -1;
    }

if(mode == "enc"){
    output = output + fs::path(input).extension().string() + ".enc";
    int file_or_folder = is_file_or_folder(input);

    if(file_or_folder == 1){
        if(!zip_file(input , input + ".zip")){
            cerr << "failed to zip";
        }
    }
    else if(file_or_folder == 0){
        if(!zip_folder(input , input + ".zip")){
            cerr << "failed to zip";
        }
    }

    
    vector<uint8_t> encypted_data = final_encrypt(password,iterations,length,input+".zip");

    create_new_file(output, encypted_data);
    delete_zip(input);
    delete_zip(input + ".zip");

}
else if(mode == "dec"){
    std::string filename = fs::path(input).filename().string(); // "example.txt.enc"

    std::string base_name = fs::path(input).stem().string(); // "example.txt"

    vector<uint8_t>  to_be_decrypted = read_a_file(input);

    vector<uint8_t> decrypted = decrypt_aes_256(to_be_decrypted,password,iterations);

    create_new_file(input + ".unzipped", decrypted);

    unzip_file(input + ".unzipped",output);

    // delete_zip(input);
    delete_zip(input + ".unzipped");

    fix_extracted_directory(output);

}

}