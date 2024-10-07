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
#ifndef __CONST_H__
#define __CONST_H__
//#include "common/Types.h"

const std::string COMPRESS = "Compress";
const std::string DEDUP = "Dedup";
const std::string RESOURCE_ID = "ResourceID";
const std::string TARGET_RESOURCE_ID = "TargetResourceID";
const std::string HOST = "Host";
const std::string ISCSICONNECTOR = "IscsiConnector";
const std::string IP = "IP";

const std::string STORAGE = "BackupStorage";
const std::string STORAGE_TYPE = "Type";
const std::string STORAGE_IP = "IP";
const std::string STORAGE_PORT = "Port";
const std::string STORAGE_POOL = "PoolId";
const std::string STORAGE_USERNAME = "Username";
const std::string STORAGE_PASSWORD = "Password";

const std::string BANDWIDTHMIN = "BandwidthMin";
const std::string BANDWIDTHMAX = "BandwidthMax";
const std::string BANDWIDTHBURST = "BandwidthBurst";
const std::string IOPSMIN = "IOPSMin";
const std::string IOPSMAX = "IOPSMax";
const std::string IOPSBURST = "IOPSBurst";
const std::string LATENCY = "Latency";
const std::string BURST_TIME = "BurstTime";

const std::string NAME = "NAME";
const std::string ID = "ID";
const std::string OPERATIONSYSTEM = "OPERATIONSYSTEM";
const std::string ALLOCTYPE = "ALLOCTYPE";
const std::string CAPACITY = "CAPACITY";
const std::string PARENTID = "PARENTID";
const std::string PARENTTYPE = "PARENTTYPE";
const std::string USECHAP = "USECHAP";
const std::string CHAPNAME = "CHAPNAME";
const std::string CHAPPASSWORD = "CHAPPASSWORD";
const std::string DISCOVERYVERMODE = "DISCOVERYVERMODE";
const std::string NORMALVERMODE = "NORMALVERMODE";
const std::string SUBTYPE = "SUBTYPE";
const std::string WRITEPOLICY = "WRITEPOLICY";
const std::string WORKLOADTYPEID = "WORKLOADTYPEID";
const std::string ENABLECOMPRESSION = "ENABLECOMPRESSION";
const std::string ENABLESMARTDEDUP = "ENABLESMARTDEDUP";
const std::string DESCRIPTION = "DESCRIPTION";

const std::string SUPPORTPROTOCOL_NFS = "1";
const std::string SUPPORTPROTOCOL_CIFS = "2";
const std::string SUPPORTPROTOCOL_NFS_CIFS = "3";
const std::string SUPPORTPROTOCOL_DATATURBO = "1024";
const std::string SUPPORTPROTOCOL_ISCSI = "4";
const std::string ETHERNET_PORT = "1";
const std::string PORT_ROLE_SERVICE = "2";
const std::string PORT_ROLE_MANAGE_SERVICE = "3";
const std::string PORT_ROLE_REPLICATION = "4";
const std::string RUNNINGSTATUS_LINKUP = "10";


const std::string illegalChar = ".-:/@?+()<>$!~#%^*&|,;";
const int MAX_LENGTH = 31;
const int KB = 1024;
const int TWO = 2;
const long long MAX_BITMAP_LENGTH_ONCE = 256;
const long long ONE_MB = 1024 * 1024;
const int SINGLE_WAY = 1;
const int DO_NOT_USE = 0;
const unsigned long long CAPACITY_COEFFICIENT = 512;
const int LOGIC_PORT_IS_SERVICE = 2;
const int LOGIC_PORT_IS_MANAGE_AND_SERVICE = 3;
constexpr int FUSION_STORAGE_SNAPSHOP_STATUS_NORMAL = 0;
constexpr int FUSION_STORAGE_SNAPSHOP_STATUS_REVERTING = 16;
constexpr int DORADO_ERROR_CODE_OK = 0; // no any error happen when calling Dorado rest interface.

#endif
