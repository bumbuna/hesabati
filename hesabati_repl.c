//Jacob Bumbuna <developer@devbumbuna.com>
//2022

#include "hesabati.h"
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv) {
    int t;
    void *p;
    char *line = NULL;
    size_t linesz = 0;
    printf("libhesabati v%d.%d.%d.%d\n%s\n",
    HESABATI_VERSION_MAJOR,
    HESABATI_VERSION_MINOR,
    HESABATI_VERSION_PATCH,
    HESABATI_VERSION_TWEAK,
    HESABATI_HOMEPAGE_URL);
    while(1) {
        printf(">>> ");
        if(getline(&line, &linesz, stdin) != -1) {
            if(!hesabati(line, &t, &p)) {
                if(t == TYPE_DOUBLE) {
                    printf("%f", *(hdouble_t*)p);
                } else if(t == TYPE_NUM) {
                    printf("%li", *(hnum_t*)p);
                }
                printf("\n");
            } else {
                printf("Error Occured\n");
            }
        } else {
            break;
        }
    }
    free(line);
}

