# Description
nopfrog is a simple LD_PRELOAD rootkti for x64 Linux that uses the binary rewriting method outlined in [zpoline](https://github.com/yasukata/zpoline) to exhaustively hook syscalls. Unlike most LD_PRELOAD rootkits that hook the glibc syscall wrappers and hope for the best, nopfrog can intercept and modify all raw syscalls from both statically and dynamically linked programs, without `ptrace`, changing the target's source, recompiling the kernel or inserting an LKM. 
 
**WORK IN PROGRESS!**

## Features 
- **File hiding**
Hooks `openat()` and `getdents64()` to hide files containing a magic string in their name (i.e `__pwned__a.txt`)
- **Process hiding**
    - Hooks `getdents64()` to hide processes with magic strings in their names from `ps`, `top`, etc.
    - Hooks `kill()` to prevent victim from terminating hidden procs
- **Stealth**
    - Filters rootkit shared objects from `ldd` output, `/proc/<pid>/maps` and `/proc/<pid>/smaps`
    - Backs up existing `/etc/ld.so.preload` file and serves fake one to user
- **Anti-RE**
    - XORs out (most) sensitive strings and cleans memory


## Upcoming
- Hiding files and processes by GID **(in progress)**
- Anti-VM 
- Easy setup and install scripts **(in progress)**
- Network hiding 

## Installation 
```git clone --recursive https://github.com/mad-cat-lon/nopfrog```
```cd nopfrog```
```sudo ./easy_install.sh``

## Hooking a single program  
Set `/proc/sys/vm/mmap_min_addr` to 0

```sudo sh -c "echo 0 > /proc/sys/vm/mmap_min_addr"```

```LIBZPHOOK=./nopfrog.so LD_PRELOAD=./libzpoline.so [program you wish to run]```

## Configuration

## FAQ

## References 
- https://intezer.com/blog/research/new-linux-threat-symbiote/
- https://magisterquis.github.io/2018/03/31/in-memory-only-elf-execution.html
 