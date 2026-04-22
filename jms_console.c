#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    char *jms_in = NULL;
    char *jms_out = NULL;
    char *oper_file = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "w:r:o:")) != -1) {
        switch (opt) {
            case 'w': jms_in = optarg; break;
            case 'r': jms_out = optarg; break;
            case 'o': oper_file = optarg; break;
            default: exit(EXIT_FAILURE);
        }
    }

    if (jms_in == NULL || jms_out == NULL) {
        fprintf(stderr, "Usage: %s -w <jms_in> -r <jms_out> [-o <ops_file>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Άνοιγμα pipes (Η σειρά είναι σημαντική για να μην κλειδώσουν οι διεργασίες)
    int fd_write = open(jms_in, O_WRONLY);
    if (fd_write == -1) exit(1);
    
    int fd_read = open(jms_out, O_RDONLY);
    if (fd_read == -1) exit(1);

    FILE *input_stream = (oper_file != NULL) ? fopen(oper_file, "r") : stdin;
    if (input_stream == NULL) exit(1);

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    char response[2048];

    while ((nread = getline(&line, &len, input_stream)) != -1) {
        if (nread > 0 && line[nread - 1] == '\n') line[nread - 1] = '\0';

        if (strlen(line) > 0) {
            write(fd_write, line, strlen(line) + 1);

            ssize_t n = read(fd_read, response, sizeof(response) - 1);
            if (n > 0) {
                response[n] = '\0';
                printf("%s\n", response);
            }

            if (strcmp(line, "exit") == 0) break;
        }
    }

    free(line);
    if (oper_file != NULL) fclose(input_stream);
    close(fd_write);
    close(fd_read);
    return 0;
}