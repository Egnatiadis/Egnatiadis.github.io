#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

typedef struct { int jid; pid_t pid; time_t start; } Job;
Job jobs[100]; int j_cnt = 0;
int fd_out_coord;

void handle_shutdown(int sig) {
    int active = 0;
    for(int i=0; i<j_cnt; i++) {
        if(waitpid(jobs[i].pid, NULL, WNOHANG) == 0) {
            kill(jobs[i].pid, SIGTERM);
            active++;
        }
    }
    char res[32]; sprintf(res, "%d", active);
    write(fd_out_coord, res, strlen(res)+1);
    exit(0);
}

void execute(int id, char* cmd, char* path) {
    time_t t = time(NULL); struct tm *tm = localtime(&t);
    pid_t p = fork();
    if (p == 0) {
        char dir[512], out[550], err[550];
        sprintf(dir, "%s/outputs_%d_%d_%04d%02d%02d_%02d%02d%02d", path, id, getpid(), 
                tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        mkdir(dir, 0777);
        // Μορφή αρχείων ακριβώς: stdout_jid και stderr_jid
        sprintf(out, "%s/stdout_%d", dir, id); sprintf(err, "%s/stderr_%d", dir, id);
        int f1 = open(out, O_WRONLY|O_CREAT|O_TRUNC, 0666); 
        int f2 = open(err, O_WRONLY|O_CREAT|O_TRUNC, 0666);
        dup2(f1, 1); dup2(f2, 2);
        execl("/bin/sh", "sh", "-c", cmd, NULL); exit(1);
    }
    jobs[j_cnt].jid = id; jobs[j_cnt].pid = p; jobs[j_cnt].start = t; j_cnt++;
    char ack[128]; sprintf(ack, "JobID: %d, PID: %d", id, p);
    write(fd_out_coord, ack, strlen(ack)+1);
}

int main(int argc, char *argv[]) {
    signal(SIGTERM, handle_shutdown);
    int fd_in = open(argv[1], O_RDONLY); fd_out_coord = open(argv[2], O_WRONLY);
    char buf[2048];
    while (read(fd_in, buf, sizeof(buf)-1) > 0) {
        if (strncmp(buf, "SUBMIT", 6) == 0) {
            int j; char c[1024]; sscanf(buf, "SUBMIT %d %[^\n]", &j, c); execute(j, c, argv[3]);
        }
        else if (strncmp(buf, "status", 6) == 0) {
            int n_lim = -1; if(strncmp(buf, "status-all ", 11) == 0) n_lim = atoi(buf+11);
            int target = (strncmp(buf, "status ", 7) == 0) ? atoi(buf+7) : -1;
            char res[2048] = "";
            for(int i=0; i<j_cnt; i++) {
                if(target != -1 && jobs[i].jid != target) continue;
                if(n_lim != -1 && (time(NULL)-jobs[i].start) > n_lim) continue;
                int st; pid_t r = waitpid(jobs[i].pid, &st, WNOHANG|WUNTRACED);
                char line[256];
                if(r == 0) sprintf(line, "\nJobID %d Status: Active (running for %ld seconds)", jobs[i].jid, time(NULL)-jobs[i].start);
                else if(WIFSTOPPED(st)) sprintf(line, "\nJobID %d Status: Suspended", jobs[i].jid);
                else sprintf(line, "\nJobID %d Status: Finished", jobs[i].jid);
                strcat(res, line);
            }
            write(fd_out_coord, strlen(res)>0?res:"NONE", strlen(res)>0?strlen(res)+1:5);
        }
        else if (strcmp(buf, "show-active") == 0 || strcmp(buf, "show-finished") == 0) {
            char res[1024] = ""; int act = (strcmp(buf, "show-active")==0);
            for(int i=0; i<j_cnt; i++) {
                int r = waitpid(jobs[i].pid, NULL, WNOHANG | WUNTRACED);
                // Αν ζητάμε active και το waitpid επιστρέψει 0 (τρέχει), ή αν ζητάμε finished και r != 0
                if((act && r==0) || (!act && r!=0)) { 
                    char t[32]; sprintf(t, "\nJobID %d", jobs[i].jid); strcat(res, t); 
                }
            }
            write(fd_out_coord, strlen(res)>0?res:"NONE", strlen(res)>0?strlen(res)+1:5);
        }
        else if (strncmp(buf, "suspend", 7) == 0 || strncmp(buf, "resume", 6) == 0) {
            int tid = atoi(strchr(buf, ' ')+1); char res[128] = "NONE";
            for(int i=0; i<j_cnt; i++) if(jobs[i].jid == tid) {
                kill(jobs[i].pid, (buf[0]=='s')?SIGSTOP:SIGCONT);
                sprintf(res, "Sent %s signal to JobID %d", (buf[0]=='s')?"suspend":"resume", tid);
                break;
            }
            write(fd_out_coord, res, strlen(res)+1);
        }
        memset(buf, 0, sizeof(buf));
    }
    return 0;
}
