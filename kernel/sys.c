/*
 * sys.c
 * Misc. system calls.
 */
#include <asm/types.h>

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
