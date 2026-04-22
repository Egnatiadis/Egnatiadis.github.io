#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>

void execute_job(int jobID, char* command, char* log_path) {
    char folder[1024];
    char out_file[1100]; 
    char err_file[1100];
    
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    pid_t job_pid = fork();

    if (job_pid == 0) { // Child process
        // Δημιουργία ονόματος φακέλου σύμφωνα με την εκφώνηση
        snprintf(folder, sizeof(folder), "%s/outputs_%d_%d_%04d%02d%02d_%02d%02d%02d", 
                log_path, jobID, getpid(), 
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
        
        // Δημιουργία φακέλου (χρειάζεται έλεγχο αν υπάρχει το log_path)
        mkdir(folder, 0777);

        snprintf(out_file, sizeof(out_file), "%s/stdout_%d", folder, jobID);
        snprintf(err_file, sizeof(err_file), "%s/stderr_%d", folder, jobID);

        int fd_out = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        int fd_err = open(err_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);

        if (fd_out >= 0 && fd_err >= 0) {
            dup2(fd_out, STDOUT_FILENO);
            dup2(fd_err, STDERR_FILENO);
            close(fd_out); 
            close(fd_err);
        }

        char *args[] = {"sh", "-c", command, NULL};
        execvp("sh", args);
        perror("execvp failed");
        exit(1);
    } else if (job_pid > 0) {
        // Silent
    } else {
        perror("fork failed");
    }
}

int main(int argc, char *argv[]) {
    // Τα ορίσματα πρέπει να είναι: <read_pipe> <write_pipe> <log_path>
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <read_pipe> <write_pipe> <log_path>\n", argv[0]);
        return 1;
    }

    int fd_read = open(argv[1], O_RDONLY);
    if (fd_read < 0) { perror("Pool: open read_pipe"); exit(1); }
    
    int fd_write = open(argv[2], O_WRONLY);
    if (fd_write < 0) { perror("Pool: open write_pipe"); close(fd_read); exit(1); }

    char *log_path = argv[3];
    char buf[2048]; 

    while (1) {
        memset(buf, 0, sizeof(buf));
        ssize_t n = read(fd_read, buf, sizeof(buf) - 1);
        if (n <= 0) break;

        if (strncmp(buf, "SUBMIT", 6) == 0) {
            int jid;
            char cmd[1024];
            if (sscanf(buf, "SUBMIT %d %[^\n]", &jid, cmd) == 2) {
                execute_job(jid, cmd, log_path);
                // Πρέπει να στείλει πίσω πληροφορίες στον συντονιστή
                write(fd_write, "ACK_SUBMIT", 11);
            }
        } else if (strcmp(buf, "SHUTDOWN") == 0) {
            break;
        }
    }

    close(fd_read);
    close(fd_write);
    return 0;
}
