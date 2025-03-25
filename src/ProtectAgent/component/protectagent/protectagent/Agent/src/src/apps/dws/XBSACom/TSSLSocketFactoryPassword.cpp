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
#include "XBSACom/TSSLSocketFactoryPassword.h"
#include "common/Log.h"
#include "common/Path.h"
#include "securecom/CryptAlg.h"
#include "common/ConfigXmlParse.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::transport;

TSSLSocketFactoryPassword::TSSLSocketFactoryPassword()
{
}
TSSLSocketFactoryPassword::~TSSLSocketFactoryPassword()
{
}

EXTER_ATTACK void TSSLSocketFactoryPassword::getPassword(std::string& password, int size)
{
    INFOLOG("update ssl cert password");
    std::string ciPherStr;
    if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueString(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, ciPherStr)) {
        ERRLOG("get GetValueString of ssl_key_password failed.");
        return;
    }

    DecryptStr(ciPherStr, password);
    if (password.empty()) {
        ERRLOG("DecryptStr private key password failed.");
        return;
    }
}