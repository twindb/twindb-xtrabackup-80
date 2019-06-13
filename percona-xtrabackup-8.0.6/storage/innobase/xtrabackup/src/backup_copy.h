
#ifndef XTRABACKUP_BACKUP_COPY_H
#define XTRABACKUP_BACKUP_COPY_H

#include "datasink.h"
#include "xtrabackup_config.h"

struct Backup_context;

/* special files */
#define XTRABACKUP_SLAVE_INFO "xtrabackup_slave_info"
#define XTRABACKUP_GALERA_INFO "xtrabackup_galera_info"
#define XTRABACKUP_BINLOG_INFO "xtrabackup_binlog_info"
#define XTRABACKUP_INFO "xtrabackup_info"

bool backup_file_print(const char *filename, const char *message, int len);

bool backup_file_printf(const char *filename, const char *fmt, ...)
    __attribute__((format(printf, 2, 0)));

/************************************************************************
Return true if first and second arguments are the same path. */
bool equal_paths(const char *first, const char *second);

/************************************************************************
Copy file for backup/restore.
@return true in case of success. */
bool copy_file(ds_ctxt_t *datasink, const char *src_file_path,
               const char *dst_file_path, uint thread_n, ssize_t pos = -1);

/* Backup non-InnoDB data.
@return true if success. */
bool backup_start(Backup_context &context);

/* Finsh the backup. Release all locks. Write down backup metadata.
@return true if success. */
bool backup_finish(Backup_context &context);

bool apply_log_finish();
bool copy_back(int argc, char **argv);
bool decrypt_decompress();
#ifdef HAVE_VERSION_CHECK
void version_check();
#endif
bool is_path_separator(char);
bool directory_exists(const char *dir, bool create);

#endif
