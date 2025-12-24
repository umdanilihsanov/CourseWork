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
        int file_count = 0;

        while ((entry = readdir(dp)) != NULL) {
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
                continue;

            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s%s", input_dir, entry->d_name);
            
            long digits = count_digits(filepath);
            if (digits >= 0) {
                printf("[Родитель]: Обработан файл '%s' -> Найдено цифр: %ld\n", entry->d_name, digits);
                total_digits += digits;
                file_count++;
            }
        }
        closedir(dp);

        printf("[Родитель]: Обработка завершена. Всего файлов: %d. Общая сумма: %ld\n", file_count, total_digits);
        printf("[Родитель]: Отправляю результат в pipe потомку...\n");

        // Передача данных: фиксируем ширину в 10 символов
        char buffer[12];
        int width = 10;
        sprintf(buffer, "%*ld", width, total_digits); 
        
        if (write(fd[1], buffer, width) == -1) {
            perror("[Родитель]: Ошибка записи в pipe");
        }
        
        close(fd[1]); // Закрываем запись
        printf("[Родитель]: Данные переданы. Ожидаю завершения потомка (wait)...\n");

        wait(NULL); // Ждем завершения потомка
        printf("[Родитель]: Потомок завершил работу. Программа успешно закончена.\n");

    } else { 
        /* --- ПРОЦЕСС-ПОТОМОК --- */
        close(fd[1]); // Закрываем запись
        printf("[Потомок]: PID = %d. Запущен и ожидает данные из pipe...\n", getpid());

        char buffer[11];
        int width = 10;
        
        // Читаем ровно столько байт, сколько отправил родитель
        ssize_t bytes_read = read(fd[0], buffer, width);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            long received_total = atol(buffer);
            printf("[Потомок]: Получено число из pipe: %ld\n", received_total);

            printf("[Потомок]: Открываю файл '%s' для записи...\n", output_path);
            FILE *output = fopen(output_path, "w");
            if (output) {
                fprintf(output, "Общее количество цифр во всех файлах: %ld\n", received_total);
                fclose(output);
                printf("[Потомок]: Результат успешно записан. Завершаюсь.\n");
            } else {
                perror("[Потомок]: Ошибка открытия выходного файла");
            }
        } else {
            printf("[Потомок]: Ошибка: не удалось прочитать данные из pipe.\n");
        }
        
        close(fd[0]);
        exit(0);
    }

    return 0;
}
