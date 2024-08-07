#include <lib.h>
#include <fs.h>
#include <fsreq.h>

int flag[256];

void history() {
    int fd, r, length = 0, count = 0;
	char tmp[128 * 128] = {0};
	if ((fd = open("/.history", O_RDONLY | O_CREAT)) < 0) {
		user_panic("open .history fail!");
	}
	if ((r = read(fd, tmp, sizeof tmp)) < 0) {
		user_panic("read .history fail!");
	}
	close(fd);
	char record[MAXPATHLEN] = {0};
	for (int j = 0; tmp[j]; j++) {
		if (tmp[j] != '\n') {
			record[length++] = tmp[j];
		} else {
			record[length] = '\0';
			printf("%d %s\n", count, record);
			count++;
			length = 0;
			memset(record, 0, sizeof record);
		}
	}
}

void usage(void) {
	printf("usage: history [-p] [file...]\n");
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
	history();
	printf("\n");
	return 0;
}