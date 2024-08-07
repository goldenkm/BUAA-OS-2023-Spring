/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;
	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		u_int op_read = 0;
		u_int offset = begin + off;
		/* Exercise 5.3: Your code here. (1/2) */
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_ID, 4));
		//offset是相对于磁盘的位置，off是相对于begin的位置，应该用前者
		panic_on(syscall_write_dev(&offset, DEV_DISK_ADDRESS + DEV_DISK_OFFSET, 4));
		panic_on(syscall_write_dev(&op_read, DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION, 4));
		int status = 0;
		panic_on(syscall_read_dev(&status, DEV_DISK_ADDRESS + DEV_DISK_STATUS, 4));
		panic_on(status == 0);
		//memcpy(dst, (void *)(DEV_DISK_ADDRESS + DEV_DISK_BUFFER), DEV_DISK_BUFFER_LEN);
		panic_on(syscall_read_dev(dst + off, DEV_DISK_ADDRESS + DEV_DISK_BUFFER, DEV_DISK_BUFFER_LEN)); 
	}
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;
	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		u_int op_write = 1;
		u_int offset = begin + off;
		/* Exercise 5.3: Your code here. (2/2) */
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_ID, sizeof(diskno)));
                panic_on(syscall_write_dev(&offset, DEV_DISK_ADDRESS + DEV_DISK_OFFSET, sizeof(off)));
		//这行代码的位置很重要，务必先放到缓冲区再启动操作
		panic_on(syscall_write_dev(src + off, DEV_DISK_ADDRESS + DEV_DISK_BUFFER,  DEV_DISK_BUFFER_LEN));
                panic_on(syscall_write_dev(&op_write, DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION, sizeof(op_write)));
		int status;
                panic_on(syscall_read_dev(&status, DEV_DISK_ADDRESS + DEV_DISK_STATUS, sizeof(status)));
                panic_on((status == 0));
                //memcpy((void *)(DEV_DISK_ADDRESS + DEV_DISK_BUFFER), src, DEV_DISK_BUFFER_LEN);
		//panic_on(syscall_write_dev(src + off, DEV_DISK_ADDRESS + DEV_DISK_BUFFER,  DEV_DISK_BUFFER_LEN));
	}
}
