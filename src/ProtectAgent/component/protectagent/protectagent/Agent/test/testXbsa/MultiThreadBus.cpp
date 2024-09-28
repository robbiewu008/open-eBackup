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
#include "MultiThreadBus.h"
#include "RoachClient.h"
#include <unistd.h>
#include "CommonDefine.h"

void backupFun(const std::string libPath, int num, const std::string &testFileName)
{
    RoachClient client(libPath);
    client.initSymbol();
    std::string serial = GenerateSerial();
    std::string objectNamePrefix = std::string("/roach/metadata/internalTest/backup/") + serial + "/";
    client.setObjectName(objectNamePrefix);
    if (!testFileName.empty()) {
        client.setTestFileName(testFileName);
    }   
    client.setSerialNum(serial);
    while(true)
    {
        int ret = client.simulateBackup();
        if (BSA_RC_SUCCESS != ret)
        {
            Log("backup child process %d stop, current bsa handle is %ld", num, client.getCurrentBsaHandle());
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}

void deleteFun(const std::string libPath, int num, const std::string &testFileName)
{
    RoachClient client(libPath);
    client.initSymbol();
    std::string serial = GenerateSerial();
    std::string objectNamePrefix = std::string("/roach/metadata/internalTest/delete/") + serial + "/";
    client.setObjectName(objectNamePrefix);
    if (!testFileName.empty()) {
        client.setTestFileName(testFileName);
    }  
    client.setSerialNum(serial);
    while(true)
    {
        int ret = client.simulateDelete();
        if (BSA_RC_SUCCESS != ret)
        {
            Log("delete child process %d stop", num);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}

void recoverFun(const std::string libPath, int num, const std::string &testFileName)
{
    RoachClient client(libPath);
    client.initSymbol();
    std::string serial = GenerateSerial();
    std::string objectNamePrefix = std::string("/roach/metadata/internalTest/recover/") + serial + "/";
    client.setObjectName(objectNamePrefix);
    client.setSerialNum(serial);
    if (!testFileName.empty()) {
        client.setTestFileName(testFileName);
    }  
    while(true)
    {
        int ret = client.simulateRecover();
        if (BSA_RC_SUCCESS != ret)
        {
            Log("delete child process %d stop", num);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}

MultiThreadBus::MultiThreadBus()
{
    m_testFileName = "";
    m_operateFile = false;
}

MultiThreadBus::~MultiThreadBus()
{
}

void MultiThreadBus::startBackupBus(int num, const std::string &libPath)
{
    // 1. ����Ŀ¼�洢����Ԫ����
    if (m_operateFile) {
        bool ret = CreateNewDir(TEST_BACKUP_DIR);
        if (!ret)
        {
            Log("Create backup dir failed.");
            return;
        }
    }

    for (int i = 0; i<num; i++)
    {
        pid_t newPid = 0;
        newPid = fork();
        if (newPid > 0)
        {
            backupFun(libPath, i, m_testFileName);
            exit(0);
        }
    }
}

void MultiThreadBus::startDeleteBus(int num, const std::string &libPath)
{
    // 1. ��������ɾ�����ܵĴ洢����Ԫ����Ŀ¼
    Log("m_operateFile:%d", m_operateFile);
    if (m_operateFile) {
        bool ret = CreateNewDir(TEST_DELETE_DIR);
        if (!ret)
        {
            Log("Create delete dir failed.");
            return;
        }
    }

    for (int i = 0; i<num; i++)
    {
        pid_t newPid = 0;
        newPid = fork();
        if (newPid > 0)
        {
            deleteFun(libPath, i, m_testFileName);
            exit(0);
        }
    }
}

void MultiThreadBus::startRecoverBus(int num, const std::string &libPath)
{
    // 1. �������ڻָ����ܵ�Ŀ¼
    if (m_operateFile)
    {
        bool ret = CreateNewDir(TEST_RECOVER_DIR);
        if (!ret)
        {
            Log("Create recover dir failed.");
            return;
        }
    }

    for (int i = 0; i<num; i++)
    {
        pid_t newPid = 0;
        newPid = fork();
        if (newPid > 0)
        {
            recoverFun(libPath, i, m_testFileName);
            exit(0);
        }
    }
}

void MultiThreadBus::setTestFileName(const std::string &fileName)
{
    m_testFileName = fileName;
    m_operateFile = true;
}
