# 🔐 Encryptor (C++)

A simple command-line tool to **encrypt/decrypt and zip/unzip** files or folders using OpenSSL and libzip.

## 📦 Build

Make sure `libssl` and `libzip` are installed.

**Linux:**

```bash
sudo apt install libssl-dev libzip-dev g++

To Compile-
g++ main.cpp -o main -lssl -lcrypto -lzip

For Usage-
./main -h
