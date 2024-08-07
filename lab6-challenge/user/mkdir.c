#include <lib.h>
#include <fs.h>
#include <fsreq.h>

int flag[256];

int dir_is_exist(char *path) {
	int fd;
	fd = open(path, O_RDONLY);
	close(fd);
	return fd >= 0 ? 1 : 0;
}

void mkdir(char *path) {
    int r, i, count = 0;;
	char dirs[50][1024];
	i = 0;
	int pathLen = strlen(path);
	while (i < pathLen) {
		int length = 0;
		while (path[i] != '/' && path[i]) {
			dirs[count][length++] = path[i++];
		}
		dirs[count++][length] = '\0';
		i++;
	}
	char now[1024] = {0};
	struct File *f;
	i = 0;
	while (i < count) {
		strcat(now, "/");
		strcat(now, dirs[i]);
		if (flag['p']) {
			if (!dir_is_exist(now)) {
				create(now, FTYPE_DIR);
			}
		} else {
			if (i == count - 1) { //最后一个dir，要创建
				create(now, FTYPE_DIR);
			} else {
				if (!dir_is_exist(now)) {
					debugf("mkdir fail\n");
				}
			}
		}
		i++;
	}
}

void usage(void) {
	printf("usage: mkdir [-p] [file...]\n");
	exit();
}

int main(int argc, char **argv) {
	ARGBEGIN {
	default:
		usage();
	case 'p':
		flag[(u_char)ARGC()]++;
		break;
	}
	ARGEND
	for (int i = 0; i < argc; i++) {
		char path[1024] = {0};
		parse_path(argv[i], path);
		mkdir(path);
	}
	return 0;
}

