#include <args.h>
#include <lib.h>
#include <fs.h>
#include <env.h>
#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"
//test
/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */


//内部命令直接写在这个文件里
void chdir(char *path) {
	char new_path[1024] = {0};
	parse_path(path, new_path);
	int fd;
	if ((fd = open(new_path, O_RDONLY)) < 0) {
		printf("no such file or directory!\n");
		return;
	}
	syscall_change_dir(new_path);
}

void getcwd() {
	char cur_path[1024] = {0};
	syscall_get_cur_path(cur_path);
	printf("%s\n", cur_path);
}

int _gettoken(char *s, char **p1, char **p2) {
	//p1是字符串的首端，p2是字符串的尾端
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}
	
	//实现引号支持
	if (*s == '\"') {
		*p1 = ++s;
		while ((*s != '\"' || *(s-1) == '\\') && (*s != '\"' || *(s-1) != '\\' || *(s-2) != '\\')) {
			s++;
		}
		*s++ = 0;	//必须要封好字符串
		*p2 = s;
		return 'w';
	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		if (*(s + 1) == '>') {
			t = '>>';
			s++;
		}
		*s++ = 0;
		*p2 = s;
		return t;
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

void remove_slash(char *t) {
	char tmp[128] = {0};
	int length = 0;
	for (int i = 0; t[i]; i++) {
		int flag = (t[i+1] == '\'') || (t[i+1] == '\"');
		if (t[i] == '\\' && t[i+1] == '\\') {
			tmp[length++] = '\\';
			i++;
		} else if (t[i] == '\\' && flag) {
			continue;
		} else {
			tmp[length++] = t[i];
		}
	}
	tmp[length] = '\0';
	strcpy(t, tmp);
}

int parsecmd(char **argv, int *rightpipe) {
	int argc = 0;
	while (1) {
		char *t;
		int fd, r, child;
		int c = gettoken(0, &t);
		switch (c) {
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			remove_slash(t);
			argv[argc++] = t;
			break;
		case ';':	//lab6-challenge
			child = fork();
			*rightpipe = child;
			//重点在于让父亲执行分号右边的指令。
			if (child > 0) {
				wait(child);
				return parsecmd(argv, rightpipe);
			} else if (child == 0) {
				return argc;
			} else {
				user_panic("fork error!\n");
			}
			break;
		case '&': 
			child = fork();
			//*rightpipe = child;
			if (child == 0) {
				return argc;
			} else if (child > 0) {
				return parsecmd(argv, 0);
			} else {
				user_panic("fork error!\n");
			}
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
			r = open(t, O_RDONLY);
			if (r < 0) {
				user_panic("open fail!");
			}
			fd = r;
			r = dup(fd, 0);
			if (r < 0) {
				user_panic("dup fail!");
			}
			r = close(fd);
			if (r < 0) {
				user_panic("close fail!");
			}
			//user_panic("< redirection not implemented");

			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
			r = open(t, O_WRONLY | O_CREAT);
	        if (r < 0) {
                user_panic("open fail!");
            }      
		    fd = r;	
			r = dup(fd, 1);
            if (r < 0) {
                user_panic("dup fail!");
            }       
            r = close(fd);
            if (r < 0) {
				user_panic("close fail!");
            }       
			//user_panic("> redirection not implemented");

			break;
		case '>>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: >> not followed by word\n");
				exit();
			}
			if ((fd = open(t, O_WRONLY | O_CREAT)) < 0) {
          		user_panic("open fail!");
     		}      
			int size;
			if ((size = get_size(fd)) < 0) {
				user_panic("get size fail!");
			}
			if ((r = seek(fd, size))) {
				user_panic("seek fail!");
			}
            if ((r = dup(fd, 1)) < 0) {
                user_panic("dup fail!");
            }       
       		if ((r = close(fd)) < 0) {
				user_panic("close fail!");
        	}    
			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			if ((r = pipe(p)) < 0) {
				user_panic("pipe alloc fail!");
			}
			child = fork();
			*rightpipe = child;
			if (child == 0) {
				if ((r = dup(p[0], 0)) < 0) {
					user_panic("| dup fail!");
				}
				if ((r = close(p[0])) < 0) {
					user_panic("| close fail!");
				}
				if ((r = close(p[1])) < 0) {
					user_panic("| close fail!");
				}
				return parsecmd(argv, rightpipe);
			} else if (child > 0) {
				if ((r = dup(p[1], 1)) < 0) {
					user_panic("| dup fail!");
				}
				if ((r = close(p[1])) < 0) {
					user_panic("| close fail!");
				}
				if ((r = close(p[0])) < 0) {
					user_panic("| close fail!");
				}
				return argc;
			} else {
				user_panic("fork error!\n");
			}
			user_panic("| not implemented");

			break;
		}
	}

	return argc;
}

