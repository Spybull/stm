# Simple Trash manager

# Deps
## APPLE
```shell
brew install openssl@3 zstd argp-standalone jansson sqlcipher libssh libcsv
```

# Install
```shell
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```