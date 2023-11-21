#include <string.h> 
#include <stdio.h> 
#include "../rkconsts.h"

int check_if_hidden_path(const char *path) {
    if ((strstr(path, HIDDEN_PATH_STR)) != NULL) {
        return 1;
    }
    return 0;
}