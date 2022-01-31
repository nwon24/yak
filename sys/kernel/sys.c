/*
 * sys.c
 * Misc. system calls.
 */
#include <asm/types.h>
#include <asm/paging.h>

#include <drivers/timer.h>

#include <generic/errno.h>

#include <kernel/proc.h>

uid_t
kernel_getuid(void)
{
	return current_process->uid;
}

uid_t
kernel_geteuid(void)
{
	return current_process->euid;
}

gid_t
kernel_getgid(void)
{
	return current_process->gid;
}

gid_t
kernel_getegid(void)
{
	return current_process->egid;
}

int
kernel_setuid(uid_t uid)
{
	if (!current_process->uid || !current_process->euid) {
		current_process->uid = uid;
		current_process->euid = uid;
		current_process->suid = uid;
	} else if (uid == current_process->uid || uid == current_process->suid) {
		current_process->euid = uid;
	} else {
		return -EPERM;
	}
	return 0;
}

int
kernel_setgid(gid_t gid)
{
	if (!current_process->uid || !current_process->euid) {
		current_process->gid = gid;
		current_process->egid = gid;
		current_process->sgid = gid;
	} else if (gid == current_process->gid || gid == current_process->sgid) {
		current_process->egid = gid;
	} else {
		return -EPERM;
	}
	return 0;
}

pid_t
kernel_setsid(void)
{
	if (current_process->pgrp_info.leader)
		return -EPERM;
	current_process->pgrp_info.leader = 1;
	current_process->pgrp_info.sid = current_process->pid;
	current_process->pgrp_info.pgid = current_process->pid;
	current_process->tty = -1;
	return current_process->pgrp_info.pgid;
}

pid_t
kernel_getsid(pid_t pid)
{
	struct process *proc;
	pid_t sid;

	if (pid == 0)
		sid = current_process->pgrp_info.sid;
	for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
		if (proc->state != PROC_EXITED && proc->pid == pid) {
			sid = proc->pgrp_info.sid;
			break;
		}
	}
	if (proc >= LAST_PROC)
		return -ESRCH;
	for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
		if (proc->state != PROC_EXITED && proc->pgrp_info.leader && proc->pgrp_info.sid == sid)
			return proc->pgrp_info.pgid;
	}
	return -ESRCH;
}

mode_t
kernel_umask(mode_t cmask)
{
	mode_t old;

	old = current_process->umask;
	current_process->umask = cmask & 0777;
	return old;
}

time_t
kernel_time(time_t *tloc)
{
	if (tloc != NULL) {
		if (!check_user_ptr(tloc))
			return -EFAULT;
		*tloc = CURRENT_TIME;
	}
	return CURRENT_TIME;
}

int
kernel_stime(time_t *tloc)
{
	if (tloc == NULL || !check_user_ptr(tloc))
		return -EFAULT;
	if (current_process->uid == 0 && current_process->euid == 0)
		startup_time = *tloc;
	else
		return -EPERM;
	return 0;
}

int
kernel_setpgid(pid_t pid, pid_t pgid)
{
	struct process *proc;

	if (pgid < 0)
		return -EINVAL;
	if (pid == 0)
		pid = current_process->pid;
	if (pgid == 0)
		pgid = pid;
	if (pid == current_process->pid) {
		if (current_process->pgrp_info.leader)
			return -EPERM;
		current_process->pgrp_info.pgid = pgid;
		return 0;
	}
	for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
		if (proc->pid == pid && proc->ppid == current_process->pid) {
			if (proc->pgrp_info.leader)
				return -EPERM;
			if (proc->pgrp_info.sid != current_process->pgrp_info.sid)
				return -EPERM;
			proc->pgrp_info.pgid = pgid;
			return 0;
		}
	}
	return -ESRCH;
}

pid_t
kernel_getpid(void)
{
	return current_process->pid;
}

pid_t
kernel_getppid(void)
{
	return current_process->ppid;
}

pid_t
kernel_getpgid(pid_t pid)
{
	struct process *proc;

	if (pid < 0)
		return -EINVAL;
	if (pid == 0)
		return current_process->pgrp_info.pgid;
	for (proc = FIRST_PROC; proc < LAST_PROC; proc++) {
		if (proc->pid == pid) {
			if (proc->pgrp_info.sid != current_process->pgrp_info.sid)
				return -EPERM;
			return proc->pgrp_info.pgid;
		}
	}
	return -ESRCH;
}

pid_t
kernel_getpgrp(void)
{
	return current_process->pgrp_info.pgid;
}
