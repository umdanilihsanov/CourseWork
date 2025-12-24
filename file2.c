#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>

typedef struct {
    long result;
    int ready; // Флаг готовности
} shared_data;

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

void* writer_thread(void* arg) {
    shared_data *data = (shared_data*)arg;
    
    printf("[Нить-записи]: Запущена. Ожидаю сигнала готовности данных...\n");
    
    // Ожидание готовности данных
    while(!data->ready) { 
        // Небольшая пауза, чтобы не перегружать процессор в цикле ожидания
        usleep(1000); 
    }

    printf("[Нить-записи]: Данные получены. Записываю результат %ld в файл...\n", data->result);

    FILE *out = fopen("output/result_threads.txt", "w");
    if (out) {
        fprintf(out, "Max punctuation count (from thread): %ld\n", data->result);
        fclose(out);
        printf("[Нить-записи]: Файл успешно сохранен. Завершаю работу.\n");
    } else {
        perror("[Нить-записи]: Ошибка открытия выходного файла");
    }
    return NULL;
}

int main() {
    shared_data data = {0, 0};
    pthread_t tid;

    printf("[Main]: Программа запущена. Подготовка структуры данных.\n");

    if (pthread_create(&tid, NULL, writer_thread, &data) != 0) {
        perror("[Main]: Ошибка создания нити");
        return 1;
    }
    printf("[Main]: Нить-записи успешно создана (ID: %lu).\n", (unsigned long)tid);

    // Основная нить: обработка
    DIR *dp = opendir("input/");
    struct dirent *entry;
    if (dp) {
        printf("[Main]: Начинаю сканирование директории 'input/'...\n");
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            
            char path[512];
            snprintf(path, sizeof(path), "input/%s", entry->d_name);
            
            long current = count_punct_in_file(path);
            printf("[Main]: Обработан файл '%s' (найдено знаков: %ld).\n", entry->d_name, current);
            
            if (current > data.result) {
                data.result = current;
                printf("[Main]: Обновлен локальный максимум: %ld\n", data.result);
            }
        }
        closedir(dp);
    } else {
        perror("[Main]: Ошибка открытия директории 'input/'");
    }

    printf("[Main]: Обработка завершена. Отправляю сигнал готовности...\n");
    data.ready = 1; 

    // Ждем завершения пишущей нити
    pthread_join(tid, NULL);
    
    printf("[Main]: Все нити завершены. Выход из программы.\n");
    return 0;
}
