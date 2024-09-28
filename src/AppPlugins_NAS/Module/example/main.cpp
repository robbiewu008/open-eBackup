#include <iostream>
#include "Log.h"
#include "common/JsonHelper.h"
#include "ConfigIniReader.h"
#include "ConfigIniReaderImpl.h"
#include "common/Path.h"

#include "Log.h"

using namespace std;
using namespace Module;

int main(int argc, char** argv)
{
    CLogger::GetInstance().Init("test.log", "/tmp");
    cout << "hello world" << endl;

    ERRLOG("Get DmeClient Ins failed. jobId=%d.", 123);
    HCP_Log(INFO, "test") << "Get CreateFactory success" << HCPENDLOG;

    CLogger::GetInstance().Log(INFO - 1, 1, "main.cpp", "main", "hello log");

    CLogger::GetInstance().SetLogConf(0, 100, 100); // 手动设置

    INFOLOG("bin path: %s", argv[0]);
    DBGLOG("bin path: %s", argv[0]);
    int ret = CPath::GetInstance().Init(argv[0]);
    if (ret != 0) {
        return false;
    }
    string rootPath = CPath::GetInstance().GetRootPath();
    INFOLOG("bin path: %s", rootPath.c_str());
    printf("config reader\n");
    int logLevel = ConfigReader::getInt("General", "LogLevel");
    INFOLOG("1 logLevel: %d", logLevel);
    printf("1 logLevel: %d\n", logLevel);

    std::string conf = rootPath + "/conf/hcpconf.ini";
    INFOLOG("conf: %s", conf.c_str());
    printf("1 logLevel: %d\n", logLevel);

    // ConfigReaderImpl::instance()->refresh({conf});
    logLevel = ConfigReader::getInt("General", "LogLevel");
    printf("2 logLevel: %d\n", logLevel);
    INFOLOG("2 logLevel: %d", logLevel);
    DBGLOG("3 logLevel: %d", logLevel);

    cout << "hello world" << endl;
    return 0;
}
