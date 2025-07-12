# 🔐 File Encryptor

A secure, cross-platform file and folder encryption tool built with C++ that uses military-grade AES-256-CBC encryption with PBKDF2 key derivation.

## ✨ Features

- **🛡️ Military-Grade Security**: AES-256-CBC encryption with PBKDF2 key derivation (100,000 iterations)
- **📁 File & Folder Support**: Encrypt single files or entire directory structures
- **🖥️ Interactive CLI**: User-friendly interface with tab autocompletion for file paths
- **🏠 Home Directory Support**: Works with `~` paths and automatic expansion
- **🔒 Secure Password Input**: Hidden password entry with confirmation for encryption
- **📦 Smart Compression**: Automatic ZIP compression before encryption
- **🔄 Directory Preservation**: Maintains folder structure during decryption
- **⚡ Fast Performance**: Optimized for large files and directories
- **🐧 Linux Native**: Built specifically for Linux with bash-style tab completion

## 🚀 Quick Start

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake libssl-dev libzip-dev pkg-config

# CentOS/RHEL/Fedora
sudo dnf install gcc-c++ cmake openssl-devel libzip-devel pkgconfig

# Arch Linux
sudo pacman -S base-devel cmake openssl libzip
```

### Build & Install

```bash
git clone https://github.com/muskiteer/Encyptor
cd encryptor
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Usage

#### Interactive Mode (Recommended)
```bash
./build/bin/encryptor
```
- Follow the guided prompts
- Use **Tab** for path autocompletion
- Supports `~/` home directory paths

#### Command Line Mode
```bash
# Encrypt a file
./build/bin/encryptor -i document.txt -o ~/encrypted_output -p your_password -e

# Encrypt a folder
./build/bin/encryptor -i ~/my_folder -o ~/encrypted_output -p your_password -e

# Decrypt
./build/bin/encryptor -i file.txt.enc -o ~/decrypted_output -p your_password -d

# Show help
./build/bin/encryptor -h
```

## 📖 How It Works

### Encryption Process
1. **Input Processing**: Files/folders are compressed into ZIP format
2. **Key Derivation**: Password → PBKDF2 (100,000 iterations) → 256-bit key
3. **Random Generation**: Cryptographically secure salt and IV generation
4. **Encryption**: AES-256-CBC encryption with integrity delimiter
5. **Output**: Single `.enc` file containing: `[salt][IV][encrypted_data]`

### Decryption Process
1. **File Reading**: Extract salt, IV, and encrypted data
2. **Key Derivation**: Recreate encryption key using password and salt
3. **Decryption**: AES-256-CBC decryption with integrity verification
4. **Extraction**: Decompress ZIP and restore original structure

## 🔧 Technical Specifications

| Component | Technology |
|-----------|------------|
| **Encryption** | AES-256-CBC |
| **Key Derivation** | PBKDF2-HMAC-SHA256 |
| **Iterations** | 100,000 (configurable) |
| **Salt/IV Size** | 128-bit (16 bytes) |
| **Compression** | ZIP with directory preservation |
| **Libraries** | OpenSSL, libzip |
| **Language** | C++17 |

## 📁 Project Structure

```
encryptor/
├── CMakeLists.txt          # Build configuration
├── main.cpp                # Main application entry
├── cmd/
│   └── cli.h              # Interactive CLI with tab completion
├── internal/
│   ├── directory.h        # Directory structure cleanup utilities
│   ├── encryption.h       # AES encryption/decryption functions
│   └── zip.h              # ZIP compression utilities
├── test/                   # Test files and examples
│   ├── testing.txt        # Sample test file
│   └── *.enc              # Generated encrypted files
├── build/                  # Build output (gitignored)
│   └── bin/
│       └── encryptor      # Compiled binary
└── README.md              # This file
```

## 🛡️ Security Features

- **Strong Key Derivation**: 100,000 PBKDF2 iterations (configurable up to 600,000)
- **Cryptographically Secure Random**: Uses OpenSSL's RAND_bytes()
- **Memory Security**: Automatic password clearing after use
- **Path Validation**: Protection against directory traversal attacks
- **Integrity Verification**: Built-in tamper detection
- **No Key Storage**: Keys are derived fresh each time

## 📋 Command Reference

