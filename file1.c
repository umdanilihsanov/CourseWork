#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

long count_digits(const char *filepath)
{
	FILE *file = fopen(filepath, "r");
	if (!file) {
		perror("Ошибка открытия входного файла.");
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

int main() 
{	
	const char *input_dir = "input/";
	const char *output_dir = "output/";
	const char *output_filename = "result.txt";
	
	char output_path[512];
	snprintf(output_path, sizeof(output_path), "%s%s", output_dir, output_filename);
	FILE *output = fopen(output_path, "w");
	if (!output) {
		perror("Ошибка открытия выходного файла");
		return 1;
	}
		
	DIR *dp = opendir(input_dir);
	struct dirent *entry;
	
	if (dp == NULL) {
		fprintf(stderr, "Нет директории %s\n", input_dir);
		fclose(output);
		return 1;
	}
	
	printf("Прочитанные файлы:\n");
	while ((entry = readdir(dp)) != NULL) {	
		if ((strcmp(".", entry->d_name)== 0) || (strcmp("..", entry->d_name) == 0));
		else { 
		char filepath[512];
		snprintf(filepath, sizeof(filepath), "%s%s", input_dir, entry->d_name);
		long digits = count_digits(filepath);
		if (digits >= 0) {
			printf("%s\n", entry->d_name);
		}
		fprintf(output, "%s: %ld\n", entry->d_name, digits);
		}
	}
	
	fclose(output);
	closedir(dp);
	printf("Подсчет количества цифр выполнен. Результат записан в файл '%s'.\n", output_filename);
	return 0;
}
