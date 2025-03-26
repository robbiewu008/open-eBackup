#ifdef WIN32
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <exception>
#include "Log.h"
#include "File.h"
#include "Path.h"
#include "ScanMgr.h"
#include "ScannerSerializableConfig.h"

using namespace Module::FileSystemUtil;

extern void InitLog();
extern bool StartScanner(const std::string& scanConfigJsonPath, std::shared_ptr<Scanner> &scanner,
                         const std::vector<std::string> enqueuePathList);
extern void MonitorScanner(std::shared_ptr<Scanner> &scanner);

static bool CheckScanConfigPathW(const std::wstring& wScanConfigJsonPath)
{
    if (wScanConfigJsonPath.empty()) {
        std::cout << "Need To Specify ScanConfigPath!" << std::endl;
        return false;
    }
    std::wcout << L"Using scanConfig Path: " << wScanConfigJsonPath << std::endl;
    std::string scanConfigJsonPath = Utf16ToUtf8(wScanConfigJsonPath);
    if (!Module::CFile::DirExist(scanConfigJsonPath.c_str())) {
        std::cout << "ScanConfigPath Invalid." << std::endl;
        return false;
    }
    return true;
}

int wmain(int argc, wchar_t** wargv)
{
    std::wstring wScanConfigJsonPath{};
    std::vector<std::string> enqueuePathList{};
    std::shared_ptr<Scanner> scanner = nullptr;
    std::cout << "Start Windows Scanner Demo" << std::endl;
    if (argc < 3) {
        std::cout << "Need at least 2 params, usage: ScannerDemo -c scanConfig.json <enqueuePathList...> -l <logpath>" << std::endl;
    }
    for (int i = 1; i < argc; ++i) {
        if (std::wstring(wargv[i]) == L"-c" && i + 1 < argc) {
            wScanConfigJsonPath = std::wstring(wargv[i + 1]);
            ++i;
        }
        else if (std::wstring(wargv[i]) == L"-l" && i + 1 < argc) { // log path
            LOG_PATH = Utf16ToUtf8(std::wstring(wargv[i + 1]));
            ++i;
        }
        else if (std::wstring(wargv[i]) == L"-L" && i + 1 < argc) { // log level
            LOG_LEVEL_STR = Utf16ToUtf8(std::wstring(wargv[i + 1]));
            ++i;
        }
        else { /* Recognize As Enqueue List */
            std::wstring enqueuePathW = std::wstring(wargv[i]);
            std::wcout << "Enqueue: " << enqueuePathW << std::endl;
            enqueuePathList.push_back(Utf16ToUtf8(enqueuePathW));
        }
    }
    if (!InitLog()) {
        std::cout << "Init Log Failed" << std::endl;
        return -1;
    }
    if (!CheckScanConfigPathW(wScanConfigJsonPath)) {
        return -1;
    }
    if (!StartScanner(Utf16ToUtf8(wScanConfigJsonPath), scanner, enqueuePathList)) {
        ERRLOG("=== Start Scanner Failed ===");
        std::cout << "Start Scanner Failed" << std::endl;
        return -1;
    }
    MonitorScanner(scanner);
    if (scanner != nullptr) {
        ScanStatistics stats = scanner->GetStatistics();
        std::cout << "Totally Dirs: " << stats.mTotDirs << ", Files: " << stats.mTotFiles << std::endl;
        INFOLOG("Scanner Destroying!");
        scanner->Destroy();
        scanner.reset();
    }
    INFOLOG("=== Scanner Completed! ===");
    std::cout << "Scan Complete." << std::endl;
    return 0;
}

#endif