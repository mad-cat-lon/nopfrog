# nopfrog: simple Linux userland rootkit
nopfrog is a simple userland rootkit for x86_64 Linux that uses the method implemented by [zpoline](https://github.com/yasukata/zpoline) to hook syscalls.
WORK IN PROGRESS!

## Installation 
```git clone --recursive https://github.com/mad-cat-lon/jmpman```

## Running 
Set `/proc/sys/vm/mmap_min_addr` to 0

```sudo sh -c "echo 0 > /proc/sys/vm/mmap_min_addr"```

```LIBZPHOOK=./jmpman.so LD_PRELOAD=./libzpoline.so [program you wish to run]```

## Configuration


## FAQ
#### This rootkit uses LD_PRELOAD. Won't static binaries be unaffected by the hooking?
