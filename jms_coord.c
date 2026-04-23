#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_POOLS 50

typedef struct {
    pid_t pid;
    int job_count;
    int fd_in;
    int fd_out;
    char p_in[64];
    char p_out[64];
} Pool;

int main(int argc, char *argv[]) {
    char *path = NULL; int limit = 0, opt;
    while ((opt = getopt(argc, argv, "l:n:")) != -1) {
        if (opt == 'l') path = optarg; else if (opt == 'n') limit = atoi(optarg);
    }
    if (!path || limit <= 0) exit(1);

    unlink("jms_in"); unlink("jms_out");
    mkfifo("jms_in", 0666); mkfifo("jms_out", 0666);
    int fdr = open("jms_in", O_RDONLY);
    int fdw = open("jms_out", O_WRONLY);

    Pool pools[MAX_POOLS];
    int p_count = 0, total_j = 0;
    char buf[2048];

    while (read(fdr, buf, sizeof(buf)-1) > 0) {
        buf[strcspn(buf, "\n\r")] = 0;
        if (strlen(buf) == 0) continue;

        if (strncmp(buf, "submit ", 7) == 0) {
            total_j++;
            int target = -1;
            for(int i=0; i<p_count; i++) if(pools[i].job_count < limit) { target=i; break; }
            if (target == -1 && p_count < MAX_POOLS) {
                target = p_count++;
                sprintf(pools[target].p_in, "p%d_in", target); 
                sprintf(pools[target].p_out, "p%d_out", target);
                mkfifo(pools[target].p_in, 0666); mkfifo(pools[target].p_out, 0666);
                pid_t pid = fork();
                if (pid == 0) { execl("./jms_pool", "./jms_pool", pools[target].p_in, pools[target].p_out, path, NULL); exit(1); }
                pools[target].pid = pid;
                pools[target].fd_in = open(pools[target].p_in, O_WRONLY);
                pools[target].fd_out = open(pools[target].p_out, O_RDONLY);
                pools[target].job_count = 0;
            }
            char m[2048]; sprintf(m, "SUBMIT %d %s", total_j, buf+7);
            write(pools[target].fd_in, m, strlen(m)+1);
            pools[target].job_count++;
            char ack[128]; read(pools[target].fd_out, ack, 128);
            write(fdw, ack, strlen(ack)+1);
        }
        else if (strcmp(buf, "show-pools") == 0) {
            char res[2048] = "Pool & NumOfJobs:";
            for(int i=0; i<p_count; i++) {
                char tmp[64]; sprintf(tmp, "\n%d %d", pools[i].pid, pools[i].job_count);
                strcat(res, tmp);
            }
            write(fdw, res, strlen(res)+1);
        }
        else if (strncmp(buf, "status", 6) == 0 || strncmp(buf, "suspend", 7) == 0 || 
                 strncmp(buf, "resume", 6) == 0 || strcmp(buf, "show-active") == 0 || 
                 strcmp(buf, "show-finished") == 0) {
            char final[4096] = "";
            int is_status = (strncmp(buf, "status", 6) == 0);
            
            // Προσθήκη Headers μόνο αν η εντολή το απαιτεί
            if(strcmp(buf, "show-active") == 0) strcpy(final, "Active jobs:");
            else if(strcmp(buf, "show-finished") == 0) strcpy(final, "Finished jobs:");

            for(int i=0; i<p_count; i++) {
                write(pools[i].fd_in, buf, strlen(buf)+1);
                char temp[2048]; int n = read(pools[i].fd_out, temp, 2048);
                if(n > 0) { 
                    temp[n] = '\0'; 
                    if(strcmp(temp, "NONE") != 0) strcat(final, temp); 
                }
            }
            if(strlen(final) == 0 && is_status) strcpy(final, "JobID not found");
            write(fdw, final, strlen(final)+1);
        }
        else if (strcmp(buf, "shutdown") == 0) {
            int in_prog = 0;
            for(int i=0; i<p_count; i++) {
                kill(pools[i].pid, SIGTERM);
                char p_res[128]; read(pools[i].fd_out, p_res, 128);
                in_prog += atoi(p_res);
                close(pools[i].fd_in); close(pools[i].fd_out);
                unlink(pools[i].p_in); unlink(pools[i].p_out);
            }
            char stat[128]; sprintf(stat, "Served %d jobs, %d were still in progress", total_j, in_prog);
            write(fdw, stat, strlen(stat)+1);
            break;
        }
        memset(buf, 0, sizeof(buf));
    }
    return 0;
}