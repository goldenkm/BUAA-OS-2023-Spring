#include <lib.h>

int flag[256];

void split(char *s, char *key, char *value) {	//split by '='
	int i, length = 0;
	char tmp[128] = {0};
	for (i = 0; s[i] != '='; i++) {
		tmp[length++] = s[i];
	}
	tmp[length] = '\0';
	strcpy(key, tmp);
	i++;
	length = 0;
	memset(tmp, 0, sizeof(tmp));
	while (s[i]) {
		tmp[length++] = s[i];
		i++;
	}
	tmp[length] = '\0';
	strcpy(value, tmp);
}

void declare(char *key, char *value) {
	int read_only = flag['r'];
	int global = flag['x'];
	int r;
	if ((r = syscall_set_env_var(key, value, global, read_only)) < 0) {
		user_panic("declare environment variable fail!");
	}
}

void usage(void) {
	printf("usage: declare [-xr] [NAME [=VALUE]]\n");
	exit();
}

int main(int argc, char **argv) {
	int i;

	ARGBEGIN {
	default:
		usage();
	case 'x':
    case 'r':
		flag[(u_char)ARGC()]++;
		break;
	}
	ARGEND
	if (argc == 0) {
		syscall_print_env_var();
		return 0;
	}
	for (i = 0; i < argc; i++) {
		char key[128];
		char value[128];
		split(argv[i], key, value);
		declare(key, value);
	}
	return 0;
}