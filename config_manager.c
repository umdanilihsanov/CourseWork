#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// Конфигурация путей и параметров
#define REPO_DIR "./CourseWork"       // Где лежит Git-репозиторий
#define TARGET_DIR "/tmp/system_config_target" // Куда копируем конфиги (имитация /etc)
#define SERVICE_NAME "nginx"                 // Какую службу перезапускаем
#define MAX_COMMAND_LEN 1024

// --- Вспомогательные функции ---

/**
 * Рекурсивное создание каталогов (аналог mkdir -p)
 */
int mkdir_p(const char *path) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (len > 0 && tmp[len - 1] == '/') tmp[len - 1] = 0;

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST) return -1;
            *p = '/';
        }
    }
    if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST) return -1;
    return 0;
}

/**
 * Выполнение команды в определенном каталоге
 */
int execute_in_dir(const char *command, const char *dir) {
    char original_cwd[MAX_COMMAND_LEN];
    if (getcwd(original_cwd, sizeof(original_cwd)) == NULL) return -1;

    if (chdir(dir) != 0) {
        fprintf(stderr, "Ошибка: Не удалось перейти в %s: %s\n", dir, strerror(errno));
        return -1;
    }

    printf("Выполнение: %s\n", command);
    int status = system(command);

    if (chdir(original_cwd) != 0) {
        fprintf(stderr, "Ошибка возврата в каталог\n");
    }
    return status;
}

// --- Основные функции системы ---

// 1. Инициализация
int clone_repo() {
    printf("\n--- Клонирование репозитория ---\n");
    int status = system("git clone https://github.com/umdanilihsanov/CourseWork.git");
    return status;
}

// 2. Синхронизация и применение
int pull_and_apply() {
    char cmd[MAX_COMMAND_LEN];
    printf("\n--- Обновление и применение конфигурации ---\n");

    // Pull
    if (execute_in_dir("git pull origin main", REPO_DIR) != 0) {
        fprintf(stderr, "Предупреждение: git pull не удался (возможно, не настроен remote)\n");
    }

    // Применение (копирование)
    if (mkdir_p(TARGET_DIR) != 0) return -1;
    snprintf(cmd, MAX_COMMAND_LEN, "cp -vR %s/* %s/", REPO_DIR, TARGET_DIR);
    if (system(cmd) != 0) return -1;

    // Перезапуск службы
    snprintf(cmd, MAX_COMMAND_LEN, "sudo systemctl restart %s", SERVICE_NAME);
    return system(cmd);
}

// 3. Откат
int rollback() {
    char cmd[MAX_COMMAND_LEN];
    printf("\n--- Откат к предыдущему состоянию (HEAD~1) ---\n");
    
    if (execute_in_dir("git reset --hard", REPO_DIR) != 0) return -1;
    
    // Переприменяем файлы после отката
    snprintf(cmd, MAX_COMMAND_LEN, "cp -vR %s/* %s/", REPO_DIR, TARGET_DIR);
    system(cmd);
    
    snprintf(cmd, MAX_COMMAND_LEN, "sudo systemctl restart %s", SERVICE_NAME);
    return system(cmd);
}

// 4. Просмотр изменений
int diff(int argc, char *argv[]) {
    char cmd[MAX_COMMAND_LEN];
    if (argc == 4) {
        snprintf(cmd, MAX_COMMAND_LEN, "git --no-pager diff %s %s", argv[2], argv[3]);
    } else {
        strcpy(cmd, "git --no-pager diff HEAD");
    }
    return execute_in_dir(cmd, REPO_DIR);
}

// 5. Управление ветками
int create_branch(const char *name) {
    char cmd[MAX_COMMAND_LEN];
    snprintf(cmd, MAX_COMMAND_LEN, "git checkout -b %s", name);
    return execute_in_dir(cmd, REPO_DIR);
}

// 6. Слияние
int merge(const char *source) {
    char cmd[MAX_COMMAND_LEN];
    // Для простоты сливаем в текущую ветку
    snprintf(cmd, MAX_COMMAND_LEN, "git merge %s", source);
    return execute_in_dir(cmd, REPO_DIR);
}

/**
 * Фиксация изменений: git add . && git commit -m "сообщение"
 */
int commit_changes(const char *message) {
    char cmd[MAX_COMMAND_LEN];
    printf("\n--- Фиксация изменений (Commit) ---\n");

    // 1. Добавляем все измененные файлы в индекс
    if (execute_in_dir("git add .", REPO_DIR) != 0) {
        fprintf(stderr, " Ошибка при выполнении 'git add'.\n");
        return -1;
    }

    // 2. Делаем коммит с переданным сообщением
    snprintf(cmd, MAX_COMMAND_LEN, "git commit -m \"%s\"", message);
    int res = execute_in_dir(cmd, REPO_DIR);
    
    if (res == 0) {
        printf(" Изменения успешно зафиксированы.\n");
    } else {
        printf(" Нет изменений для коммита или произошла ошибка.\n");
    }
    return res;
}

/**
 * Отправка изменений на сервер: git push <remote> <branch>
 */
int push_changes(const char *remote, const char *branch) {
    char cmd[MAX_COMMAND_LEN];
    printf("\n--- Отправка на удаленный сервер (Push) ---\n");

    // Формируем команду, например: git push origin main
    snprintf(cmd, MAX_COMMAND_LEN, "git push %s %s", remote, branch);
    
    int res = execute_in_dir(cmd, REPO_DIR);
    if (res == 0) {
        printf(" Изменения успешно отправлены в %s/%s.\n", remote, branch);
    } else {
        fprintf(stderr, " Ошибка при выполнении push. Проверьте настройки SSH/HTTPS.\n");
    }
    return res;
}

// --- Точка входа ---

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Использование: %s <команда> [аргументы]\n", argv[0]);
        printf("Команды:\n");
        printf("  clone            	 - создать репозиторий\n");
        printf("  pull             	 - обновить из git и применить\n");
        printf("  rollback        	 - откатить на 1 коммит назад\n");
        printf("  diff [b1 b2]     	 - показать изменения\n");
        printf("  branch <имя>           - создать новую ветку\n");
        printf("  merge <имя>            - слить ветку в текущую\n");
        printf("  commit /сообщение/     - сохранить изменения локально\n");
        printf("  push [remote][branch]  - отправить на удаленный сервер\n");
        return 1;
    }

    const char *action = argv[1];

    if (strcmp(action, "clone") == 0) return clone_repo();
    
    // Проверка существования репозитория для остальных команд
    if (access(REPO_DIR, F_OK) != 0) {
        fprintf(stderr, "Ошибка: Репозиторий не найден. Сначала выполните 'clone'.\n");
        return 1;
    }

    if (strcmp(action, "pull") == 0) return pull_and_apply();
    if (strcmp(action, "rollback") == 0) return rollback();
    if (strcmp(action, "diff") == 0) return diff(argc, argv);
    if (strcmp(action, "branch") == 0 && argc == 3) return create_branch(argv[2]);
    if (strcmp(action, "merge") == 0 && argc == 3) return merge(argv[2]);

    if (strcmp(action, "commit") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Ошибка: Нужно указать сообщение коммита.\n");
            return 1;
        }
        return commit_changes(argv[2]);
    }

    if (strcmp(action, "push") == 0) {
        const char *remote = (argc >= 3) ? argv[2] : "origin";
        const char *branch = (argc >= 4) ? argv[3] : "main";
        return push_changes(remote, branch);
    }
    
    fprintf(stderr, "Неверная команда или недостаточно аргументов.\n");
    return 1;
}
