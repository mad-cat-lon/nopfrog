#define _GNU_SOURCE
#define _LARGEFILE_SOURCE 1

#ifdef DEBUG
#define DEBUG_MSG(...) fprintf(stderr, __VA_ARGS__);
#else
#define DEBUG_MSG(...)
#endif 
#define CLEAN(var) clean(var, strlen(var))

#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_STAT 4
#define SYS_FSTAT 5
#define SYS_LSTAT 6
#define SYS_ACCESS 21
#define SYS_EXECVE 59
#define SYS_KILL 62
#define SYS_GETDENTS 78
#define SYS_GETDENTS64 217
#define SYS_OPENAT 257