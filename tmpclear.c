#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define TMP_DIR "/tmp"
#define MAX_AGE 10

int main(void) {
    DIR *dir = opendir(TMP_DIR);
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    time_t now = time(NULL);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", TMP_DIR, entry->d_name);

        struct stat st;
        if (stat(path, &st) == -1) {
            perror("stat");
            continue;
        }
        
        printf("Found file: %s\n", path);
        fflush(stdout);


        if (difftime(now, st.st_mtime) > MAX_AGE) {
            printf("Want to remove: %s (last modified: %s)", path, ctime(&st.st_mtime));
            fflush(stdout);

            printf("Delete? (y/n): ");
            fflush(stdout);

            char answer;
            scanf(" %c", &answer);

            if (answer == 'y' || answer == 'Y') {
                if (remove(path) == 0) {
                    printf("Removed: %s\n", path);
                } else {
                    perror("remove");
                }
            } else {
                printf("Skipped: %s\n", path);
            }
        }
    }

    closedir(dir);
    return 0;
}
