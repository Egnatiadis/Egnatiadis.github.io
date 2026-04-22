#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_POOLS 10

typedef struct {
    pid_t pid;
    int job_count;
    int fd_in;
    int fd_out;
} PoolInfo;

int main(int argc, char *argv[]) {
    char *path = NULL;
    int jobs_pool_limit = 0;
    int opt;

    while ((opt = getopt(argc, argv, "l:n:")) != -1) {
        switch (opt) {
            case 'l': path = optarg; break;
            case 'n': jobs_pool_limit = atoi(optarg); break;
            default: exit(1);
        }
    }

    if (path == NULL || jobs_pool_limit <= 0) exit(1);

    unlink("jms_in");
    unlink("jms_out");
    if (mkfifo("jms_in", 0666) == -1 || mkfifo("jms_out", 0666) == -1) exit(1);

    int fd_read_cons = open("jms_in", O_RDONLY);
    int fd_write_cons = open("jms_out", O_WRONLY);

    PoolInfo pools[MAX_POOLS];
    int active_pools = 0;
    int total_jobs = 0;

    char buffer[1024];
    ssize_t n;
    
    while ((n = read(fd_read_cons, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        
        if (strncmp(buffer, "submit ", 7) == 0) {
            total_jobs++;
            int assigned = 0;

            for (int i = 0; i < active_pools; i++) {
                if (pools[i].job_count < jobs_pool_limit) {
                    char pool_msg[1024];
                    snprintf(pool_msg, sizeof(pool_msg), "SUBMIT %d %s", total_jobs, buffer + 7);
                    write(pools[i].fd_in, pool_msg, strlen(pool_msg) + 1);
                    pools[i].job_count++;
                    assigned = 1;
                    
                    char ack[128];
                    snprintf(ack, sizeof(ack), "JobID: %d, PID: %d", total_jobs, pools[i].pid);
                    write(fd_write_cons, ack, strlen(ack) + 1);
                    break;
                }
            }
            if (!assigned && active_pools < MAX_POOLS) {
                char p_in[32], p_out[32];
                snprintf(p_in, 32, "p_in_%d", active_pools);
                snprintf(p_out, 32, "p_out_%d", active_pools);
                unlink(p_in); unlink(p_out);
                mkfifo(p_in, 0666); mkfifo(p_out, 0666);

                pid_t pid = fork();
                if (pid == 0) {
                    execl("./jms_pool", "./jms_pool", p_in, p_out, path, NULL);
                    exit(1);
                } else {
                    pools[active_pools].pid = pid;
                    pools[active_pools].job_count = 1;
                    pools[active_pools].fd_in = open(p_in, O_WRONLY);
                    pools[active_pools].fd_out = open(p_out, O_RDONLY);
                    
                    char pool_msg[1024];
                    snprintf(pool_msg, sizeof(pool_msg), "SUBMIT %d %s", total_jobs, buffer + 7);
                    write(pools[active_pools].fd_in, pool_msg, strlen(pool_msg) + 1);
                    
                    char ack[128];
                    snprintf(ack, sizeof(ack), "JobID: %d, PID: %d", total_jobs, pid);
                    write(fd_write_cons, ack, strlen(ack) + 1);
                    active_pools++;
                }
            }
        } else if (strcmp(buffer, "shutdown") == 0) {
            for(int i=0; i<active_pools; i++) {
                write(pools[i].fd_in, "SHUTDOWN", 9);
                close(pools[i].fd_in); close(pools[i].fd_out);
            }
            char stats[128];
            snprintf(stats, sizeof(stats), "Served %d jobs, 0 were still in progress", total_jobs);
            write(fd_write_cons, stats, strlen(stats) + 1);
            break;
        } else {
            write(fd_write_cons, "Command processed", 18);
        }
        if (strcmp(buffer, "exit") == 0) break;
    }

    close(fd_read_cons); close(fd_write_cons);
    unlink("jms_in"); unlink("jms_out");
    return 0;
}