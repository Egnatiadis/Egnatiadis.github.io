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

    // Parsing των ορισμάτων 
    while ((opt = getopt(argc, argv, "w:r:o:")) != -1) {
        switch (opt) {
            case 'w': 
                jms_in = optarg; 
                break;
            case 'r': 
                jms_out = optarg; 
                break;
            case 'o': 
                oper_file = optarg; 
                break;
            default: 
                fprintf(stderr, "Usage: %s -w <jms_in> -r <jms_out> [-o <ops_file>]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (jms_in == NULL || jms_out == NULL) {
        fprintf(stderr, "Missing required arguments -w and -r\n");
        exit(EXIT_FAILURE);
    }

    printf("JMS Console started.\n");
    printf("Sending to: %s | Reading from: %s\n", jms_in, jms_out);

    FILE *input_stream = stdin;
    if (oper_file != NULL) {
        input_stream = fopen(oper_file, "r");
        if (input_stream == NULL) {
            perror("Error opening operations file");
            exit(EXIT_FAILURE);
        }
        printf("Reading commands from file: %s\n", oper_file);
    } else {
        printf("Reading commands from STDIN (Type your commands):\n");
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, input_stream)) != -1) {
        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        // Check and edit line
        if (strlen(line) > 0) {
            printf("Command received: %s\n", line);
        
            if (strcmp(line, "exit") == 0) break;
        }
    }

    // CLEAR
    free(line);
    if (oper_file != NULL) {
        fclose(input_stream);
    }

    printf("JMS Console exiting safely.\n");
    return 0;
}