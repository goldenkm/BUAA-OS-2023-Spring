#include <lib.h>

char buf[8192];

void cat(int f, char *s) {
	long n;
	int r;

	while ((n = read(f, buf, (long)sizeof buf)) > 0) {
		if ((r = write(1, buf, n)) != n) {
			user_panic("write error copying %s: %d", s, r);
		}
	}
	if (n < 0) {
		user_panic("error reading %s: %d", s, n);
	}
}

int main(int argc, char **argv) {
	int f, i;
	if (argc == 1) {
		cat(0, "<stdin>");
	} else {
		for (i = 1; i < argc; i++) {
			char path[1024] = {0};
			parse_path(argv[i], path);
			f = open(path, O_RDONLY);
			if (f < 0) {
				user_panic("can't open %s: %d", path, f);
			} else {
				cat(f, path);
				close(f);
			}
		}
	}
	return 0;
}
