#include <lib.h>
#include <fs.h>

int flag[256];

int is_dir(int fd) {
	int r;
    if ((r = get_type(fd)) < 0) {
        user_panic("cannot find the file!");
    }
	return r == FTYPE_DIR;
}

void query(char *path) {
    int r;
    if (flag['i']) {
        printf("delete the file?: %s [y/n] ", path);
        char c;
        if ((r = read(0, &c, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		    }
            printf("\n");
            if (c != 'y' && c != 'Y') {
                return;
            }
    }
}

void rm(char *path) {
    int r, fd, n;
    struct File f;
    if ((fd = open(path, O_RDONLY)) < 0) {
        user_panic("no such file or directory!");
    }
    if ((r = get_type(fd)) == FTYPE_DIR) {
        if (flag['r']) {
            while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
                char new_path[1024] = {0};
                strcpy(new_path, path);
                if (f.f_name[0]) {
                    strcat(new_path, "/");
                    strcat(new_path, f.f_name);
                    if (f.f_type == FTYPE_REG) {
                        query(new_path);
                        remove(new_path);
                    } else {
                        rm(new_path);
                    }
                }
            }
            query(path);
            remove(path);
        } else {
            user_panic("cannot remove: it is a directory");
        }
    } else {
        query(path);
        remove(path);
    }
}

void usage() {
    printf("usage: rm [-选项] [file...]\n");
	exit();
}

int main(int argc, char **argv) {
	ARGBEGIN {
	case 'r':
    case 'i':
    case 'f':
		flag[(u_char)ARGC()]++;
		break;
    default:
		usage();
        break;
	}
	ARGEND
	for (int i = 0; i < argc; i++) {
		char path[1024] = {0};
        parse_path(argv[i], path);
		rm(path);
	}
	return 0;
}