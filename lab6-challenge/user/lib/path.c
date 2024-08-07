#include <lib.h>

void parse_path(char *path, char *ans) {
	char cur_path[MAXPATHLEN] = {0}; 
	char tmp_path[MAXPATHLEN] = {0};
	char new_path[MAXPATHLEN] = {0};
	syscall_get_cur_path(cur_path);
	if (path[0] != '/') {	//传进来的是相对路径
		if (cur_path[strlen(cur_path) - 1] != '/') {
			strcpy(tmp_path, cur_path);
			strcat(tmp_path, "/");
			strcat(tmp_path, path);
		} else {
			strcpy(tmp_path, cur_path);
			strcat(tmp_path, path);
		}
	} else {
		strcpy(tmp_path, path);
	}
	char files[128][128] = {0};
	char new_files[128][128] = {0};
	char tmp_file[128] = {0};
	int p = 0;	//指针
	int length = 0, file_cnt = 0;
	int tmp_path_len = strlen(tmp_path);
	if (tmp_path[tmp_path_len - 1] != '/') {
		tmp_path[tmp_path_len] = '/';
		tmp_path[tmp_path_len + 1] = '\0';
	}
	for (int i = 1; tmp_path[i]; i++) {
		if (tmp_path[i] != '/') {
			tmp_file[length++] = tmp_path[i];
		} else {
			tmp_file[length] = '\0';
			strcpy(files[file_cnt++], tmp_file);
			memset(tmp_file, 0, sizeof(tmp_file));
			length = 0;
		}
	}
	for (int i = 0; i < file_cnt; i++) {
		if (strcmp(files[i], "..") == 0) {
			p--;
		} else if (strcmp(files[i], ".") == 0) {
			continue;
		} else {
			memset(new_files[p], 0, sizeof(new_files[p]));
			strcpy(new_files[p++], files[i]);
		}
	}
	ans[0] = '/';
	for (int i = 0; i < p; i++) {
		strcat(ans, new_files[i]);
		if (i != p - 1) {
			strcat(ans, "/");
		}
	}
}

int is_prefix(char *s1, char *s2) {	//判断s2是不是s1的前缀
	int len1 = strlen(s1);
	int len2 = strlen(s2);
	if (len2 > len1) {
		return 0;
	}
	for (int i = 0; i < len2; i++) {
		if (s2[i] != s1[i]) {
			return 0;
		}
	}
	return 1;
}