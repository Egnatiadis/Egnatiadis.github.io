#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    char *w, *r; int opt;

    // Ανάγνωση παραμέτρων γραμμής εντολών για τα ονόματα των pipes [cite: 50]
    while ((opt = getopt(argc, argv, "w:r:")) != -1) {
        // Το -w ορίζει το pipe γραφής προς τον coord [cite: 51]
        if (opt == 'w') w = optarg; 
        // Το -r ορίζει το pipe ανάγνωσης από τον coord [cite: 52]
        else if (opt == 'r') r = optarg;
    }

    // Άνοιγμα των named pipes jms_in και jms_out [cite: 19, 51, 52]
    int fdw = open(w, O_WRONLY);
    int fdr = open(r, O_RDONLY);

    char line[1024], res[4096];

    // Διάβασμα εντολών από το standard input του χρήστη [cite: 55]
    while (fgets(line, sizeof(line), stdin)) {
        
        // Αφαίρεση του χαρακτήρα αλλαγής γραμμής
        line[strcspn(line, "\n")] = 0; 
        if (strlen(line) == 0) continue;

        // Αποστολή της εντολής στον jms_coord μέσω του pipe [cite: 23, 73]
        write(fdw, line, strlen(line)+1);

        // Τερματισμός αν ο χρήστης δώσει την εντολή εξόδου
        if (strcmp(line, "exit") == 0) break;

        // Ανάγνωση της απάντησης από τον jms_coord [cite: 26, 73]
        int n = read(fdr, res, sizeof(res)-1);
        
        // Εκτύπωση του αποτελέσματος στην οθόνη του χρήστη [cite: 26]
        if (n > 0) { 
            res[n] = '\0'; 
            printf("%s\n", res); 
        }
    }

    // Κλείσιμο των file descriptors πριν την έξοδο
    close(fdw);
    close(fdr);

    return 0;
}
