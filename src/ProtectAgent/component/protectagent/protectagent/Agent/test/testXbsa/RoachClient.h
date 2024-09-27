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
#ifndef _ROACH_CLIENT_H_
#define _ROACH_CLIENT_H_

#include "xbsa.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <iostream>
#include <vector>


typedef int (*SYMBOY_COMMON)(long);
typedef int (*SYMBOY_CREATE_OBJECT)(long,BSA_ObjectDescriptor*,BSA_DataBlock32*);
typedef int (*SYMBOY_BSAINIT)(long *, BSA_SecurityToken *, BSA_ObjectOwner *, char **);
typedef int (*SYMBOY_DELETE_OBJ)(long bsaHandle, BSA_UInt64 copyId);
typedef int (*SYMBOY_END_TXN)(long bsaHandle, BSA_Vote vote);
typedef int (*SYMBOY_GET_DATA)(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);
typedef int (*SYMBOL_GET_ENV)(long bsaHandle, BSA_ObjectOwner * objectOwner, char ** ptr);
typedef int (*SYMBOL_GET_LASTERR)(BSA_UInt32 *sizePtr, char *errorCodePtr);
typedef int (*SYMBOL_GET_NEX_QUERY_OBJ)(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr);
typedef int (*SYMBOL_GET_OBJ)(long bsaHandle, BSA_ObjectDescriptor *objectDescriptorPtr, BSA_DataBlock32 *dataBlockPtr);
typedef int (*SYMBOL_GET_VERSION)(BSA_ApiVersion *apiVersionPtr);
typedef int (*SYMBOL_QUERY_OBJ)(long bsaHandle, BSA_QueryDescriptor *queryDescriptorPtr, BSA_ObjectDescriptor *objectDescriptorPtr);
typedef int (*SYMBOL_SEND_DATA)(long bsaHandle, BSA_DataBlock32 *dataBlockPtr);
typedef int (*SYMBOL_QUERY_SERVICE_PRIVIDE)(BSA_UInt32 *sizePtr, char *delimiter, char *providerPtr);
class RoachClient
{
    void *m_handle = nullptr;
    std::string m_libPath;
    SYMBOY_COMMON BSABeginTxn_fun;
    SYMBOY_COMMON BSAEndData_fun; 
    SYMBOY_COMMON BSATerminate_fun;
    SYMBOY_BSAINIT BSAInit_fun;
    SYMBOY_CREATE_OBJECT BSACreateObject_fun;
    SYMBOY_DELETE_OBJ BSADeleteObj_fun;
    SYMBOY_END_TXN BSAEndTxn_fun;
    SYMBOY_GET_DATA BSAGetData_fun;
    SYMBOL_GET_ENV BSAGetEnv_fun;
    SYMBOL_GET_LASTERR BSAGetLastErr_fun;
    SYMBOL_GET_NEX_QUERY_OBJ BSAGetNexQueryObj_fun;
    SYMBOL_GET_OBJ BSAGetObj_fun;
    SYMBOL_GET_VERSION BSAQueryVerion_fun;
    SYMBOL_QUERY_OBJ BSAQueryObj_fun;
    SYMBOL_SEND_DATA BSASendData_fun;
    SYMBOL_QUERY_SERVICE_PRIVIDE BSAQuerySerProvider_fun;
    int m_parallelBackup;  // �����ı�����
    int m_parallelRecover; // �����Ļָ���
    int m_parallelDel;  // ������ɾ����
    long m_nameIndex;
    std::string m_objectNamePrefix; // ��������ǰ׺�����������Ǳ���/ɾ��/�ָ�ҵ�� 
    std::string m_newestObjectName; // ���һ�β�����������
    long m_currentBsaHandle;
    bool m_operateFile; // �Ƿ�����ʵ�ļ����� 
    std::string m_testFileName; // ����ҵ��Ĳ����ļ� 
    std::string m_serialNum; 
public:
    RoachClient(const std::string libPath);  
    void initSymbol();
    int testBSABeginTxn(long handle);
    int testBSAInit(void);
    int simulateBackup();
    int simulateDelete();
    void setObjectName(const std::string &objNamePrefix);
    int getCurrentBsaHandle();
    int simulateRecover();
    void setTestFileName(const std::string &fileName);
    void setSerialNum(std::string serial);
private:  
    int BSAInit(long &handle);
    void initBsaObjDesc(BSA_ObjectDescriptor &objDesc, std::string objName);
    void sendData(std::string filePath);
    int QueryObject(std::vector<BSA_UInt64> &objList, const std::string &objName);
    int TerminateTxnAndSession(long bsaHandle, BSA_Vote vote);
};


#endif