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
#ifndef __AGENT_REST_INTERFACES_H__
#define __AGENT_REST_INTERFACES_H__

namespace Module {
// URL Method
#define REST_URL_METHOD_GET                       "GET"
#define REST_URL_METHOD_PUT                       "PUT"
#define REST_URL_METHOD_POST                      "POST"
#define REST_URL_METHOD_DELETE                    "DELETE"
#define REST_URL_METHOD_PATCH                     "PATCH"

// URL
    static const char *REST_URI_VERSION = "/v1";

// error message body
#define REST_PARAM_ERROR_CODE                   "errorCode"

// message head key
    static const char *HTTPPARAM_DBUSERNAME = "HTTP_DBUSERNAME";
    static const char *HTTPPARAM_DBPASSWORD = "HTTP_DBPASSWORD";
    static const char *HTTPPARAM_ASMSERNAME = "HTTP_ASMUSERNAME";

    static const char *HTTPPARAM_ASMPASSWORD = "HTTP_ASMPASSWORD";
    static const char *HTTPPARAM_SNMPAUTHPW = "HTTP_AUTHPASSWORD";
    static const char *HTTPPARAM_SNMPENCRYPW = "HTTP_ENCRYPTPASSWORD";
    static const char *HTTP_HEAD_SUBJECT_TOKEN = "X-Subject-Token";
    static const char *HTTP_HEAD_CLIENT_IP = "X-Client-IP";
    static const char *HTTP_HEAD_TOKEN = "TOKEN";
    const std::string g_headerAuthorization = "Authorization";

// add for Session hold c00377603
#define HTTP_HEAD_SESSION_COOKIE                       "Cookie"
#define HTTP_HEAD_SET_COOKIE                           "Set-Cookie"
#define HTTP_HEAD_IP_ROUTE                             "IP-Route"
// end for Session hold
    static const char *HTTP_HEAD_X_FORWARDED_FOR = "X-FORWARDED-FOR";
    static const char *HTTP_HEAD_AUTH_TOKEN = "X-Auth-Token";
    static const char *HTTP_HEAD_SUBJECT_TOKEN_UPPER = "HTTP_X_SUBJECT_TOKEN";
    static const char *HTTP_HEAD_CLIENT_IP_UPPER = "HTTP_X_CLIENT_IP";
    static const char *HTTP_HEAD_AUTH_TOKEN_UPPER = "HTTP_X_AUTH_TOKEN";
    static const char *UNKNOWN = "Unknown";
    static const char *REMOTE_ADDR = "REMOTE_ADDR";
    static const char *HTTP_X_FORWARDED_FOR = "HTTP_X_FORWARDED_FOR";
    static const char *REQUEST_URI = "REQUEST_URI";
    static const char *REQUEST_METHOD = "REQUEST_METHOD";
    static const char *CONTENT_LENGTH = "CONTENT_LENGTH";
    static const char *QUERY_STRING = "QUERY_STRING";
    static const char *STATUS = "Status";
    static const char *CONTENT_TYPE = "CONTENT_TYPE";
    static const char *CACHE_CONTROL = "Cache-Control";
    static const char *CONTENT_ENCODING = "Content-Encoding";
    static const char *UNAME = "HTTP_X_AUTH_USER";
    static const char *PW = "HTTP_X_AUTH_KEY";
    static const char *LISTEN_ADDR = "SERVER_ADDR";

// added for replication
    static const char *HTTP_HEAD_USER_TYPE = "X-UserType";


// IAM REST param
#define REST_PARAM_IAM_AUTH                       "auth"
#define REST_PARAM_IAM_IDENTITY                   "identity"
#define REST_PARAM_IAM_PASSWORD                   "password"
#define REST_PARAM_IAM_TICKET                     "ticket"
#define REST_PARAM_IAM_USER                       "user"
#define REST_PARAM_IAM_NAME                       "name"
#define REST_PARAM_IAM_SCOPE_TYPE                 "iam_type"
#define REST_PARAM_IAM_TENANT_ID                  "tenant_id"
#define REST_PARAM_IAM_DOMAIN_ID                  "domain_id"
#define REST_PARAM_IAM_EXPIRE                     "expires_at"
#define REST_PARAM_IAM_SESSION                    "id"
#define REST_PARAM_IAM_TOKEN                      "token"
#define REST_PARAM_IAM_Auth_Token                 "X-Auth-Token"
#define REST_PARAM_IAM_METHOD                     "methods"
#define REST_PARAM_IAM_REMOTE_IP                  "remote_ip"
#define REST_PARAM_USER_SCOPE                     "Scope"
#define REST_PARAM_USER_LEVEL                     "Level"
#define REST_PARAM_USER_DATA                      "data"
#define REST_PARAM_USER_ERROR                     "error"
#define REST_PARAM_USER_CODE                      "code"
#define REST_PARAM_USER_DESCRIPT                  "description"
#define REST_PARAM_USER_ID                        "Id"
#define REST_PARAM_USER_TYPE                      "TYPE"
#define REST_PARAM_USER_NAME                      "Name"
#define REST_PARAM_USER_PASSWORD                  "Password"
#define REST_PARAM_USER_ISONLINE                  "ISONLINE"
#define REST_PARAM_USER_CREATETIME                "CREATETIME"
#define REST_PARAM_USER_LOCKSTATUS                "LOCKSTATUS"
#define REST_PARAM_USER_SESSIONMODE               "SessionMode"
#define REST_PARAM_USER_SESSIONNUM                "SessionNum"
#define REST_PARAM_USER_LOCK_INFO                 "lockInfo"
#define REST_PARAM_USER_LOCK_IP_INFO              "lock_ip_info"
#define REST_PARAM_USER_LOCK_IP                   "ip"

#define REST_PARAM_IAM_ROLE                       "roles"
#define REST_PARAM_IAM_EXTRAS                     "extras"
#define REST_PARAM_IAM_AUDIT_ID                   "audit_ids"   // A list of one or two audit IDs.
#define REST_PARAM_IAM_ISSUED                     "issued_at"   // The date and time when the token was issued.
#define REST_PARAM_IAM_DOMAIN                     "domain"
#define REST_PARAM_IAM_CONDITION                  "Condition"   // query condition
#define REST_RESPONSE_IAM_USERS                   "Users"       // query result
#define REST_RESPONSE_IAM_COUNT                   "Count"       // query count result
#define REST_RESPONSE_IAM_CERT_CONTENT            "ClientCert"  // IAM client cert
#define REST_RESPONSE_IAM_CACERT_CONTENT          "CACert"      // Openstack IAM server ca cert
#define REST_RESPONSE_IAM_TOKEN_BINDING_IP        "ip"          // Token binding IP
#define REST_RESPONSE_IAM_TOKEN_SIGN              "sign"        // Token's signature
#define REST_RESPONSE_IAM_ACCESS_IP               "access_ip"
}

#endif // __AGENT_REST_INTERFACES_H__

