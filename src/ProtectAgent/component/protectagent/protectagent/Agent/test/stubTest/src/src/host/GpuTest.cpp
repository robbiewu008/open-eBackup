#include "host/GpuTest.h"

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubExecSystemWithEcho0(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    mp_string gpuInfo1 = "VGA compatible controller: NVIDIA Corporation G98 Subsystem: NVIDIA Corporation Device 062e";
    mp_string gpuInfo2 = "Flags: bus master, fast devsel, latency 0, IRQ 24";
    mp_string gpuInfo3 = "Memory at f6000000 (32-bit, non-prefetchable) [size=16M]";
    mp_string gpuInfo4 = "Memory at ec000000 (64-bit, prefetchable) [size=64M]";
    mp_string gpuInfo5 = "Memory at f4000000 (64-bit, non-prefetchable) [size=32M]";
    mp_string gpuInfo6 = "I/O ports at dc80 [size=128]";
    mp_string gpuInfo7 = "[virtual] Expansion ROM at f7e00000 [disabled] [size=128K]";
    mp_string gpuInfo8 = "Capabilities: <access denied>";
    mp_string gpuInfo9 = "Kernel driver in use: nvidia";

    strEcho.push_back(gpuInfo1);
    strEcho.push_back(gpuInfo2);
    strEcho.push_back(gpuInfo3);
    strEcho.push_back(gpuInfo4);
    strEcho.push_back(gpuInfo5);
    strEcho.push_back(gpuInfo6);
    strEcho.push_back(gpuInfo7);
    strEcho.push_back(gpuInfo8);
    strEcho.push_back(gpuInfo9);
    return 0;
}

mp_int32 StubExecSystemWithEchoEmpty(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    return MP_SUCCESS;
}

mp_int32 StubExecSystemWithEchoFail(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    return MP_FAILED;
}

mp_int32 StubExtractGpuInfoSuccess(gpu_info_t& gpuInfo, vector<mp_string>& cmdEchoVector)
{
    return MP_SUCCESS;
}

mp_int32 StubExtractGpuInfoFail(gpu_info_t& gpuInfo, vector<mp_string>& cmdEchoVector)
{
    return MP_FAILED;
}

mp_bool StubGetGpuControllerNameSuccess(mp_string& ctrlName, const mp_string& gpuInfoStr)
{
    return MP_TRUE;
}

mp_bool StubGetGpuControllerNameFail(mp_string& ctrlName, const mp_string& gpuInfoStr)
{
    return MP_FALSE;
}

mp_bool StubGetGpuSubSystemSuccess(mp_string& ctrlName, const mp_string& gpuInfoStr)
{
    return MP_TRUE;
}

mp_bool StubGetGpuSubSystemFail(mp_string& ctrlName, const mp_string& gpuInfoStr)
{
    return MP_FALSE;
}

mp_bool StubGetGpuMemSuccess(gpu_info_t& gpuInfo, const mp_string& gpuInfoStr)
{
    return MP_TRUE;
}

mp_bool StubGetGpuMemFail(gpu_info_t& gpuInfo, const mp_string& gpuInfoStr)
{
    return MP_FALSE;
}

/*
 * 用例名称：获取GPU设备ID
 * 前置条件：
 * check点：获取成功，检查返回值
            读取失败，检查返回值
 */
TEST_F(GpuTest, GetGpuDeviceId)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CGpu work;
    mp_int32 iRet; 

    stub.set(&CSystemExec::ExecSystemWithEcho, StubExecSystemWithEcho0);
    iRet = work.GetGpuDeviceId();
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CSystemExec::ExecSystemWithEcho, StubExecSystemWithEchoFail);
    iRet = work.GetGpuDeviceId();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CSystemExec::ExecSystemWithEcho, StubExecSystemWithEchoEmpty);
    iRet = work.GetGpuDeviceId();
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：提取GPU信息
 * 前置条件：
 * check点：获取成功，检查返回值
 */
