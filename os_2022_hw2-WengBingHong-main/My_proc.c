/*
 * procfs3.c
 */
#define DEBUG

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <linux/minmax.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_MAX_SIZE 2048UL
#define PROCFS_ENTRY_FILENAME "thread_info"

static struct proc_dir_entry *our_proc_file;
static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

static unsigned long tid[32];
static int pid[32];
static int t_nums = -1;
static int read_count = 0;
static long long utime[32];
static long context_switch_time[32];

int p_id;
struct pid *pid_struct;
struct task_struct *task;

static ssize_t procfs_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
    char time_infos[100];
    if (procfs_buffer_size == 0) {  // offset == 0 prevent initial read
        pr_debug("procfs_read: END\n");
        *offset = 0;
        return 0;
    }
    pr_debug("procfs_read\n");
    p_id = task_pid_nr(current);
    pr_debug("kernel get pid = %d\n", p_id);

    procfs_buffer_size = min(procfs_buffer_size, length);
    pr_debug("procfs_buffer_size = %ld\n", procfs_buffer_size);
    pr_debug("sizeof(procfs_buffer) = %ld\n", sizeof(procfs_buffer));
    pr_debug("length = %ld\n", length);

    sprintf(time_infos, "ThreadID:%d Time:%lld(ms) context switch times:%ld", pid[read_count], utime[read_count], context_switch_time[read_count]);
    read_count++;
    if (copy_to_user(buffer, time_infos, 100))
        return -EFAULT;
    *offset += 100;
    pr_debug("user get %s\n", time_infos);
    pr_debug("-----------------------------------------------");

    t_nums = -1;
    return *offset;
}
static ssize_t procfs_write(struct file *file, const char __user *buffer, size_t len, loff_t *off) {
    procfs_buffer_size = min(PROCFS_MAX_SIZE, len);
    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
        return -EFAULT;
    *off += procfs_buffer_size;

    pr_debug("procfs_write: write %lu bytes \"%s\"\n", procfs_buffer_size, procfs_buffer);  // I can get id here // write to kernel buffer? // procfs_buffer??
    pr_debug("off is %lld\n", *off);

    t_nums++;
    kstrtoul(procfs_buffer, 10, &tid[t_nums]);  // store id into array, use k str to ul
    pr_debug("tid[%d] is %lu\n", t_nums, tid[t_nums]);
    pid[t_nums] = current->pid;
    pr_debug("current->pid = %d\n", pid[t_nums]);

    p_id = task_pid_nr(current);
    pr_debug("kernel get pid = %d\n", p_id);

    pid_struct = find_get_pid(p_id);
    if (pid_struct == NULL) {
        pr_debug("Error: Could not find_get_pid\n");
        return -EFAULT;
    }
    task = pid_task(pid_struct, PIDTYPE_PID);
    utime[t_nums] = task->utime / 1000 / 1000;  // convert microsecond to milliseconds
    context_switch_time[t_nums] = task->nvcsw + task->nivcsw;
    pr_debug("name is \"%s\"\n", task->comm);
    pr_debug("utime is \"%lld\"\n", utime[t_nums]);
    pr_debug("stime is \"%lld\"\n", task->stime);
    pr_debug("nvcsw is \"%ld\"\n", task->nvcsw);
    pr_debug("nivcsw is \"%ld\"\n", task->nivcsw);
    pr_debug("context switch time is \"%ld\"\n", context_switch_time[t_nums]);
    pr_debug("-----------------------------------------------");

    read_count = 0;
    return procfs_buffer_size;
}
static int procfs_open(struct inode *inode, struct file *file) {
    try_module_get(THIS_MODULE);
    return 0;
}
static int procfs_close(struct inode *inode, struct file *file) {
    module_put(THIS_MODULE);
    return 0;
}

#ifdef HAVE_PROC_OPS
static struct proc_ops file_ops_4_our_proc_file = {
    .proc_read = procfs_read,
    .proc_write = procfs_write,
    .proc_open = procfs_open,
    .proc_release = procfs_close,
};
#else
static const struct file_operations file_ops_4_our_proc_file = {
    .read = procfs_read,
    .write = procfs_write,
    .open = procfs_open,
    .release = procfs_close,
};
#endif

static int __init procfs3_init(void) {
    our_proc_file = proc_create(PROCFS_ENTRY_FILENAME, 0666, NULL, &file_ops_4_our_proc_file);
    if (our_proc_file == NULL) {
        remove_proc_entry(PROCFS_ENTRY_FILENAME, NULL);
        pr_debug("Error: Could not initialize /proc/%s\n", PROCFS_ENTRY_FILENAME);
        return -ENOMEM;
    }
    proc_set_size(our_proc_file, 80);
    proc_set_user(our_proc_file, GLOBAL_ROOT_UID, GLOBAL_ROOT_GID);

    pr_debug("/proc/%s created\n", PROCFS_ENTRY_FILENAME);
    pr_debug("-----------------------------------------------");
    return 0;
}

static void __exit procfs3_exit(void) {
    remove_proc_entry(PROCFS_ENTRY_FILENAME, NULL);
    pr_debug("/proc/%s removed\n", PROCFS_ENTRY_FILENAME);
}

module_init(procfs3_init);
module_exit(procfs3_exit);

MODULE_LICENSE("GPL");
