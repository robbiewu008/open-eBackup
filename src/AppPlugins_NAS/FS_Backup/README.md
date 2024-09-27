# FS_Backup C接口文件集备份说明

> Backup C接口可以用于执行文件集备份/NFS防篡改，本篇只介绍文件集备份相关的接口

Backup的任务类型有四种，分别定义如下，分别对应Scanner生成的四种控制文件（`control_xxxxxxxx.txt`，`delete_xxxxxxxx.txt`，`hardlink_xxxxxxxx.txt`，`mtime_xxxxxxxxx.txt`）：
```c
const int BACKUP_PHASE_COPY = 1;
const int BACKUP_PHASE_DELETE = 2;
const int BACKUP_PHASE_HARDLINK = 3;
const int BACKUP_PHASE_DIR = 4;
```
文件集备份如果没有删除、硬链接，且不涉及修改目录的Mtime信息，则只会用到`BACKUP_PHASE_COPY`阶段。

Backup任务执行的流程为：
1. `InitLog(const char* fullLogPath, int logLevel)`初始化日志。配置日志输出目录和日志等级
2. `CreateBackupInst(const char* source, const char* destination, const char* metaPath, int phase, bool writeMeta)`创建Backup实例子，成功返回`void*`失败返回`NULL`。配置Backup的源端目录路和目标端目录路径，`metaPath`为Scanner生成的meta文件所在的目录，`phase`描述Backup的任务类型。这里描述源端和目标端而不是生产端和副本，因为恢复场景可以看成备份的逆向过程：源端副本，目标端为生产端。
3. 创建完成Backup实例后可以用`ConfigureThreadPool(void* backupHandle, int readThreadNum, int writeThreadNum)`配置线程池线程数，再用`ConfigureMemory(void* backupHandle, int maxMemory)`配置内存大小
4. `Enqueue(void* backupHandle, const char* backupControlFile)`将Scanner生产的控制文件路径传入Backup实例，控制文件类型必须和Backup实例类型一致
5. 上述配置完成后用过`Start(void* backupHandle)`异步启动backup，成功返回0，失败返回-1
6. Backup启动成功后，用`GetStatus(void* backupHandle)`监控Backup状态，成功结束返回1，执行中返回2，失败返回3。同时可以用`GetStats(void* backupHandle, BackupStatistics* backupStats)`获取Backup统计信息
7. backup实际创建成功后，任务无论失败或者成功，都要保证`DestroyBackupInst(void* backupHandle)`执行，防止内存泄漏

Backup统计信息，期中数据大小以字节为单位：
```c
typedef struct BackupStats_S {
    uint64_t noOfDirToBackup    = 0;        /* Number of directories to be backed up */
    uint64_t noOfFilesToBackup  = 0;        /* Number of files to be backed up */
    uint64_t noOfBytesToBackup  = 0;        /* Number of bytes (in KB) to be backed up */
    uint64_t noOfDirCopied      = 0;        /* Number of directories copied */
    uint64_t noOfFilesCopied    = 0;        /* Number of files copied */
    uint64_t noOfBytesCopied    = 0;        /* Number of bytes (in KB) copied */
    uint64_t noOfDirFailed      = 0;        /* Number of directories copy failed */
    uint64_t noOfFilesFailed    = 0;        /* Number of files copy failed */
    uint64_t backupspeed        = 0;        /* Backup speed (in KBps) */
} BackupStatistics;
```

> Enqueue的控制文件路径必须是绝对路径，且是平台对应的标准路径