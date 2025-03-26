# FS_Scanner

## Scanner编译方法
1.下载依赖
```
cd build
sh download_code_for_developer.sh
```
2.第一次执行编译，需要编译dme
```
sh make_nas_scanner.sh
```
3.若非第一次编译，可以执行
```
sh build_nas_scanner.sh
```

## Scanner LLT运行方法
执行LLT之前需要执行scanner编译
方法1：执行指定用例，例如：ScanTaskTest.Test1
```
cd test
sh installgmock.sh --gtest_filter=ScanTaskTest.Test1
```
方法2：通过gdb调试单个用例
```
cd test
sh installgmock.sh gdb
```
进入gdb后设置用例参数
```
(gdb) set args --gtest_filter=ScanTaskTest.Test1
(gdb) run
```
方法3：全量跑LLT用例
```
cd test
sh installgmock.sh
```
方法4：全量执行LLT，包含整体的覆盖率
```
cd build
sh make_scanner.sh LLT
cd ../test
sh installgmock.sh
ll coverage_report_scanner
```
## scanner_demo 基于快照的挂载克隆文件系统的文件扫描
cd tool/scanner_demo
1.执行编译demo
```
sh test.sh
```
2.执行编译并运行
```
sh test.sh run
```
3.文件路径
可执行文件路径: bin/scanner_demo_exe
日志路径: 定义在scanner_demo.cpp中的LOG_PATH, 默认/tmp/scanner_demo.log

