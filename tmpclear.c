#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <limits.h> // Included for PATH_MAX

#define TMP_DIR "/tmp"
#define MAX_AGE 10

// Helper function to clear the input buffer and prevent infinite loops
void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

int main(void) {
    DIR *dir = opendir(TMP_DIR);
    if (!dir) {
        perror("opendir failed on " TMP_DIR);
        return 1;
    }

    struct dirent *entry;
    time_t now = time(NULL);
    int yes_to_all = 0; // State flag for "yes to all"

    while ((entry = readdir(dir)) != NULL) {
        // Skip current and parent directory pointers
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char path[PATH_MAX];
        
        // Build the path and check for truncation
        int n = snprintf(path, sizeof(path), "%s/%s", TMP_DIR, entry->d_name);
        if (n >= sizeof(path) || n < 0) {
            fprintf(stderr, "Warning: Path truncated or error for %s\n", entry->d_name);
            continue;
        }

        struct stat st;
        // Use lstat instead of stat to evaluate symlinks directly (safer)
        if (lstat(path, &st) == -1) {
            // Silently skip files we don't have permission to read
            continue; 
        }
        
        if (difftime(now, st.st_mtime) > MAX_AGE) {
            int do_delete = yes_to_all; // Automatically delete if "yes to all" is active

            // Only prompt if "yes to all" hasn't been triggered yet
            if (!yes_to_all) {
                // ctime includes a newline, so format accordingly
                printf("\nWant to remove: %s\nLast modified: %s", path, ctime(&st.st_mtime));
                printf("Delete? (y = yes, n = no, a = all, q = quit/skip rest): ");
                fflush(stdout);

                char answer;
                if (scanf(" %c", &answer) == 1) {
                    clear_stdin(); // Consume leftover characters like the 'Enter' key

                    if (answer == 'a' || answer == 'A') {
                        yes_to_all = 1;
                        do_delete = 1;
                    } else if (answer == 'q' || answer == 'Q') {
                        printf("🛑 Skipping all remaining files. Exiting cleanup.\n");
                        break; // Break out of the while loop entirely
                    } else if (answer == 'y' || answer == 'Y') {
                        do_delete = 1;
                    } else {
                        printf("⏭️ Skipped: %s\n", path);
                    }
                }
            }

            // Execute the deletion if the flag is set (either manually or via "yes to all")
            if (do_delete) {
                if (remove(path) == 0) {
                    printf("✅ Removed: %s\n", path);
                } else {
                    perror("❌ Failed to remove");
                }
            }
        }
    }

    closedir(dir);
    return 0;
}
