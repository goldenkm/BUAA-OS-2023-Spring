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
        printf("cp: overwrite the file?: %s [y/n] ", path);
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

int copy(int srcfd, int dstfd) {
	int r;
    char tmp[128 * 128] = {0};
    if ((r = read(srcfd, tmp, sizeof tmp)) < 0) {
        user_panic("cannot read the file");
    }
    if ((r = write(dstfd, tmp, sizeof tmp)) < 0) {
        user_panic("cannot write the file");
    }
}

int must_dir;
void cp(char *src, char *dst) {
    int r, newfd, oldfd, n;
    struct File f;
    if ((oldfd = open(src, O_RDONLY)) < 0) {
        user_panic("cannot open the file!");
    }
    int cpdir = is_dir(oldfd);
    if (cpdir) {
        if ((newfd = open(dst, O_RDWR | O_MKDIR)) < 0) {
            user_panic("cannot open the file!");
        }
        if (must_dir && !is_dir(newfd)) {
            user_panic("%s is not a directory!", dst);
        }
        if (flag['r']) {
            query(dst);
            copy(oldfd, newfd);
            while ((n = readn(oldfd, &f, sizeof f)) == sizeof f) {
                char new_src[1024] = {0}, new_dst[1024] = {0};
                strcpy(new_src, src);
                strcpy(new_dst, dst);
                if (f.f_name[0]) {
                    strcat(new_src, "/");
                    strcat(new_src, f.f_name);
                    strcat(new_dst, "/");
                    strcat(new_dst, f.f_name);
                    if (f.f_type == FTYPE_REG) {
                        query(new_dst);
                        copy(oldfd, newfd);
                    } else {
                        cp(new_src, new_dst);
                    }
                }
            }
        } else {
            user_panic("without -r; pass the directory \'%s\'", src);
        }
    } else {
        if ((newfd = open(dst, O_RDWR | O_CREAT)) < 0) {
            user_panic("cannot open the file!");
        }
        if (must_dir && !is_dir(newfd)) {
            user_panic("%s is not a directory!", dst);
        }
        query(dst);
        copy(oldfd, newfd);
    }
    close(oldfd);
    close(newfd);
}

void usage() {
    printf("usage: cp [-选项] [file...]\n");
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
    if (argc == 2) {
        char src[1024] = {0};
        parse_path(argv[0], src);
        char dst[1024] = {0};
        parse_path(argv[1], dst);
        must_dir = 0;
        cp(src, dst);
    } else if (argc > 2) {
        char *dst;
        parse_path(argv[argc - 1], dst);
        for (int i = 0; i < argc - 1; i++) {
            char src[1024] = {0};
            parse_path(argv[i], src);
            must_dir = 1;
            cp(src, dst);
        }
    }
	return 0;
}