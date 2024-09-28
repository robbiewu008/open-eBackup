/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "common/IpTest.h"
#include "common/ErrorCode.h"
#include "common/Ip.h"
/*------------------------------------------------------------ 
 Description :����CIP::IsIPV4(mp_string&)
 Input         :   
 Output       :    
 Return       : 
 Create By  : 
Modification : 
-------------------------------------------------------------*/
using namespace std;

static mp_void StubCLoggerLog(mp_void){
    return;
}
mp_bool MockFileExist_TRUE(const mp_string& pszFilePath)
{
    return MP_TRUE;
}

mp_bool MockFileExist_FALSE(const mp_string& pszFilePath)
{
    return MP_FALSE;
}

mp_int32 MockReadFile(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("ENVIRONMENT_TYPE=0");
    return MP_SUCCESS;
}

mp_int32 MockReadFile_FALSE(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("ENVIRONMENT_TYPE=0");
    return MP_FAILED;
}

TEST_F(IPTest, IsIPV4TEST){
    mp_string strIpAddr = "";
    mp_bool ret = true;

    //���ַ���
    ret = CIP::IsIPV4(strIpAddr);
    EXPECT_EQ(MP_FALSE, ret);

    //ȫ���ַ���
    strIpAddr = "0.0.0.0";
    ret = CIP::IsIPV4(strIpAddr);
    EXPECT_EQ(MP_FALSE, ret);

    //ĳ���γ���255
    strIpAddr = "127.0.3.256";
    ret = CIP::IsIPV4(strIpAddr);
    EXPECT_EQ(MP_FALSE, ret);

    //���������ַ�
    strIpAddr = "1o9.25.31.4l";
    ret = CIP::IsIPV4(strIpAddr);
    EXPECT_EQ(MP_FALSE, ret);

    //��ipv4���͸�ʽ
    strIpAddr = "10:2:3:5";
    ret = CIP::IsIPV4(strIpAddr);
    EXPECT_EQ(MP_FALSE, ret);
    
    //�����ַ���
    strIpAddr = "10.2.3.5";
    ret = CIP::IsIPV4(strIpAddr);
    EXPECT_EQ(MP_TRUE, ret);
}

TEST_F(IPTest, GetUIntIpAddr)
{
    mp_int32 iRet;
    mp_string ipArr = "10.10";
    mp_uint32 uiIpAddr;
    mp_uint32 uiLen;
    stub.set(&CLogger::Log, StubCLoggerLog);

    iRet = CIP::IPV4StrToUInt(ipArr, uiIpAddr);
    EXPECT_EQ(iRet, MP_FAILED);

    ipArr = "10.10.10.10";
    iRet = CIP::IPV4StrToUInt(ipArr, uiIpAddr);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(uiIpAddr, 0x0a0a0a0a);

    uiLen = 3;
    mp_uchar charIpAdd[5] = {0};
    iRet = CIP::IPV4StrToUInt(ipArr, charIpAdd, uiLen);
    EXPECT_EQ(iRet, MP_FAILED);

    uiLen = 4;
    iRet = CIP::IPV4StrToUInt(ipArr, charIpAdd, uiLen);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(uiIpAddr, 0x0a0a0a0a);
}

TEST_F(IPTest, IPV4UIntToStr)
{
    mp_int32 iRet;
    mp_uchar IpAdd[3] = {0x0a,0x0a};
    mp_string strIpAddr;

    iRet = CIP::IPV4UIntToStr(IpAdd, strIpAddr);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(strIpAddr, "10.10.0.0");

    mp_uchar charIpAdd[5] = {0x0a,0x0a,0x0a,0x0a};
    iRet = CIP::IPV4UIntToStr(charIpAdd, strIpAddr);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(strIpAddr, "10.10.10.10");
}

TEST_F(IPTest, IPV6StrToUInt)
{
    mp_int32 iRet;
    mp_uint32 uiLen;
    mp_uchar charIpAdd[17] = {0};
    mp_string strIpAddr = "0::0:0:0:0";
    stub.set(&CLogger::Log, StubCLoggerLog);

    uiLen = 3;
    iRet = CIP::IPV6StrToUInt(strIpAddr,charIpAdd, uiLen);
    EXPECT_EQ(iRet,MP_FAILED); // ��ЧIP����ֵ�������ж����

    uiLen = 16;
    iRet = CIP::IPV6StrToUInt(strIpAddr,charIpAdd, uiLen);
    EXPECT_EQ(iRet, MP_SUCCESS);
    for (int i = 0; i < IPV6_NUMERIC_LEN; i++) {
      EXPECT_EQ(charIpAdd[i], 0x00);
    }
}

