#include <stdlib.h>
#include <stdio.h>

#include "util.h"

void errif(bool condition, const char* errmsg){
    if(condition){
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
}