#include <asm/asm.h>
#include <mmu.h>
#include <trap.h>

.text
LEAF(env_pop_tf)
.set reorder
.set at
	sll     a1, a1, 6
	mtc0    a1, CP0_ENTRYHI
	move    sp, a0
	j       ret_from_exception
END(env_pop_tf)

LEAF(enable_irq)
	li      t0, (STATUS_CU0 | STATUS_IM4 | STATUS_IEc) # 将t0的1、12、28位置1
	mtc0    t0, CP0_STATUS	# 将CP0的IEc、IM4、CU0位置1，开启全局终断使能和始终中断使能，并允许用户态使用cp0
	jr      ra	# 返回
END(enable_irq)
