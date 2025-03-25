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
#ifndef TSSLSOCKET_PASSWORD_H
#define TSSLSOCKET_PASSWORD_H

#include <iostream>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TSSLSocket.h>
#include "common/Defines.h"

typedef enum { // 移动位置
    BSA_GET_DATA_FROM_NAS = 0,
    BSA_GET_DATA_FROM_ARCHIVE = 1,
} BSA_Get_Data_Type;

class TSSLSocketFactoryPassword : public apache::thrift::transport::TSSLSocketFactory {
public:
    TSSLSocketFactoryPassword();
    ~TSSLSocketFactoryPassword();
protected:
    EXTER_ATTACK void getPassword(std::string& password, int size);
};

#endif