#ifndef SYSCALL_H
#define SYSCALL_H

#ifndef __ASSEMBLER__

enum {
	SYS_putchar,
	SYS_print_cons,
	SYS_getenvid,
	SYS_yield,
	SYS_env_destroy,
	SYS_set_tlb_mod_entry,
	SYS_mem_alloc,
	SYS_mem_map,
	SYS_mem_unmap,
	SYS_exofork,
	SYS_set_env_status,
	SYS_set_trapframe,
	SYS_panic,
	SYS_ipc_try_send,
	SYS_ipc_recv,
	SYS_cgetc,
	SYS_write_dev,
	SYS_read_dev,
	SYS_change_dir,
	SYS_get_cur_path,
	SYS_set_shell_id,
	SYS_get_shell_id,
	SYS_get_env_var,
	SYS_set_env_var,
	SYS_unset_env_var,
	SYS_print_env_var,
	MAX_SYSNO,
};

#endif

#endif
