#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
int main(int argc, char *argv[]) {
    char inbuff[4096];
    int retnum = 0;
    if (argc < 3) {
        fprintf(stderr, "improper args, must have a delimiter and a source file\n");
        return 22;
    }
    if (strlen(argv[1]) > 1) {
        fprintf(stderr, "multi character delimiter, use only single character delimiter.\n");
        return 2;
    }
    char delim = argv[1][0];
    int file = 0;
    int bytreadcnt = 4096;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            do {
                bytreadcnt = read(0, inbuff, 4096);
                for (int i = 0; i < bytreadcnt; i++) {
                    if (inbuff[i] == delim) {
                        inbuff[i] = '\n';
                    }
                }
                if (-1 == write(1, inbuff, bytreadcnt)) {
                    retnum = 245;
                    fprintf(stderr, "Failed to write to stdout\n");
                }
            } while (bytreadcnt > 0);

        } else {
            file = open(argv[i], O_RDONLY);
            if (file <= 0) {
                fprintf(stderr, "Failed to open file\n");
                retnum = 2;
            }
            do {
                bytreadcnt = read(file, inbuff, 4096);

                for (int i = 0; i < bytreadcnt; i++) {
                    if (inbuff[i] == delim) {
                        inbuff[i] = '\n';
                    }
                }

                if (-1 == write(1, inbuff, bytreadcnt)) {
                    retnum = 245;
                }

            } while (bytreadcnt >= 4096);
            close(file);
        }
    }
    return retnum;
}