TEST_F(IPTest, IPV6UIntToStr)
{
    mp_int32 iRet;
    mp_uchar charIpAdd[17] = {0};
    mp_string strIpAddr;
    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = CIP::IPV6UIntToStr(charIpAdd, strIpAddr);
    EXPECT_EQ(iRet,MP_SUCCESS);
    EXPECT_EQ(strIpAddr, "::");
}

TEST_F(IPTest, IsIPv6)
{
    mp_int32 iRet;
    mp_string ipv6Addr = ":::::8::::::8::::8";
    stub.set(&CLogger::Log, StubCLoggerLog);

    iRet = CIP::IsIPv6(ipv6Addr);
    EXPECT_EQ(iRet,MP_FALSE);

    ipv6Addr = "::";
    iRet = CIP::IsIPv6(ipv6Addr);
    EXPECT_EQ(iRet,MP_TRUE);

    ipv6Addr = "fe80::2a6e:d4ff:fe89:4506";
    iRet = CIP::IsIPv6(ipv6Addr);
    EXPECT_EQ(iRet,MP_TRUE);
}

TEST_F(IPTest, CheckIsIPv6OrIPv4)
{
    mp_int32 iRet;
    mp_string ipv6Addr = ":::::8::::::8::::8";
    stub.set(&CLogger::Log, StubCLoggerLog);

    iRet = CIP::CheckIsIPv6OrIPv4(ipv6Addr);
    EXPECT_EQ(iRet,MP_FALSE);

    ipv6Addr = "10.10.10.10";
    iRet = CIP::CheckIsIPv6OrIPv4(ipv6Addr);
    EXPECT_EQ(iRet,MP_TRUE);

    ipv6Addr = "fe80::2a6e:d4ff:fe89:4506";
    iRet = CIP::CheckIsIPv6OrIPv4(ipv6Addr);
    EXPECT_EQ(iRet,MP_TRUE);

    mp_string strIpv6 = "fe80::2a6e:d4ff:fe89:4506";
    mp_string strIpv6_format = CIP::FormatFullUrl(strIpv6);
    EXPECT_EQ("[fe80::2a6e:d4ff:fe89:4506]", strIpv6_format);
    EXPECT_EQ(strIpv6, CIP::ParseIPv6(strIpv6_format, false));
}

TEST_F(IPTest, GetListenPort)
{
    mp_int32 iRet;
    mp_string ipAddr;
    mp_string port;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(CMpFile,FileExist), StubFileExist);
    stub.set(ADDR(CMpFile, ReadFile), StubCMpFileReadFile);
    EXPECT_EQ(MP_SUCCESS, CIP::GetListenIPAndPort(ipAddr, port));
}


/*
 * �������ƣ���ȡ���������ļ��ɹ�
 * ǰ�����������������ļ����ڣ��Ҷ�ȡ���ݳɹ�
 * check�㣺GetBuildINEnvironmentTyper�ɹ�
 */
TEST_F(IPTest, GetBuildINEnvironmentTypeSuccessTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CMpFile::FileExist, MockFileExist_TRUE);
    stub.set(&CMpFile::ReadFile, MockReadFile);
    mp_string environment;
    auto iRet = CIP::GetBuildINEnvironmentType(environment);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * �������ƣ���ȡ���������ļ�ʧ��
 * ǰ��������NA
 * check�㣺���������ļ�������
 */
TEST_F(IPTest, GetBuildINEnvironmentTypeFailedTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CMpFile::FileExist, MockFileExist_FALSE);
    mp_string environment;
    auto iRet = CIP::GetBuildINEnvironmentType(environment);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * �������ƣ���ȡ���������ļ�ʧ��
 * ǰ�����������������ļ�����
 * check�㣺��ȡ���������ļ��ɹ�
 */
TEST_F(IPTest, GetBuildINEnvironmentTypeTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CMpFile::FileExist, MockFileExist_TRUE);
    stub.set(&CMpFile::ReadFile, MockReadFile_FALSE);
    mp_string environment;
    auto iRet = CIP::GetBuildINEnvironmentType(environment);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * �������ƣ���ȡ���������ɹ�
 * ǰ��������������������
 * check�㣺��ȡ���������ɹ�
 */
TEST_F(IPTest, GetHostEnvSuccTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string tmpEnv;
    mp_int32 iRet = CIP::GetHostEnv("PATH", tmpEnv);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * �������ƣ���ȡ��������ʧ��
 * ǰ����������������������
 * check�㣺��ȡ��������ʧ��
 */
TEST_F(IPTest, GetHostEnvFailTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string tmpEnv;
    mp_int32 iRet = CIP::GetHostEnv("b775-49ef-9a52", tmpEnv);
    EXPECT_EQ(iRet, MP_FAILED);
}
