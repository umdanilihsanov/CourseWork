#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>

long count_punct_in_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) return -1;
    long count = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ispunct(ch)) count++;
    }
    fclose(file);
    return count;
}

int main() {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe failed");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid > 0) { // Родитель
        close(fd[0]); // Закрываем чтение
        DIR *dp = opendir("input/");
        struct dirent *entry;
        long max_punct = 0;

        if (dp) {
            while ((entry = readdir(dp)) != NULL) {
                if (entry->d_name[0] == '.') continue;
                char path[512];
                snprintf(path, sizeof(path), "input/%s", entry->d_name);
                long current = count_punct_in_file(path);
                if (current > max_punct) max_punct = current;
            }
            closedir(dp);
        }

        // Передача через pipe. Используем %10ld для фиксированной ширины (10 символов)
        char buffer[12];
        int len = sprintf(buffer, "%10ld", max_punct);
        write(fd[1], buffer, 10);
        close(fd[1]);
        
        wait(NULL); // Ждем потомка
        printf("Родитель: данные отправлены, потомок завершил работу.\n");
    } else { // Потомок
        close(fd[1]); // Закрываем запись
        char buffer[11];
        buffer[10] = '\0';
        
        read(fd[0], buffer, 10);
        long received_max = atol(buffer);
        close(fd[0]);

        FILE *out = fopen("output/result_pipe.txt", "w");
        if (out) {
            fprintf(out, "Max punctuation count: %ld\n", received_max);
            fclose(out);
        }
        exit(0);
    }
    return 0;
}
