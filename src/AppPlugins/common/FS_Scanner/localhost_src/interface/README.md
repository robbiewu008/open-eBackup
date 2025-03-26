# Scanner C接口使用说明

```c
typedef struct ScanConf_S {
    char *jobId;                    /* Job id */
    char *metaPath;                 /* Metadata path for cache files of nas share */
    char *metaPathForCtrlFiles;     /* Metadata path for control files of nas share */
    bool enableProduce = true;      /* won't produce meta and control file once disabled */
} ScanConf;
```
`jobId`传入任务的UUID，`metaPath`传入生成meta文件的目录路径，`metaPathForCtrlFiles`传入生成控制文件的目录路径。`enableProduce`默认为`true`，设施为`false`时不会生成控制文件和meta文件（用于给某些应用统计副本大小，关闭meta/ctrl生成以提升遍历性能）

```c
typedef struct ScanStats_S {
    uint64_t openDirRequestCnt;
    uint64_t openDirTime;
    uint64_t totalDirs;
    uint64_t totalFiles;
    uint64_t totalSize;
    uint64_t totalFailedDirs;
    uint64_t totalFailedFiles;
    uint64_t totalControlFiles;
    uint64_t totalSizeToBackup;
    uint64_t totalDirsToBackup;
    uint64_t totalFilesToBackup;
} ScanStats;
```
描述扫描过程的统计信息（文件/目录数，大小，失败文件/目录数，待备份数），大小以字节为单位。


> 所有的`char*`类型不可为`NULL`，路径必须是绝对路径。Windows下路径形式为`C:\dir1\dir2`，*nix下路径形式为`/dir1/dir2`，需要区别平台的PATH SEPARATOR

Scanner的调用流程为：
1. `InitLog(const char* fullLogPath, int logLevel)`初始化日志，设置日志输出目录和日志等级
2. `CreateScannerInst(ScanConf scanConf)`从Scanner配置中创建scanner实例，失败返回`NULL`
3. `StartScanner(void *scannerHandle, const char *dirPath)`和`StartScannerBatch(void *scannerHandle, const char *dirPathList)`异步启动扫描任务。`StartScanner(void *scannerHandle, const char *dirPath)`扫描`dirPath`中指定的路径。`StartScannerBatch(void *scannerHandle, const char *dirPathList)`可以传入多个扫描路径（以;分割）。
4. `MonitorScanner(void *scannerHandle, const char *jobId)`监控扫描任务是否结束，该接口为阻塞接口，扫描成功返回`true`，失败返回`false`。
5. 在`MonitorScanner`阻塞调用执行的过程中可以在其他线程中用`GetStatistics(void *scannerHandle)`检测Scanner的统计信息，返回`ScanStats`结构
6. 在Scanner实例创建成功后，无论是否执行成功，都需要用`DestroyScannerInst(void *scannerHandle)`释放资源，避免内存泄漏


`StartScanner(void *scannerHandle, const char *dirPath)`和`StartScannerBatch(void *scannerHandle, const char *dirPathList)`传入的路径总和数量都不能超过100，否则会卡死，对于要扫描多个文件的场景，建议传入其上层目录的路径。所有的路径必须是绝对路径，且是平台对应的标准路径，除了Windows的驱动器根目录(`C:\`)或*nix的根目录(`/`)都不以SEPARATOR字符结尾。这两个接口包含了隐含的`prefix`参数，为：从前往后找第一个'/'之前的前缀路径。例如：`/mnt/114514/dir1`的前缀就是`/mnt/114514`。

Scanner的前缀（prefix）机制用于修改控制文件的路径前缀，以适配Backup的rootPath。例如一个目录`/mnt/114514/dir1`，他的结构如下：
```
/mnt/114514/dir1
  |---dir2
  |     |---file1
  |     |---file2
  |---file3
  |---file4
```
如果不设置前缀，即`prefix`为空，则控制文件为：
```
/mnt/114514/dir1
file
file4
/mnt/114514/dir1/dir2
file1
file2
```
用这种控制文件去调用backup，则副本中文件会带上`/mnt/114514`的路径结构。如果想让副本中只有`dir1`的目录结构，需要把前缀设置为`/mnt/114514`，此时生成的控制文件为：
```
/dir1
file
file4
/dir1/dir2
file1
file2
```
由于历史原因，`StartScanner(void *scannerHandle, const char *dirPath)`和`StartScannerBatch(void *scannerHandle, const char *dirPathList)`中没有支持设置`prefix`。所以需要定制控制文件的场景就需要使用V2接口`bool StartScannerV2(void *scannerHandle, const char* path, const char* prefix)`