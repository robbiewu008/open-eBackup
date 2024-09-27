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