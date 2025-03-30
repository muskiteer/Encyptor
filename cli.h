#include <iostream>
#include <string>
#include <vector>

using namespace std;
bool a1 = false,a2 = false,a3=false,a4 = false;

int cli(int argc,char* argv[],string& input,string& output,string& password,string& mode){
    if(string(argv[1]) == "-h" && argc ==2){
        std::cout << "Usage: myprogram -i <input_file> -o <output_file> -p <password> (-e | -d)\n\n"
              << "Options:\n"
              << "  -i <input_file>    Specify the input file.\n"
              << "  -o <output_file>   Specify the output file.\n"
              << "  -p <password>      Provide the password for encryption/decryption.\n"
              << "  -e                 Encrypt the input file.\n"
              << "  -d                 Decrypt the input file.\n"
              << "  -h                 Show this help message and exit.\n\n"
              << "Notes:\n"
              << "  - All parameters (-i, -o, and -p) are mandatory.\n"
              << "  - You must specify either -e (encryption) or -d (decryption), but not both.\n"
              << "  - Output should have valid path and then a name of file without any extension\n"
              << "  - if decrypted the output will be in folder format always even if only a file was encrypted\n";
              return -1;
    }
    else if(argc!=8){
        cerr<< "Incorrent parameter.\n" 
        << "Refer to help page with -h flag";
        return -1;
    }
    else{
        for(int i = 1;i<argc;i++){
            if(string(argv[i]) == "-p"){
                if(string(argv[i+1])[0] != '-' && i+1 < argc){
                    password = argv[i+1];
                    a1 = true;

                }
            }
            else if(string(argv[i]) == "-e"){
                mode = "enc";
                a2= true;
            }
            else if(string(argv[i]) == "-d"){
                mode = "dec";
                a2 = true;
            }
            else if(string(argv[i]) == "-i"){
                if(string(argv[i+1])[0] != '-' && i+1 < argc){
                    input = argv[i+1];
                    a3 = true;
                }
            }
            else if(string(argv[i]) == "-o"){
                if(string(argv[i+1])[0] != '-' && i+1 < argc){
                    output = argv[i+1];
                    a4 = true;
                }
            }
            
        }
    }

    if(a1 && a2 && a3 && a4){
        return 0;
    }
    else{
        cerr<< "Incorrent parameter.\n" 
        << "Refer to help page with -h flag";
        return -1;
    }
}