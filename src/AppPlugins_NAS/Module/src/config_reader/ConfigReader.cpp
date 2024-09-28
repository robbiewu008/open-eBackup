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
#include "ConfigIniReader.h"
#include "ConfigIniValue.h"
#include "ConfigIniReaderImpl.h"
#include "log/Log.h"
#include "define/Types.h"
#include "define/Defines.h"

using namespace std;

namespace Module {

static string ipv4_regular_expression = "((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]\\d?)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]\\d?|0)){3})";

static string ipv6_regular_expression = "((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:)))(%.+)?";

int ConfigReader::getInt(const string & sectionName, const string & keyName,bool logFlag)
{
    return ConfigReaderImpl::instance()->getInt(sectionName, keyName,logFlag);
}
unsigned int ConfigReader::getUint(const string & sectionName, const string & keyName)
{
    int val = 0;

    val = ConfigReaderImpl::instance()->getInt(sectionName, keyName);
    if(val < 0) {
        HCP_Log(ERR, MODULE_NAME) << "config: " << sectionName << " value maybe is wrong" << HCPENDLOG;
        return 0;
    } else {
        return (unsigned int)val;
    }
}

string ConfigReader::getString(const string & sectionName, const string & keyName, bool logFlag)
{
    return ConfigReaderImpl::instance()->getString(sectionName, keyName,logFlag);
}

//Get management IP
string ConfigReader::getManagementIP()
{
    return ConfigReader::getPlaneIP("General", "HCPManagementPlane");
}

//Get internal IP
string ConfigReader::getInternalIP()
{
    return ConfigReader::getPlaneIP("General", "HCPInternalPlane");
}

//Get protected environment management IP
string ConfigReader::getPEManagementIP()
{
    return ConfigReader::getPlaneIP("General", "ProtectedEnvironmentManagementPlane");
}

//Get Storage IP
string ConfigReader::getPEStorageIP()
{
    return ConfigReader::getPlaneIP("General", "ProtectedEnvironmentStoragePlane");
}

//Get BackupStorage IP
string ConfigReader::getBackupStorageIP()
{
    return ConfigReader::getPlaneIP("General", "BackupStoragePlane");
}

string ConfigReader::AdminNodeGetLeaderIP()
{
#ifndef NATIVE_PROFILE
    string leaderIPs = ConfigReader::getString("BackupNode", "CurrentLeaderIP");
    string internalIP = ConfigReader::getPlaneIP("General", "HCPInternalPlane"); 
    size_t pos = leaderIPs.find(',');
    if (pos == string::npos) {
        return leaderIPs;
    }
    string firstLeaderIP = leaderIPs.substr(0,pos);
    string secondLeaderIP = leaderIPs.substr(pos+1);

    pos = internalIP.find(firstLeaderIP);
    if (pos != string::npos) {
        return firstLeaderIP;
    }
    pos = internalIP.find(secondLeaderIP);
    if (pos != string::npos) {
        return secondLeaderIP;
    } else {
        return "";
    }
#else
 return ConfigReader::GetLeaderIP();
#endif
}
string ConfigReader::GetLeaderIP()
{
    return ConfigReader::getString("General", "CurrentLeaderIP");
}

string ConfigReader::getLoadbalanceIP()
{
    return "";
}

string ConfigReader::GetPrimaryAndStandbyIP()
{
    return ConfigReader::getString("General", "PrimaryAndStandbyIP");
}

string ConfigReader::GetEbkMgrBindPort()
{
    int bindPort = ConfigReader::getInt("BackupNode", "HeartBeatDefaultBindPort");
    stringstream ss;
    ss<<bindPort;
    return ss.str();
}

string ConfigReader::GetEbkAdminBindPort()
{
    int bindPort = ConfigReader::getInt("AdminNode", "HeartBeatServerPort");
    stringstream ss;
    ss<<bindPort;
    return ss.str();
}

string ConfigReader::getAdminNodeIAMUser()
{
    string userName = ConfigReader::getString("MicroService", "IAMUser");
    return userName;
}

string ConfigReader::getAdminNodeIAMPasswd()
{
    string pa = ConfigReader::getString("MicroService", "IAMPasswd");
    return pa;
}

string ConfigReader::getBackupNodeIAMUser()
{
#ifndef NATIVE_PROFILE
    string userName = ConfigReader::getString("BackupNode", "IAMUser");
#else
    string userName = ConfigReader::getString("MicroService", "IAMUser");
#endif
    return userName;
}

string ConfigReader::getBackupNodeIAMPasswd()
{
#ifndef NATIVE_PROFILE
    string pa = ConfigReader::getString("BackupNode", "IAMPasswd");
#else
    string pa = ConfigReader::getString("MicroService", "IAMPasswd");
#endif

    return pa;
}

void ConfigReader::destroy()
{
    ConfigReaderImpl::destroy();
}

string ConfigReader::getPlaneIP(const string & sectionName, const string & keyName)
{
    return "";

}


void ConfigReader::getStringValueSet(const string& sectionName, const string& keyName, set<string>& t)
{
    string values=ConfigReader::getString(sectionName, keyName);
    string delim = ",";
    size_t last = 0;
    size_t index=values.find_first_of(delim,last);
    string value;
    while (index != string::npos) {
        value = values.substr(last,index-last);

        if (!value.empty()) {
           t.insert(value);
        }
        last=index+1;
        index=values.find_first_of(delim,last);
    }
    if(index-last>0) {
        value = values.substr(last,index-last);

        if(!value.empty()) {
            t.insert(value);
        }
    }

    HCP_Logger_noid(DEBUG, MODULE_NAME) <<
        "Get the number of "<< keyName << ":" << t.size() << HCPENDLOG;

    //DEBUG
    for (auto it : t) {
      HCP_Logger_noid(DEBUG, MODULE_NAME) << it << HCPENDLOG;
    }

    return;
}

void ConfigReader::getIntValueSet(const string& sectionName, const string& keyName, const string &delim, set<int>& t)
{
    string values = ConfigReader::getString(sectionName, keyName);
    size_t startFindPos = 0;
    size_t delimPos = values.find_first_of(delim, startFindPos);
    string value;
    while (delimPos != string::npos) {
        value = values.substr(startFindPos, delimPos - startFindPos);

        if (!value.empty()) {
           t.insert(atoi(value.c_str()));
        }
        startFindPos = delimPos + delim.size();
        delimPos = values.find_first_of(delim, startFindPos);
    }
    if (delimPos - startFindPos > 0) {
        value = values.substr(startFindPos, delimPos - startFindPos);
        if (!value.empty()) {
            t.insert(atoi(value.c_str()));
        }
    }
    return;
}

//Get OceanStor disk array not retry error code
set<int> ConfigReader::getOceanStorDiskArrayNotRetryErrorCode()
{
    set<int> NotRetryErrorCode;
    getIntValueSet("BackupNode", "NotRetryErrorCodeForDiskArray", ",", NotRetryErrorCode);

    return NotRetryErrorCode;
}

void ConfigReader::setIntConfigInfo(const string & sectionName, const string & keyName, int minValue, int maxValue, int defaultValue){
    ConfigReaderImpl::instance()->putIntConfigInfo(sectionName, keyName, minValue, maxValue, defaultValue);
}

void ConfigReader::setStringConfigInfo(const string & sectionName, const string & keyName, const string & defaultValue){
    ConfigReaderImpl::instance()->putStringConfigInfo(sectionName, keyName, defaultValue);
}
    
void ConfigReader::setPConfigInfo(const string & sectionName, const string & keyName, const string & defaultValue){
    ConfigReaderImpl::instance()->putIPConfigInfo(sectionName, keyName, defaultValue);
}

bool ConfigReader::checkConfigInfo(const string & sectionName, const string & keyName, const string & value, bool & isInt)
{
    return ConfigReaderImpl::instance()->checkConfigInfo(sectionName, keyName, value, isInt);
}

string ConfigReader::getConfigVersion()
{
    return ConfigReaderImpl::instance()->getConfigVersion();
}

void ConfigReader::getIntValueVector(
    const string& sectionName, const string& keyName, const string &delim, vector<int>& t)
{
    string values = ConfigReader::getString(sectionName, keyName);
    size_t startFindPos = 0;
    size_t delimPos = values.find_first_of(delim, startFindPos);
    string value;
    while(delimPos != string::npos) {
        value = values.substr(startFindPos, delimPos - startFindPos);
        if (!value.empty()) {
            t.push_back(atoi(value.c_str()));
        }
        startFindPos = delimPos  + delim.size();
        delimPos = values.find_first_of(delim, startFindPos);
    }
    if(delimPos - startFindPos > 0) {
        value = values.substr(startFindPos, delimPos - startFindPos);

        if (!value.empty()) {
            t.push_back(atoi(value.c_str()));
        }
    }
    return;
}

int ConfigReader::GetLogLevelFromK8s()
{
    return ConfigReaderImpl::instance()->GetLogLevelFromK8s();
}

bool ConfigReader::GetFileServerSslConfig()
{
    return ConfigReaderImpl::instance()->GetFileServerSslConfig();
}

void ConfigReader::ReadLogLevelFromPM(bool readLoglevelFromPM)
{
    return ConfigReaderImpl::instance()->ReadLogLevelFromPM(readLoglevelFromPM);
}

void ConfigReader::SetUpdateCnf(bool updateCnf)
{
    return ConfigReaderImpl::instance()->SetUpdateCnf(updateCnf);
}
} // namespace Module
