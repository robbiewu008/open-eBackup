#ifndef __THRIFTSERVER_H
#define __THRIFTSERVER_H

#include "XBSAServer/BSAServiceHandler.h"
#include "XBSACom/TSSLSocketFactoryPassword.h"

using namespace apache::thrift::transport;
namespace {
    const std::string DEFAULT_THRIFT_SERVER_PATH = "thrift/server";
    const std::string DEFAULT_LISTEN_IP = "127.0.0.1";
    const std::string CERT_FILE_NAME = "/client.crt.pem";
    const std::string PRIVATE_KEY_NAME = "/client.pem";
    const int DEFAULT_PORT = 59560;
    const int DEFAULT_IOTHREAD_SIZE = 1024;
    const unsigned int DEFAULT_FRAMED_SIZE = 1024 * 1024;
    const unsigned int DEFAULT_WRITE_BUFFER_SIZE = 1024 * 1024;
}

class ThriftServer {
public:
struct SslConfigInfo {
    SslConfigInfo()
    {
        sslFlag = 1;
        certPath = "";
        algorithmSuite = "";
    }

    int32_t sslFlag;
    std::string certPath;
    std::string algorithmSuite;
};

struct NormalConfigInfo {
    NormalConfigInfo()
    {
        port = DEFAULT_PORT;
        maxConnections = DEFAULT_IOTHREAD_SIZE;
    }

    int32_t port;
    int32_t maxConnections;
};

public:
    ThriftServer();
    static ThriftServer *GetInstance();
    EXTER_ATTACK int Init();
private:
    ~ThriftServer();
    std::shared_ptr<apache::thrift::server::TNonblockingServer> m_serverPtr;
    std::shared_ptr<apache::thrift::concurrency::Thread> m_threadPtr;
    int32_t Init(const NormalConfigInfo &normalConfig, const SslConfigInfo &sslConfig);
    std::shared_ptr<TSSLSocketFactoryPassword> createServerSocketFactory(const std::string &certFilePath,
        const std::string &cipherkey);
    void CheckAppIsUsedSockFile();
    void InitServerTransport(std::shared_ptr<TNonblockingServerTransport> &serverTransport, std::string &sockFilePath,
                             const NormalConfigInfo &normalConfig,
                             const SslConfigInfo &sslConfig);
    bool m_isUseSockFile = false;
};

#endif