TEST_F(GpuTest, ExtractGpuInfo)
{
    CGpu work;
    mp_int32 iRet;

    stub.set(&CGpu::GetGpuControllerName, StubGetGpuControllerNameSuccess);
    stub.set(&CGpu::GetGpuSubSystem, StubGetGpuSubSystemSuccess);
    stub.set(&CGpu::GetGpuMem, StubGetGpuMemSuccess);
    gpu_info_t gpuInfo;
    vector<mp_string> cmdEchoVector = { "111", "222" };
    iRet = work.ExtractGpuInfo(gpuInfo, cmdEchoVector);
    EXPECT_EQ(iRet, MP_SUCCESS);

    cmdEchoVector.clear();
    iRet = work.ExtractGpuInfo(gpuInfo, cmdEchoVector);
    EXPECT_EQ(iRet, MP_FAILED);

    cmdEchoVector.push_back("111");
    stub.set(&CGpu::GetGpuControllerName, StubGetGpuControllerNameFail);
    iRet = work.ExtractGpuInfo(gpuInfo, cmdEchoVector);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CGpu::GetGpuControllerName, StubGetGpuControllerNameSuccess);
    stub.set(&CGpu::GetGpuSubSystem, StubGetGpuSubSystemFail);
    iRet = work.ExtractGpuInfo(gpuInfo, cmdEchoVector);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CGpu::GetGpuControllerName, StubGetGpuControllerNameSuccess);
    stub.set(&CGpu::GetGpuSubSystem, StubGetGpuSubSystemSuccess);
    stub.set(&CGpu::GetGpuMem, StubGetGpuMemFail);
    iRet = work.ExtractGpuInfo(gpuInfo, cmdEchoVector);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * 用例名称：获取GPU控制器名称
 * 前置条件：
 * check点：获取成功，检查返回值
            获取失败，检查返回值
 */
TEST_F(GpuTest, GetGpuControllerName)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CGpu work;
    mp_bool iRet;

    mp_string ctrlName;
    mp_string gpuInfoStr = "00:02.0 VGA compatible controller: Cirrus Logic GD 5446";
    iRet = work.GetGpuControllerName(ctrlName, gpuInfoStr);
    EXPECT_EQ(iRet, MP_TRUE);
    EXPECT_EQ(ctrlName, "Cirrus Logic GD 5446");

    gpuInfoStr = "";
    iRet = work.GetGpuControllerName(ctrlName, gpuInfoStr);
    EXPECT_EQ(iRet, MP_TRUE);

    gpuInfoStr = "00:02.0 VGA compatible controller";
    iRet = work.GetGpuControllerName(ctrlName, gpuInfoStr);
    EXPECT_EQ(iRet, MP_FALSE);
}

/*
 * 用例名称：获取GPU子系统
 * 前置条件：
 * check点：获取成功，检查返回值
            获取失败，检查返回值
 */
TEST_F(GpuTest, GetGpuSubSystem)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CGpu work;
    mp_bool iRet;

    mp_string gpuSubSystem;
    mp_string gpuInfoStr = "Subsystem: VMware SVGA";
    iRet = work.GetGpuSubSystem(gpuSubSystem, gpuInfoStr);
    EXPECT_EQ(iRet, MP_TRUE);
    EXPECT_EQ(gpuSubSystem, "VMware SVGA");

    gpuInfoStr = "";
    iRet = work.GetGpuControllerName(gpuSubSystem, gpuInfoStr);
    EXPECT_EQ(iRet, MP_TRUE);
}

/*
 * 用例名称：获取显存信息
 * 前置条件：
 * check点：获取成功，检查返回值
            获取失败，检查返回值
 */
TEST_F(GpuTest, GetGpuMem)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CGpu work;
    mp_bool iRet;

    gpu_info_t gpuInfo;
    mp_string gpuInfoStr = "Memory at fb000000 (32-bit, non-prefetchable) [size=16M]";
    iRet = work.GetGpuMem(gpuInfo, gpuInfoStr);
    EXPECT_EQ(iRet, MP_TRUE);

    gpuInfoStr = "Memory at d0000000 (64-bit, prefetchable) [size=128M]";
    iRet = work.GetGpuMem(gpuInfo, gpuInfoStr);
    EXPECT_EQ(iRet, MP_TRUE);
}