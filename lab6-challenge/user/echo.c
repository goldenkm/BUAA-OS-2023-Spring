#include <lib.h>

int main(int argc, char **argv) {
	int i, j, nflag, r;

	nflag = 0;
	if (argc > 1 && strcmp(argv[1], "-n") == 0) {
		nflag = 1;
		argc--;
		argv++;
	}
	for (i = 1; i < argc; i++) {
		if (i > 1) {
			printf(" ");
		}
		if (argv[i][0] == '$') {
			char key[128], value[128];
			for (j = 1; argv[i][j]; j++) {
				key[j-1] = argv[i][j];
			}
			key[j-1] = '\0';
			if ((r = syscall_get_env_var(key, value)) < 0) {
				user_panic("get environment variable fail!");
			}
			printf("%s", value);
			continue;
		}
		printf("%s", argv[i]);
	}
	if (!nflag) {
		printf("\n");
	}
	return 0;
}
