#include <lib.h>
#include <fs.h>
#include <fsreq.h>

int flag[256];

void touch(char *path) {
    int r;
    char realPath[1024] = {0};
    if (path[0] != '/') {
        realPath[0] = '/';
    }
    strcat(realPath, path);
    r = create(realPath, FTYPE_REG);
    if (r < 0) {
        debugf("touch fail\n");
    }
}

void usage(void) {
	printf("usage: touch [-p] [file...]\n");
	exit();
}

int main(int argc, char **argv) {
	int i;

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
		touch(path);
	}
	return 0;
}