### Interactive Mode
```bash
./build/bin/encryptor
```
**Features:**
- Tab completion for file paths
- Home directory (`~`) support
- Hidden password input
- Password confirmation for encryption
- Real-time path validation

### Command Line Options
```bash
-i <path>    Input file or folder
-o <path>    Output directory
-p <pass>    Password for encryption/decryption
-e           Encrypt mode
-d           Decrypt mode
-h           Show help
```

## 🚨 Security Considerations

### ✅ Strong Points
- Industry-standard AES-256-CBC encryption
- High iteration PBKDF2 (100,000+ iterations)
- Cryptographically secure random generation
- Memory clearing after password use
- Path traversal protection

### ⚠️ Important Notes
- **Password Strength**: Use strong, unique passwords
- **Backup**: Keep secure backups of encrypted files
- **Updates**: Regularly update OpenSSL and dependencies
- **Environment**: Use on trusted systems only

## 🔧 Build Options

### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Release Build (Optimized)
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Custom Iterations
Modify `main.cpp` line 8:
```cpp
const int iterations = 600000;  // Increase for higher security
```

## 🐛 Troubleshooting

### Common Issues

**"libssl not found"**
```bash
sudo apt install libssl-dev  # Ubuntu/Debian
sudo dnf install openssl-devel  # CentOS/Fedora
```

**"libzip not found"**
```bash
sudo apt install libzip-dev  # Ubuntu/Debian
sudo dnf install libzip-devel  # CentOS/Fedora
```

**"Permission denied"**
```bash
chmod +x build/bin/encryptor
```

**Tab completion not working**
- Ensure you're in interactive mode (`build/bin/encryptor` without arguments)
- Check terminal supports ANSI escape sequences

## 📈 Performance

### Benchmarks (approximate)
- **Small files** (<1MB): Near-instantaneous
- **Medium files** (1-100MB): 1-10 seconds
- **Large files** (100MB-1GB): 10-60 seconds
- **Folders**: Depends on total size and file count

### Optimization Tips
- Use Release build for production
- SSD storage for better I/O performance
- Adjust PBKDF2 iterations based on security vs speed needs

## 🧪 Testing

The `test` directory contains sample files for testing:

```bash
# Test encryption
./build/bin/encryptor -i test/testing.txt -o /tmp/encrypted_test -p testpass123 -e

# Test decryption
./build/bin/encryptor -i /tmp/encrypted_test.txt.enc -o /tmp/decrypted_test -p testpass123 -d
```

## 🔍 Example Usage

### Encrypting a Single File
```bash
$ ./build/bin/encryptor
=== File Encryption Tool ===
Tab for autocompletion, Enter to confirm

Choose operation:
1. Encrypt (e)
2. Decrypt (d)
Enter choice (e/d): e
Input file/folder: ~/Documents/secret.txt
Output directory: ~/encrypted/
Enter password: ********
Confirm password: ********

Operation: Encrypt
Input: /home/user/Documents/secret.txt
Output: /home/user/encrypted/
Proceeding...

Encrypting...
File successfully zipped: /home/user/Documents/secret.txt.tmp.zip
Encryption completed successfully: /home/user/encrypted/secret.txt.enc
```

### Decrypting a File
```bash
$ ./build/bin/encryptor -i ~/encrypted/secret.txt.enc -o ~/decrypted/ -p mypassword -d
Decrypting...
Extraction completed: /home/user/decrypted/
No unnecessary nesting detected.
Decryption completed successfully: /home/user/decrypted/
```

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

### Development Guidelines
- Follow C++17 standards
- Add security tests for new features
- Update documentation for API changes
- Ensure cross-platform compatibility

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- **OpenSSL** - Cryptographic library
- **libzip** - ZIP compression library
- **CMake** - Build system
- **Linux Community** - Terminal handling inspiration

## 📞 Support

- 🐛 **Issues**: Create a GitHub issue
- 💬 **Questions**: Start a GitHub discussion

## 🔄 Version History

### v1.0.0
- Initial release
- AES-256-CBC encryption
- Interactive CLI with tab completion
- File and folder support
- ZIP compression
- Home directory support

---

**⚠️ Disclaimer**: This software is provided as-is. Always test thoroughly and maintain secure backups of important data.

**🔐 Security Note**: This tool is designed for personal and educational use. For enterprise or high-security environments, consider additional security audits and compliance requirements.