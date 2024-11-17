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
#include "ThriftServer.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "message/tcp/CSocket.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::server;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

ThriftServer::ThriftServer()
{
}

ThriftServer::~ThriftServer()
{
    if (m_serverPtr.get()) {
        m_serverPtr->stop();
        m_serverPtr.reset();
    }
    if (m_threadPtr.get()) {
        m_threadPtr->join();
    }
}

static void SetFDCloseOnExec(int32_t fd)
{
#ifndef WIN32
    DBGLOG("SetFDCloseOnExec enter handler = %d\n.", fd);
    mp_int32 iFlag = fcntl(fd, F_GETFD);
    if (iFlag == MP_FAILED) {
        ERRLOG("fcntl failed! handler = %d\n.", fd);
        return;
    }
    iFlag = iFlag | FD_CLOEXEC;
    auto iRet = fcntl(fd, F_SETFD, iFlag);
    if (iRet == MP_FAILED) {
        ERRLOG("fcntl failed! handler = %d\n.", fd);
        return;
    }
#endif
}

ThriftServer* ThriftServer::GetInstance()
{
    static ThriftServer m_server;
    return &m_server;
}

std::shared_ptr<TSSLSocketFactoryPassword> ThriftServer::createServerSocketFactory(const std::string &certFilePath,
    const std::string &cipherkey)
{
    std::shared_ptr<TSSLSocketFactoryPassword> pServerSocketFactory;
    pServerSocketFactory = std::make_shared<TSSLSocketFactoryPassword>();
    pServerSocketFactory->ciphers(cipherkey);
    std::string certFile = certFilePath + CERT_FILE_NAME;
    pServerSocketFactory->loadCertificate(certFile.c_str());
    pServerSocketFactory->overrideDefaultPasswordCallback();
    std::string keyFile = certFilePath + PRIVATE_KEY_NAME;
    pServerSocketFactory->loadPrivateKey(keyFile.c_str());
    pServerSocketFactory->server(true);
    return pServerSocketFactory;
}

EXTER_ATTACK int ThriftServer::Init()
{
    SslConfigInfo sslConfig;
    NormalConfigInfo normalConfig;

    int32_t result = CConfigXmlParser::GetInstance().GetValueInt32(CFG_THRIFT_SECTION,
        CFG_THRIFT_SERVER_PORT, normalConfig.port);
    if (result != MP_SUCCESS) {
        WARNLOG("Get thrift server port error.");
    }

    // 先判断端口是否被占用
    mp_socket sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    result = CSocket::Bind(sock, "127.0.0.1", normalConfig.port);
    if (result != MP_SUCCESS) {
        ERRLOG("Port[%d] is in use", normalConfig.port);
        return result;
    } else {
        CSocket::Close(sock);
    }

    result = CConfigXmlParser::GetInstance().GetValueInt32(CFG_THRIFT_SECTION,
        CFG_THRIFT_MAX_THREAD_SIZE, normalConfig.maxConnections);
    if (result != MP_SUCCESS) {
        WARNLOG("Get thrift max thread size error.");
    }

    sslConfig.sslFlag = 1;

    if (sslConfig.sslFlag) {
        sslConfig.certPath = CPath::GetInstance().GetConfFilePath(DEFAULT_THRIFT_SERVER_PATH);
        result = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION,
            CFG_ALGORITHM_SUITE, sslConfig.algorithmSuite);
        if (result != MP_SUCCESS) {
            ERRLOG("Get thrift ssl algorithm suite error.");
            return result;
        }
    }

    return this->Init(normalConfig, sslConfig);
}

int32_t ThriftServer::Init(const NormalConfigInfo &normalConfig, const SslConfigInfo &sslConfig)
{
    try {
        INFOLOG("agent service start, ip:%s port:%d", DEFAULT_LISTEN_IP.c_str(), normalConfig.port);
        std::shared_ptr<BSAServiceHandler> handler = std::make_shared<BSAServiceHandler>();
        std::shared_ptr<TProcessor> processor = std::make_shared<BSAServiceProcessor>(handler);
        std::shared_ptr<TNonblockingServerTransport> serverTransport;
        if (sslConfig.sslFlag) {
            std::shared_ptr<TSSLSocketFactory> pServerSocketFactory =
                createServerSocketFactory(sslConfig.certPath, sslConfig.algorithmSuite);
            auto socket = std::make_shared<TNonblockingSSLServerSocket>(DEFAULT_LISTEN_IP,
                normalConfig.port, pServerSocketFactory);
            socket->setListenCallback(SetFDCloseOnExec);
            serverTransport = socket;
        } else {
            auto socket = std::make_shared<TNonblockingServerSocket>(DEFAULT_LISTEN_IP, normalConfig.port);
            socket->setListenCallback(SetFDCloseOnExec);
            serverTransport = socket;
        }

        std::shared_ptr<TProtocolFactory> protocolFactory = std::make_shared<TBinaryProtocolFactory>();
        m_serverPtr = std::make_shared<TNonblockingServer>(processor, protocolFactory, serverTransport);

        m_serverPtr->setOverloadAction(T_OVERLOAD_DRAIN_TASK_QUEUE);
        m_serverPtr->setNumIOThreads(normalConfig.maxConnections);
        m_serverPtr->setResizeBufferEveryN(0);
        m_serverPtr->setIdleReadBufferLimit(0);
        m_serverPtr->setIdleWriteBufferLimit(0);
        m_serverPtr->setMaxFrameSize(DEFAULT_FRAMED_SIZE);
        m_serverPtr->setWriteBufferDefaultSize(DEFAULT_WRITE_BUFFER_SIZE);
        // 启动线程
        apache::thrift::concurrency::ThreadFactory factory;
        factory.setDetached(false);
        std::shared_ptr<apache::thrift::concurrency::Runnable> serverThreadRunner(m_serverPtr);
        m_threadPtr = factory.newThread(serverThreadRunner);
        m_threadPtr->start();
    } catch (TException& tx) {
        ERRLOG("thrift Server Init failed, %s", tx.what());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}