void runcmd(char *s) {
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;
	int child = spawn(argv[0], argv);
	close_all();
	if (child >= 0) {
		wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

char cmd[1024];
int pointer;
int his_pointer; //历史记录的指针
int his_num; //历史记录的数目
char instructions[50][10] = {"cat", "cd", "history", "num", "touch", "mkdir", "declare", "rm", "ls", "echo", "sh", "tree", "unset", "halt"};
void readline(char *buf, u_int n) {
	int r;
	//memset(buf, 0, sizeof(buf));
	memset(cmd, 0, sizeof(cmd));
	pointer = 0;
	for (int i = 0; i < n; i++) {
		if ((r = read(0, buf + i, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		//debugf("%d", buf[i]);
		/***********读入方向键***********/
		if (buf[i] == 27) {
			if ((r = read(0, buf + i + 1, 1)) != 1) {
				if (r < 0) {
					debugf("read error: %d\n", r);
				}
				exit();
			}
			if (buf[i+1] == 91) {
				if ((r = read(0, buf + i + 2, 1)) != 1) {
					if (r < 0) {
						debugf("read error: %d\n", r);
					}
					exit();
				}	
				if (buf[i+2] == 68) { //left
					if (pointer > 0) { //注意左右移动光标需要有个极限
						pointer--;
					} else {
						printf(" ");
					}
					continue;
				} else if (buf[i+2] == 67) { //right
					if (cmd[pointer] != '\0') {
						pointer++;
					} else {
						printf("\b");
					}
					continue;
				} else if (buf[i+2] == 65) { //up
					printf("%c%c%c", 27, 91, 66);
					if (his_pointer == 0) {
						//debugf("no more history upside!\n");
						continue;
					}
					his_pointer--;
					char tmp[128 * 128] = {0};
					int fd, count = 0, length = 0;
					if ((fd =  open("/.history", O_RDWR)) < 0) {
						user_panic("open fail!");
					}
					//保存当前的指令
					cmd[pointer] = '\0';
					//读历史指令
					if ((r = read(fd, tmp, sizeof tmp)) < 0) {
						user_panic("read .history fail!");
					}
					close(fd);
					//存进二维数组
					char records[100][1024] = {0};
					for (int j = 0; tmp[j]; j++) {
						if (tmp[j] != '\n') {
							records[count][length++] = tmp[j];
						} else {
							records[count][length] = '\0';
							count++;
							length = 0;
						}
					}
					strcpy(cmd, records[his_pointer]);
					//清除当前控制台的输出
					for (int j = 0; j < pointer; j++) {
						printf("\b \b");
					}
					pointer = strlen(cmd);
					printf("%s", cmd);
					continue;
				} else if (buf[i+2] == 66) { //down
					his_pointer++;
					if (his_pointer == his_num) {
						memset(cmd, 0, sizeof cmd);
					}
					if (his_pointer == his_num + 1) {
						continue;
					}
					char tmp[128 * 128] = {0};
					int fd, count = 0, length = 0;
					if ((fd =  open("/.history", O_RDWR)) < 0) {
						user_panic("open fail!");
					}
					//保存当前的指令
					cmd[pointer] = '\0';
					//读历史指令
					if ((r = read(fd, tmp, sizeof tmp)) < 0) {
						user_panic("read .history fail!");
					}
					close(fd);
					//存进二维数组
					char records[100][1024] = {0};
					for (int j = 0; tmp[j]; j++) {
						if (tmp[j] != '\n') {
							records[count][length++] = tmp[j];
						} else {
							records[count][length] = '\0';
							count++;
							length = 0;
						}
					}
					strcpy(cmd, records[his_pointer]);
					//清除当前控制台的输出
					for (int j = 0; j < pointer; j++) {
						printf("\b \b");
					}
					pointer = strlen(cmd);
					printf("%s", cmd);
					continue;
				}
			}
		}
		/***********读入删除键***********/
		if (buf[i] == 127) { //实现删除功能
			char tmp[1024] = {0};
			if (pointer == 0) { //不能无止境的删除
				continue;
			}
			printf("\b ");
			int length = 0;
			for (int j = pointer; j < n; j++) {
				if (cmd[j] == 0) {
					break;
				}
				tmp[length++] = cmd[j];
				printf(" ");
			}
			tmp[length] = '\0';
			for (int j = 0; j <= length; j++) {
				printf("\b");
			}
			cmd[--pointer] = '\0';
			strcat(cmd, tmp);
			for (int j = 0; j < length; j++) {
				printf("%c", tmp[j]);
			}
			for (int j = 0; j < length; j++) {
				printf("\b");
			}
			continue;
		}
		if (buf[i] == '\b') {
			if (i > 0) {
				i -= 2;
			} else {
				i = -1;
			}
			if (buf[i] != '\b') {
				printf("\b");
			}
			continue;
		}
		/***********读入tab键***********/
		if (buf[i] == '\t') {
			int space_cnt = 8 - (14+pointer) % 8;
			for (int j = 0; j < space_cnt; j++) {
				printf("\b");
			}
			int space_idx = -1, length = 0;
			int flag = 0;	//记录是否重复找到
			for (int j = 0; j < pointer; j++) {
				if (cmd[j] == ' ') {
					space_idx = j;
				}
			}
			char prefix[128] = {0}, tmp[128] = {0};
			if (space_idx > 0) {
				for (int j = space_idx + 1; j < pointer; j++) {
					prefix[length++] = cmd[j];
				}
				prefix[length] = 0;
			} else {
				strcpy(prefix, cmd);
			}
			//检查现有的指令
			for (int j = 0; instructions[j][0]; j++) {
				if (is_prefix(instructions[j], prefix)) {
					if (flag) {		//如果找到两个，就不补全（阉割版
						flag++;
						continue;
					}
					flag++;
					strcpy(tmp, instructions[j]);
				}
			}
			//检查当前工作路径下的文件
			char cur_path[1024] = {0};
			syscall_get_cur_path(cur_path);
			struct File f;
			int fd;
			if ((fd = open(cur_path, O_RDONLY)) < 0) {
				user_panic("cannot open the file!");
			}
			while ((r = readn(fd, &f, sizeof f)) == sizeof f) {
				//debugf("%s!\n", f.f_name);
				if (f.f_name[0]) {
					if (is_prefix(f.f_name, prefix)) {
						if (flag) {
							flag++;
							continue;
						}
						flag++;
						strcpy(tmp, f.f_name);
					}
				}
			}
			//没找到或找到不止一个
			if (!tmp[0] || flag > 1) {
				continue;
			}
			//找到了
			int start = space_idx > 0 ? space_idx : 0;
			for (int j = start + 1; j < pointer; j++) {
				printf("\b \b");
			}
			printf("%s", tmp);
			cmd[space_idx + 1] = '\0';
			strcat(cmd, tmp);
			pointer = strlen(cmd);
			continue;
		}
		/***********读入回车键***********/
		if (buf[i] == '\r' || buf[i] == '\n') {
			buf[i] = 0;
			//存进历史记录里
			int fd;
			if ((fd = open("/.history", O_WRONLY)) < 0) {
				user_panic("open .history fail!");
			}
			int size;
			if ((size = get_size(fd)) < 0) {
				user_panic("get size fail!");
			}
			if ((r = seek(fd, size))) {
				user_panic("seek fail!");
			}
			if ((r = write(fd, cmd, strlen(cmd))) < 0) {
				user_panic("write .history fail!");
			}
			if ((r = write(fd, "\n", 1)) < 0) {
				user_panic("write .history fail!");
			}
			close(fd);
			his_num++;
			his_pointer = his_num; //每次读入回车都让指针指向最后一个
			return;
		}
		/***********正常读入字符***********/
		if (cmd[pointer] != 0) {
			char tmp[1024] = {0};
			int length = 0;
			for (int j = pointer; j < n; j++) {
				if (cmd[j] == 0) {
					break;
				}
				tmp[length++] = cmd[j];
				printf(" ");
			}
			tmp[length] = '\0';
			for (int j = 0; j < length; j++) {
				printf("\b");
			}
			cmd[pointer++] = buf[i];
			cmd[pointer] = '\0';
			strcat(cmd, tmp);
			for (int j = 0; j < length; j++) {
				printf("%c", tmp[j]);
			}
			for (int j = 0; j < length; j++) {
				printf("\b");
			}
		} else {
			cmd[pointer++] = buf[i];
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

void history_init() {
    int r;
	if ((r = open("/.history", O_RDONLY)) >= 0) {
		return;
	}
    if ((r = create("/.history", FTYPE_REG)) < 0) {
        debugf("create .history fail\n");
    } else {
		debugf(".history is created\n");
	}
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	history_init();
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     JkmOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	syscall_set_shell_id();
	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			char cur_path[1024] = {0};
			syscall_get_cur_path(cur_path);
			printf("\033[1;32m""\n@21373035:");
			printf("\033[1;34m%s\033[0m", cur_path);
			printf(" $ ");
		}
		readline(buf, sizeof buf);
		//debugf("%s!!\n", cmd);
		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		/*********** 内部命令cd ***********/
		if (cmd[0] == 'c' && cmd[1] == 'd') {
			char path[MAXPATHLEN] = {0};
			int i;
			for (i = 0; cmd[i]; i++) {
				path[i] = cmd[i + 3];
			}
			path[i] = '\0';
			chdir(path);
			continue;
		}
		/************* 内部命令pwd *************/
		if (cmd[0] == 'p' && cmd[1] == 'w' && cmd[2] == 'd') {
			getcwd();
			continue;
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			runcmd(cmd);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}
