#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    char *path = NULL;
    int jobs_pool = 0;
    int opt;

    while ((opt = getopt(argc, argv, "l:n:")) != -1) {
        switch (opt) {
            case 'l': path = optarg; break;
            case 'n': jobs_pool = atoi(optarg); break;
            default: exit(EXIT_FAILURE);
        }
    }

    if (path == NULL || jobs_pool <= 0) {
        fprintf(stderr, "Error: Missing or invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    unlink("jms_in");
    unlink("jms_out");
    if (mkfifo("jms_in", 0666) == -1 || mkfifo("jms_out", 0666) == -1) {
        perror("mkfifo"); 
        exit(1);
    }

    printf("JMS Coordinator started. Waiting for connection...\n");
    fflush(stdout);

    int fd_read = open("jms_in", O_RDONLY);
    if (fd_read == -1) { perror("open read"); exit(1); }

    int fd_write = open("jms_out", O_WRONLY);
    if (fd_write == -1) { perror("open write"); exit(1); }

    printf("Console connected.\n");
    fflush(stdout);

    char buffer[1024];
    ssize_t n;
    
    while ((n = read(fd_read, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        
        printf("Coord received: %s\n", buffer);
        fflush(stdout); 

        char ack[1100];
        snprintf(ack, sizeof(ack), "Received: %s", buffer);
        write(fd_write, ack, strlen(ack) + 1);

        if (strcmp(buffer, "exit") == 0) break;
    }

    close(fd_read);
    close(fd_write);
    unlink("jms_in");
    unlink("jms_out");
    printf("Coordinator shutting down.\n");

    return 0;
}