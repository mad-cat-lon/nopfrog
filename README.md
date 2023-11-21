# jmpman: simple Linux rootkit
jmpman is a simple userland rootkit for x86_64 Linux that uses the method implemented by [zpoline](https://github.com/yasukata/zpoline) to hook syscalls.
WORK IN PROGRESS!

## Installation 
```git clone --recursive https://github.com/mad-cat-lon/jmpman```

## Running 
Set `/proc/sys/vm/mmap_min_addr` to 0

```sudo sh -c "echo 0 > /proc/sys/vm/mmap_min_addr"```

```LIBZPHOOK=./jmpman.so LD_PRELOAD=./libzpoline.so [program you wish to run]```