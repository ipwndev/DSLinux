void do_bad_area(struct task_struct *tsk, struct mm_struct *mm,
		 unsigned long addr, unsigned int fsr, struct pt_regs *regs);
#ifdef CONFIG_MMU
void show_pte(struct mm_struct *mm, unsigned long addr);
#endif

unsigned long search_exception_table(unsigned long addr);
