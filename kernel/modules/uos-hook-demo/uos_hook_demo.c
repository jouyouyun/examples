#include <linux/binfmts.h>
#include <linux/file.h>
#include <linux/path.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/slab.h>
#include <linux/socket.h>

#include <linux/lsm_uos_hook_manager.h>

#define MODULE_NAME "uos_hook_demo"

struct hook_demo_entry {
	int hook_id;
	struct uos_hook_cb_entry cb;
};

static int is_target(const char *str)
{
	if (unlikely(!current) || unlikely(!current->comm)) {
		pr_info("The current invalid\n");
		return 0;
	}

	if (strstr(current->comm, str) != current->comm)
		return 0;

	return 1;
}

static char *get_absolute_path(const struct file *file) {
	struct path path;
	char *buf = NULL;
	char *tmp = NULL;
	char *filepath = NULL;
	int err = 0;

	buf = kzalloc(PATH_MAX, GFP_KERNEL);
	if (unlikely(buf == NULL))
		return NULL;

	tmp = d_absolute_path((const struct path *)&(file->f_path), buf, PATH_MAX);
	if (IS_ERR(tmp)) {
		goto error;
	}

	if (!d_is_symlink(file->f_path.dentry)) {
		goto ret;
	}

	err = kern_path(tmp, LOOKUP_FOLLOW, &path);
	if (err || unlikely(path.dentry == NULL)) {
		goto error;
	}

	memset(buf, 0, PATH_MAX);
	tmp = d_absolute_path((const struct path *)&path, buf, PATH_MAX);
	if (IS_ERR(tmp)) {
		goto error;
	}

 ret:
	filepath = kzalloc(PATH_MAX, GFP_KERNEL);
	if (likely(filepath)) {
		strcpy(filepath, tmp);
	}

 error:
	kfree(buf);
	buf = NULL;
	return filepath;
}

static int hook_mmap_file(struct file *file, unsigned long reqprot,
                          unsigned long prot, unsigned long flags) {
	if (!is_target("uhm_mf_")) {
		return 0;
	}

	pr_info("[%s] cmd: %s\n", __func__, current->comm);
	return -1;
}

static int hook_socket_create(int family, int type, int protocol, int kern) {
	if (!is_target("uhm_sc_")) {
		return 0;
	}

	pr_info("[%s] cmd: %s\n", __func__, current->comm);
	return -1;
}

static int hook_socket_listen(struct socket *sock, int backlog) {
	if (!is_target("uhm_sl_")) {
		return 0;
	}

	pr_info("[%s] cmd: %s\n", __func__, current->comm);
	return -1;
}

static void hook_bprm_committing_creds(struct linux_binprm *bprm)
{
	if (unlikely(!bprm) || unlikely(!bprm->buf) || unlikely(!bprm->filename) ||
	    unlikely(!bprm->interp))
		return;

	pr_info("[%s] buf: %s, filename: %s, interp: %s\n", __func__, bprm->buf,
		bprm->filename, bprm->interp);
}

static void hook_bprm_committed_creds(struct linux_binprm *bprm) {
	if (unlikely(!bprm) || unlikely(!bprm->buf) || unlikely(!bprm->filename) ||
	    unlikely(!bprm->interp))
		return;

	pr_info("[%s] buf: %s, filename: %s, interp: %s\n", __func__, bprm->buf,
		bprm->filename, bprm->interp);
}

static int hook_file_permission(struct file *file, int mask)
{
	if (unlikely(!file) || unlikely(!file->f_path.dentry) ||
	    unlikely(!file->f_path.dentry->d_name))
		return 0;

        path = get_absolute_path(file);
        if (unlikely(!path))
		return 0;

        pr_info("[%s] path: %s\n", __func__, path);
        kfree(path);

        return 0;
}

