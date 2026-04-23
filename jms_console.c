#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    char *w, *r; int opt;
    while ((opt = getopt(argc, argv, "w:r:")) != -1) {
        if (opt == 'w') w = optarg; else if (opt == 'r') r = optarg;
    }
    int fdw = open(w, O_WRONLY), fdr = open(r, O_RDONLY);
    char line[1024], res[4096];
    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\n")] = 0; if (strlen(line) == 0) continue;
        write(fdw, line, strlen(line)+1);
        if (strcmp(line, "exit") == 0) break;
        int n = read(fdr, res, sizeof(res)-1);
        if (n > 0) { res[n] = '\0'; printf("%s\n", res); }
    }
    return 0;
}