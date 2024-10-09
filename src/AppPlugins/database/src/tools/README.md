数据库框架工具使用指南：
一、RPC接口调用工具(linux:(dbrpctool rpctool.sh) windows:(rpctool.exe rpctool.bat))
    1、工具目录存放
        linux : /opt/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin
        windows : C:\DataBackup\ProtectClient\Plugins\GeneralDBPlugin\bin
    2、使用方式
        linux: sh /opt/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/rpctool.sh <接口名> <输入参数文件路径> <输出参数文件路径>
        windows: call C:\DataBackup\ProtectClient\Plugins\GeneralDBPlugin\bin\rpctool.bat <接口名> <输入参数文件路径> <输出参数文件路径>
        例：上报任务详情
            sh /opt/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/rpctool.sh ReportJobDetails /tmp/inputt111 /tmp/output111
            call C:\DataBackup\ProtectClient\Plugins\GeneralDBPlugin\bin\rpctool.bat ReportJobDetails C:\inputt111 C:\output111
        注：输入参数文件由应用生成，输入参数文件存放接口输入参数内容，文件内容格式为json格式；
            输出参数文件保存接口输出返回内容，文件内容格式为json格式，文件工具会自动创建；
            输入参数文件与输出参数文件由应用负责删除。
    3、接口名列表：
        CreateResource —— 创建共享资源
        QueryResource —— 查询共享资源
        UpdateResource —— 更新共享资源
        DeleteResource —— 删除共享资源
        LockResource —— 锁定共享资源
        UnLockResource —— 解锁共享资源
        ReportJobDetails —— 上报任务详情
        ReportCopyAdditionalInfo —— 上报副本信息
        QueryPreviousCopy —— 查询最新副本
        MountRepositoryByPlugin —— 挂载文件系统
        UnMountRepositoryByPlugin —— 去挂载文件系统
