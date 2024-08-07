#include <lib.h>
#include <error.h>

void unset(char *name) {
    int r;
    if ((r = syscall_unset_env_var(name)) < 0) {
        if (r == -E_BAD_ENV_VAR) {
            user_panic("cannot unset: variable is read only");
        } else if (r == -E_ENV_VAR_NOT_FOUND) {
            user_panic("cannot find the variable");
        }
    }
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        unset(argv[i]);
    }
}