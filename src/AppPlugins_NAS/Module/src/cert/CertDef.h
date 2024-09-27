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
#ifndef _CERT_DEF_H_
#define _CERT_DEF_H_
#include <vector>
//证书类型
enum CERT_TYPE_IN_CERT
{
    CERT_TYPE_PROTECTED_ENV_IN_CERT = 0,    //受保护环境类型
    CERT_TYPE_STORAGE_UNIT_IN_CERT,         //存储单元类型
    CERT_TYPE_EMAIL_NOTIFY_IN_CERT,         //邮件通知类型
    CERT_TYPE_OPENSTACK_IN_CERT,
    CERT_TYPE_BACKUP_SERVER_IN_CERT,
    CERT_TYPE_FTPS_AUTH_IN_CERT = 5,
    CERT_TYPE_ARBITRATION_IN_CERT = 6,
    CERT_TYPE_OC_IN_CERT = 7,
    CERT_TYPE_BACKUP_MANAGE_IN_CERT,
    CERT_TYPE_UNKNOW_IN_CERT = 255          //未知类型
};

enum CERT_SEND_EVENT_IN_CERT
{
    DO_NOT_SEND_EVENT_IN_CERT = 0,          //不发告警
    SEND_EVENT_IN_CERT = 1                  //发告警
};

const unsigned int MAX_CERT_FILE_LENGTH_IN_CERT = 1024*1024;    //OC证书文件最大1MB字节


enum  EN_PROTOCOL_TYPE
{
    HTTP_PROTOCOL = 0,                      //HTTP
    HTTPS_PROTOCOL,                         //HTTPS
    BUTTOM_BUTT,
};


class ServerInfo
{
public:
    virtual ~ServerInfo() = default;
	ServerInfo(): clientCert("none"),protocolType(HTTPS_PROTOCOL),clientPort(63234){}
	std::string clientCert; 
	EN_PROTOCOL_TYPE  protocolType;
	std::string clientIp;                   //Input by the User.
    std::string clientUsername;             //Input by the User.
    std::string clientPassword;             //Input by the User.
	int  clientPort = 0;                        //Get from the configuration file.
public:
    const std::string &ClientCert() const { return clientCert; }
    const EN_PROTOCOL_TYPE &ProtocolType() const { return protocolType; }
    const std::string &ClientIP() const { return clientIp; }
    const std::string &ClientUsername() const { return clientUsername; }
    const std::string &ClientPassword() const { return clientPassword; }
    const int &ClientPort() const { return clientPort; }
};

struct CertWithCRL
{
    std::string cert;
    std::string crl;
};

class OC_INFO:public ServerInfo
{
public:
    std::string clientUrn;                                              
    std::string clientUri;                                             
    std::string clientName;                                         
    int clientType;                                                    
    bool isSelfSite;                                                                          
    long vmExpTime;                                                                                                            
    std::string subscribeUrn;
    std::string subscribeUri;
    std::vector<CertWithCRL> certList;
    bool isNewVersion;
    OC_INFO()
    {
        
        clientUrn="";
        clientUri="";
        clientName="";      
        subscribeUrn="";
        subscribeUri="";
        clientPort = 0;
        vmExpTime = 0;
        clientType = 0;
        isSelfSite = false;
        isNewVersion = true;
        certList.clear();
    }

    std::string _GetClientIp()const {return clientIp;}
	
    int _GetClientPort()const{return clientPort;}

    std::vector<CertWithCRL> _GetCertList() const { return certList; }

    bool _GetIsNewVersion()const {return isNewVersion;}
	
    virtual ~OC_INFO(){}
};

class AUTH_INFO:public ServerInfo
{
public:
    std::string authType;
    std::string clientUri;   
    std::vector<CertWithCRL> certList;
    
    AUTH_INFO()
    {
        authType = "";
        clientUri="";
        certList.clear();
    }

   	std::string _GetAuthType()const {
		return authType;
	}

   	std::string _GetClientUri()const {
		return clientUri;
	}
    std::vector<CertWithCRL> _GetCertList() const
    {
        return certList;
	}
	
	
    virtual ~AUTH_INFO(){}
};


#endif
