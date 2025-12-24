#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

long count_punct(const char *filepath, long max_punct)
{
	FILE *file = fopen(filepath, "r");
	if (!file) {
		perror("Ошибка открытия входного файла.");
		return -1;
	}
	
	long punct_count = 0;
	int ch;
	
	while ((ch = fgetc(file)) != EOF) {
		if (ispunct(ch))
			punct_count++;
	}
	
	fclose(file);
	if (max_punct <= punct_count) {
		max_punct = punct_count;	
	}
	return max_punct;
}

int main() 
{	
	const char *input_dir = "input/";
	const char *output_dir = "output/";
	const char *output_filename = "result_new.txt";
	
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
	char *max_punct_file;
	long max_punct = -1;
	long last_max_punct = -1;
	while ((entry = readdir(dp)) != NULL) {	
		if ((strcmp(".", entry->d_name)== 0) || (strcmp("..", entry->d_name) == 0)) continue;
		else { 
		char filepath[512];
		snprintf(filepath, sizeof(filepath), "%s%s", input_dir, entry->d_name);
		last_max_punct = max_punct;
		long punct = count_punct(filepath, max_punct);
		max_punct = punct;
		if (punct >= 0) {
			printf("%s\n", entry->d_name);
			}
		if (last_max_punct != max_punct) {
			max_punct_file = entry->d_name;
			}
		}
	}
	fprintf(output, "%s: %ld\n", max_punct_file, max_punct);
	
	fclose(output);
	closedir(dp);
	printf("Подсчет количества знаков пунктуации выполнен. Результат записан в файл '%s'.\n", output_filename);
	return 0;
}
