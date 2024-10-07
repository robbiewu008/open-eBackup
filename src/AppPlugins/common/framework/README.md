备份插件框架-使用Module仓库替代DME_FRAMEWORK
=================
## 框架本地编译
1. Module仓库已通过 git submodule 方式添加到
开发者本地初始化 Module 的子代码仓
git submodule update --init --recursive
统一切换子仓分支 到 SUB_SYSTEM
git submodule foreach "git checkout SUB_SYSTEM;:"

Module仓库在AppPlugins根目录下 /Module

2. 第一次执行编译，需要全量编译出包
```shell
cd /framework/build
sh build.sh
```
3. 若非第一次编译，可以执行
```shell
sh build_framework.sh
```

4. 编译好的库目录

框架动态库:
/framework/lib/libbasic_job.so
/framework/lib/librpc.so
/framework/lib/libthrift_client.so
/framework/lib/libthrift_interface.so
/framework/lib/libthrift_server.so
/framework/lib/libutils.so

Module仓动态库:
/framework/lib/libcommon_util.so
/framework/lib/libconfig_reader_util.so
/framework/lib/liblog_util.so

Agent sdk:
/framework/lib/agent_sdk/libpluginsdk.so

插件thrift开源使用的boost库(应用动态库使用boost建议去 /Module/third_open_src 中引用):
framework/lib/3rd/libboost*.so

插件使用的安全函数库：
framework/lib/3rd/libsecurec.so



## Module仓库使用方法
1. 目录

|目录|功能|
|:---|:---|
|/Module/src/define|常量定义
|/Module/src/log|日志
|/Module/src/common|文件操作、json、线程等|
|/Module/src/config_reader|读取配置文件|
|/Module/src/system|系统库|
|/Module/src/threadpool|线程池|
|/Module/src/metafile_parser|meta文件解析|
|/Module/src/parser|control文件解析|

2. 使用方法-直接编译源码
liblog_util.so、libcommon_util.so、libconfig_reader_util.so 三个动态库是框架使用的，框架编译完之后拷贝到 /framework/lib目录下
其余模块或新增模块，可通过add_subdirectory方式，举例：
add_subdirectory(${MODULE_PATH}/log output_log)

3. 使用方法-引用动态库
调用 /framework/build/build_framework.sh 后，会在 /framework/lib 目录下生成部分动态库
/framework/lib/libcommon_util.so
/framework/lib/libconfig_reader_util.so
/framework/lib/liblog_util.so

4. 使用开源库
开源三方库已经编译好并下载在 /Module/third_open_src
请优先使用该目录下的开源库

5. 自研库
自研库已经编译好并下载在 /Module/platform

## 应用门禁编译
**每个应用独立编译，各自提供编译脚本，门禁会依据所提交代码目录，执行编译脚本**
```shell
sh /plugins/${app}/CI/script/build.sh
```
**流程**
1. 编译插件公共框架 
```shell
sh /framework/build/build.sh
```
2. 编译插件应用

## 应用LLT
**每个应用独立编译LLT，各自提供LLT编译脚本，门禁会依据所提交代码目录，执行编译脚本**
```shell
sh /plugins/${app}/CI/script/build.sh LLT
```

**流程**

1.初始化HDT框架
调用 /test/install_hdt.sh
2.编译公共框架
3.编译应用LLT
4.执行用例

## 应用打包
**每个应用独立打包，各自提供打包脚本，流水线根据配置参数，执行对应打包脚本**
```shell
sh /plugins/${app}/CI/script/pack.sh
```

**流程**

1.编译插件框架
```shell
sh /framework/build/build.sh
```

2.编译应用动态库

3.拷贝应用动态库、配置文件、业务脚本等到 /framework/output_pkg/（每次 build_framework.sh 都会动态生成 /framework/output_pkg/）
去掉了service目录，统一把动态库放置在lib目录下
```shell
/framework/output_pkg/lib             # 框架动态库(启动脚本会添加LD_LIBRARY_PATH)
/framework/output_pkg/lib/service     # 应用动态库(启动脚本会添加LD_LIBRARY_PATH)
/framework/output_pkg/lib/3rd         # 框架和应用动态库的三方库(启动脚本会添加LD_LIBRARY_PATH)
/framework/output_pkg/conf            # 应用配置文件
/framework/output_pkg/script          # 应用业务脚本
/framework/output_pkg/install/install.sh     # 应用安装脚本，插件框架安装过程中会调用该脚本，做额外初始化动作，若没有脚本则跳过
```

4.添加应用必要配置文件
/framework/output_pkg/conf 必须放置 plugin_attribute_1.1.0.json,每个应用独一份
plugin_attribute_1.1.0.json 的作用：
* 框架打包脚本会根据json确定插件包的命名
* 框架安装脚本根据json做初始化
* Agent读取json确定插件应用类型
* 框架读取json获取应用名称，向Agent注册

5.调用框架通用打包脚本(框架提供安装部署启动脚本)
```shell
sh /framework/build/pack.sh 
```

在out_pkg目录生成 ${app}.tar.gz 或者 ${app}.tar.xz

## 新增应用开发自测流程
1. /plugins/ 目录下新建应用，应用内部目录结构可参考：
```shell
/plugins/${app}/src
/plugins/${app}/script
/plugins/${app}/test
/plugins/${app}/lib
```

2. /plugins/${app}/conf 目录新建 plugin_attribute_1.1.0.json，举例：
```json
{
    "application": "NasFileSystem,NasShare",
    "name": "NasPlugin",
    "application_version": [{
            "application": "NasFileSystem",
            "min_version": "",
            "max_version": ""
        },
        {
            "application": "NasShare",
            "min_version": "",
            "max_version": ""
        }
    ],
    "run_account": "root",
    "feature": [
        "backup",
        "restore",
        "livemount",
        "scan_resource"
    ]
}
```

3. /framework/conf/app_lib.json 添加新的应用名以及对应的动态库名称，举例：
```json
{
    "Plugin": [{
            "PluginName": "NasPlugin",
            "LibName": "libnas",
            "AppName": "NasFileSystem,NasShare"
        }
    ]
}
```

4. 编译框架、编译应用、打包

5. 换包测试
```shell
拷贝${app}.tar.gz包到agent环境的 ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/Plugins/${app}
解压${app}.tar.gz,再次解压plugin.tar.xz
进入bin目录
执行start.sh ${DATA_BACKUP_AGENT_HOME}/DataBackup/ProtectClient/ProtectClient-E//slog/Plugins/${app} 59610 59640 127.0.0.1 59570
测试插件能否正常启动
参数说明：
$1：日志路径
$2、$3：插件端口范围
$4：本地ip
$5：agent ip
```

## 接口变更
1.thrift接口变更需要评审
2.thrift接口文件位置： /framework/thrift_files
3.修改thrift接口后，需要修改框架代码适配新接口
3.1.服务端接口新增
```
```
3.2.客户端接口新增
```
```
3.3.thrift服务新增
```
```

## 导出函数列表
TODO
新增Init接口
资源浏览接口修改
