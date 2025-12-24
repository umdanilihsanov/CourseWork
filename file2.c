#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

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
    
    // В реальных задачах здесь нужен мьютекс и cond_var, 
    // но для простоты лаб. работы используем флаг готовности
    while(!data->ready) { /* ждем */ }

    FILE *out = fopen("output/result_threads.txt", "w");
    if (out) {
        fprintf(out, "Max punctuation count (from thread): %ld\n", data->result);
        fclose(out);
    }
    return NULL;
}

int main() {
    shared_data data = {0, 0};
    pthread_t tid;

    if (pthread_create(&tid, NULL, writer_thread, &data) != 0) {
        perror("Thread error");
        return 1;
    }

    // Основная нить: обработка
    DIR *dp = opendir("input/");
    struct dirent *entry;
    if (dp) {
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            char path[512];
            snprintf(path, sizeof(path), "input/%s", entry->d_name);
            long current = count_punct_in_file(path);
            if (current > data.result) data.result = current;
        }
        closedir(dp);
    }

    data.ready = 1; // Сигнализируем второй нити
    pthread_join(tid, NULL);
    
    printf("Программа завершена. Результат в файле.\n");
    return 0;
}
