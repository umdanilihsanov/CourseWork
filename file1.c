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
        perror("[Родитель]: Ошибка открытия входного файла");
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

    printf("[Система]: Инициализация программы...\n");

    if (pipe(fd) == -1) {
        perror("[Система]: Ошибка создания pipe");
        return 1;
    }

    printf("[Система]: Pipe создан. Выполняю fork()...\n");
    pid_t pid = fork();

    if (pid < 0) {
        perror("[Система]: Ошибка fork");
        return 1;
    }

    if (pid > 0) { 
        /* --- РОДИТЕЛЬСКИЙ ПРОЦЕСС --- */
        close(fd[0]); // Закрываем чтение
        printf("[Родитель]: PID = %d. Начинаю сканирование файлов...\n", getpid());

        DIR *dp = opendir(input_dir);
        if (dp == NULL) {
            perror("[Родитель]: Ошибка открытия директории");
            close(fd[1]);
            return 1;
        }

        struct dirent *entry;
        long total_digits = 0;
        int file_count = 0
