#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>

// Функция из вашей Лаб №1
long count_digits(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Ошибка открытия входного файла");
        return -1;
    }
    long digit_count = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (isdigit(ch))
            digit_count++;
    }
    fclose(file);
    return digit_count;
}

int main() {
    const char *input_dir = "input/";
    const char *output_path = "output/result.txt";
    int fd[2]; // Дескрипторы для pipe

    if (pipe(fd) == -1) {
        perror("Ошибка создания pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Ошибка fork");
        return 1;
    }

    if (pid > 0) { 
        /* --- РОДИТЕЛЬСКИЙ ПРОЦЕСС --- */
        close(fd[0]); // Закрываем чтение, родитель только пишет

        DIR *dp = opendir(input_dir);
        if (dp == NULL) {
            perror("Ошибка открытия директории");
            close(fd[1]);
            return 1;
        }

        struct dirent *entry;
        long total_digits = 0;

        printf("Родитель: обрабатываю файлы...\n");
        while ((entry = readdir(dp)) != NULL) {
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
                continue;

            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s%s", input_dir, entry->d_name);
            long digits = count_digits(filepath);
            if (digits >= 0) {
                printf("Файл %s: %ld цифр\n", entry->d_name, digits);
                total_digits += digits;
            }
        }
        closedir(dp);

        // Передача данных: фиксируем ширину в 10 символов для надежности
        char buffer[12];
        int width = 10;
        sprintf(buffer, "%*ld", width, total_digits); 
        
        write(fd[1], buffer, width);
        close(fd[1]); // Закрываем запись, чтобы потомок получил EOF

        wait(NULL); // Ждем завершения потомка
        printf("Родитель: работа завершена.\n");

    } else { 
        /* --- ПРОЦЕСС-ПОТОМОК --- */
        close(fd[1]); // Закрываем запись, потомок только читает

        char buffer[11];
        int width = 10;
        
        // Читаем ровно столько байт, сколько отправил родитель
        ssize_t bytes_read = read(fd[0], buffer, width);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            long received_total = atol(buffer);

            FILE *output = fopen(output_path, "w");
            if (output) {
                fprintf(output, "Общее количество цифр во всех файлах: %ld\n", received_total);
                fclose(output);
                printf("Потомок: данные записаны в %s\n", output_path);
            } else {
                perror("Потомок: ошибка открытия выходного файла");
            }
        }
        close(fd[0]);
        exit(0);
    }

    return 0;
}