static int hook_file_open(struct file *file)
{
	char *path = NULL;

	if (unlikely(!file) || unlikely(!file->f_path.dentry) ||
	    unlikely(!file->f_path.dentry->d_name))
		return 0;

	path = get_absolute_path(file);
	if (unlikely(!path))
		return 0;

	pr_info("[%s] path: %s\n", __func__, path);
	kfree(path);

	return 0;
}

struct hook_demo_entry entries[] = {
    {
        .hook_id = UOS_MMAP_FILE,
        .cb =
            {
                .owner = MODULE_NAME,
                .cb_addr = (unsigned long)hook_mmap_file,
                .ret_type = UOS_HOOK_RET_TY_INT,
                .arg_len = 4,
            },
    },
    {
        .hook_id = UOS_SOCKET_CREATE,
        .cb =
            {
                .owner = MODULE_NAME,
                .cb_addr = (unsigned long)hook_socket_create,
                .ret_type = UOS_HOOK_RET_TY_INT,
                .arg_len = 4,
            },
    },
    {
        .hook_id = UOS_SOCKET_LISTEN,
        .cb =
            {
                .owner = MODULE_NAME,
                .cb_addr = (unsigned long)hook_socket_listen,
                .ret_type = UOS_HOOK_RET_TY_INT,
                .arg_len = 2,
            },
    },
    {
        .hook_id = UOS_BPRM_COMMITTING_CREDS,
        .cb =
            {
                .owner = MODULE_NAME,
                .cb_addr = (unsigned long)hook_bprm_committing_creds,
                .ret_type = UOS_HOOK_RET_TY_NONE,
                .arg_len = 1,
            },
    },
    {
        .hook_id = UOS_BPRM_COMMITTED_CREDS,
        .cb =
            {
                .owner = MODULE_NAME,
                .cb_addr = (unsigned long)hook_bprm_committed_creds,
                .ret_type = UOS_HOOK_RET_TY_NONE,
                .arg_len = 1,
            },
    },
    {
        .hook_id = UOS_FILE_PERMISSION,
        .cb =
            {
                .owner = MODULE_NAME,
                .cb_addr = (unsigned long)hook_file_permission,
                .ret_type = UOS_HOOK_RET_TY_INT,
                .arg_len = 2,
            },
    },
    {
        .hook_id = UOS_FILE_OPEN,
        .cb =
            {
                .owner = MODULE_NAME,
                .cb_addr = (unsigned long)hook_file_open,
                .ret_type = UOS_HOOK_RET_TY_INT,
                .arg_len = 1,
            },
    },
    {
        .hook_id = UOS_HOOK_NONE,
    },
};

static int registe_uos_hook(void)
{
	int i = 0, j = 0;
	int error = 0;

	for (; entries[i].hook_id != UOS_HOOK_NONE; i++) {
		error = uos_hook_register(entries[i].hook_id, &entries[i].cb);
		if (error) {
			pr_info("Failed to registe hook %d\n", i);
			break;
		}
	}

	if (entries[i].hook_id == UOS_HOOK_NONE)
		return 0;

	for (; j < i; j++) {
		error = uos_hook_cancel(entries[j].hook_id, entries[j].cb.owner);
		if (error)
			pr_info("Failed to cancel hook %d\n", j);
	}

	return -1;
}

static void cancel_uos_hook(void)
{
	int i = 0;
	int error = 0;

	for (; entries[i].hook_id != UOS_HOOK_NONE; i++) {
		error = uos_hook_cancel(entries[i].hook_id, entries[i].cb.owner);
		if (error)
			pr_info("Failed to cancel hook %d\n", i);
        }
}

int __init init_module(void)
{
	int error = 0;
	
	pr_info("start to init uos hook demo\n");
	error = registe_uos_hook();
	if (error) {
		pr_info("failed to registe hook\n");
		return -1;
	}
	
        pr_info("finish to init uos hook demo\n");
	return 0;
}

void __exit cleanup_module(void)
{
	cancel_uos_hook();
	pr_info("exit the uos hook demo\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jouyouyun");
MODULE_DESCRIPTION("The demo for uos lsm hook manager");
