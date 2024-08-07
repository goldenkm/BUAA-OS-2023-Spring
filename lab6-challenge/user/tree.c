#include <lib.h>
#include <string.h>

int flag[256];
void tree(char *path, int level) {
	int fd, n;
	struct File f;
	if ((fd = open(path, O_RDONLY)) < 0) {
		user_panic("open %s: %d", path, fd);
	}
	while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
		//debugf("%s!\n", f.f_name);
		if (f.f_name[0]) {
			for (int i = 0; i < level; i++) {
				debugf("   ");
			}
			debugf("|--");
			if (f.f_type == FTYPE_REG) {
				debugf("%s\n", f.f_name);
			} else if (f.f_type == FTYPE_DIR) {
				debugf("%s\n", f.f_name);
				char newPath[1024];
				strcpy(newPath, path);
				strcat(newPath, "/");
				strcat(newPath, f.f_name);
				tree(newPath, level+1);
			}
		}
	}
}

void usage() {
	printf("usage: ls [-dFl] [file...]\n");
	exit();
}

int main(int argc, char **argv) {
	//debugf("%d!\n", argc);
	ARGBEGIN {
		default:
			usage();
		case 'd':
		case 'F':
		case 'l':
			flag[(u_char)ARGC()]++;
			break;
	}
	ARGEND
	if (argc == 0) {
		char path[1024] = {0};
		syscall_get_cur_path(path);
		tree(path, 0);
	} else {
		for (int i = 0; i < argc; i++) {
			char path[1024] = {0};
			parse_path(argv[i], path);
			tree(path, 0);
		}
	}
	debugf("\n");
	return 0;
}
