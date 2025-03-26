# Scanner Demo For Windows

## build
For Windows:
```cmd
build_scanner_demo.bat`
```

For Linux:
```bash
# Fileset only
sh build_demo.sh
# append NAS or OBS
sh build_demo.sh -DNAS=ON -DOBS=ON
```


## Exec
Before exec ScannerDemo, prepare the directory for scan output and log (configured in config.json)
```
...
"metaPath" : "/opt/ScannerDemo/scan/meta",
"metaPathForCtrlFiles" : "/opt/ScannerDemo/scan/ctrl",
"curDcachePath" : "/opt/ScannerDemo/scan/meta/previous",
"prevDcachePath" : "/opt/ScannerDemo/scan/meta/latest",
"indexPath" : "/opt/ScannerDemo/scan/rfi",
...
```


For Windows:
```cmd
ScannerDemo.exe -c <config path> -l <log path> -L <log level> [ enqueue path list... ]
```

For Linux：
```bash
cd FS_Scanner/tool/demo
export LD_LIBRARY_PATH=$(pwd)/bin
./bin/ScannerDemo -c <config path> -l <log path> -L <log level> [ enqueue path list... ]
```

Example:
```
ScannerDemo.exe -c Win32Config.json -l C:\ScannerDemo\log D:\dir1 D:\dir2 -L DEBUG
```

```
./ScannerDemo -c PosixConfig.json -l /opt/ScannerDemo/log /root/dir1 /root/dir2 -L DEBUG
```


if prefix is involved, using `::` to split it

```
ScannerDemo.exe -c Win32Config.json -l C:\ScannerDemo\log D:\vss_snapshot\114514\C\dir1::D:\vss_snapshot\114514\C -L DEBUG
```