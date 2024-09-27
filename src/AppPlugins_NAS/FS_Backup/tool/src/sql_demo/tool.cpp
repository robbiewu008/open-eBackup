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
#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include "Path.h"
#include "Utils.h"
#include "backup_layout/SqliteOps.h"

using namespace std;
using namespace Module;

namespace {
// DBFILE is the fulldirpath of nas share including DB file name
std::string DBFILE = "copymetadata.sqlite";
std::string META_PATH = "/home/sqlite/";
const string LOG_PATH = "/home/sqlite/log/";
const string LOG_NAME = "sql_demo.log";
const int CREATE_DB_FLAGS = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
const int READ_DB_FLAGS = SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX;
std::string g_sqliteDBRootPath = META_PATH;
std::string g_sqliteDBAliasPath = "/home/sqlitealias/";
}  // namespace

#define TEST_INT_TO_STRING(i, s) \
    do {                         \
        std::stringstream ss;    \
        ss << i;                 \
        ss >> s;                 \
    } while (0);

void InitLog(int argc, char **argv)
{
    CPath::GetInstance().Init(argv[0]);
    int iLogLevel = 0;
    int iLogCount = 100;
    int iLogMaxSize = 30;
    unsigned int runShellType = 1;
    CLogger::GetInstance().Init(LOG_NAME.c_str(), LOG_PATH);
    CLogger::GetInstance().SetLogConf(0, 100, 100);
    CLogger::GetInstance().SetLogLevel(OS_LOG_DEBUG);
    cout << "Log path: " << LOG_PATH << endl;
    return;
}

void PrintSqlInfo(std::vector<IndexDetails> vecIndexInfo)
{
    DBGLOG("Enter PrintSqlInfo  vecIndexInfo.size(): %u", vecIndexInfo.size());
    uint32_t i = 0;
    for (i = 0; i < vecIndexInfo.size(); i++) {
        DBGLOG("Index %d", i);
        DBGLOG("m_actualFileName %s", vecIndexInfo[i].m_actualFileName.c_str());
        DBGLOG("m_aggregateFileName %s", vecIndexInfo[i].m_aggregateFileName.c_str());
        DBGLOG("m_metaFileName %s", vecIndexInfo[i].m_metaFileName.c_str());
        DBGLOG("m_type %s", vecIndexInfo[i].m_type.c_str());
        DBGLOG("m_actualFileSize %ld", vecIndexInfo[i].m_actualFileSize);
        DBGLOG("m_offset %ld", vecIndexInfo[i].m_offset);
        DBGLOG("m_aggregatedFileSize %ld", vecIndexInfo[i].m_aggregatedFileSize);
        DBGLOG("m_createTime %ld", vecIndexInfo[i].m_createTime);
        DBGLOG("m_modifyTime %ld", vecIndexInfo[i].m_modifyTime);
    }
    DBGLOG("Exit PrintSqlInfo ");
}

std::shared_ptr<Module::Snowflake> g_idGenerator{nullptr};
std::shared_ptr<SQLiteDB> gsqliteDb{nullptr};

int DemoSqliteWrite(std::vector<IndexDetails> &indperf)
{
    time_t start, ending;
    int32_t ret = 0;

    time(&start);
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr = gsqliteDb->PrepareDb(DBFILE, CREATE_DB_FLAGS);
    if (sqlInfoPtr == nullptr) {
        cout << "PrepareDb  failed " << endl;
        return -1;
    }

    cout << "just before lock" << endl;
    std::lock_guard<std::mutex> lock(sqlInfoPtr->m_sqliteCoreInfoMutex);
    cout << "just after lock" << endl;
    sqlite3_stmt *sqlStmt = nullptr;
    ret = gsqliteDb->PrepareSqlStmt(DBFILE, &sqlStmt, sqlInfoPtr);
    if (ret != 0) {
        cout << "PrepareSqlStmt  failed " << endl;
        return ret;
    }

    for (auto &item : indperf) {
        ret = item.InsertIndexInfo(gsqliteDb, sqlStmt, sqlInfoPtr);
        if (ret != 0) {
            cout << "InsertIndexInfo failed" << endl;
            return ret;
        }
    }
    ret = gsqliteDb->FinalizeSqlStmt(sqlStmt, sqlInfoPtr);
    if (ret != 0) {
        cout << "CommitTrans failed" << endl;
        return ret;
    }
    time(&ending);
    cout << "InsertIndexInfo time: " << difftime(ending, start) << " seconds " << endl;
    return 0;
}

int DemoSqliteQuery(IndexDetails &indperf)
{
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr = gsqliteDb->PrepareDb(DBFILE, READ_DB_FLAGS);
    if (sqlInfoPtr == nullptr) {
        cout << "PrepareDb  failed " << endl;
        return -1;
    }

    std::vector<IndexDetails> vecIndexInfo{};
    cout << "QueryIndexInfoByName with proper name" << endl;
    int ret = indperf.QueryIndexInfoByName(gsqliteDb, DBFILE, vecIndexInfo, indperf.m_actualFileName, sqlInfoPtr);
    if (ret != 0) {
        cout << "QueryIndexInfoByName  failed " << endl;
        return ret;
    }

    PrintSqlInfo(vecIndexInfo);
    return 0;
}

void *myThreadFun1(void *vargp)
{
    std::vector<IndexDetails> indperfVec{};
    for (int i = 0; i < 1000; i++) {
        IndexDetails indperf{};
        TEST_INT_TO_STRING(i, indperf.m_uuid);
        indperf.m_type = "f";
        std::string a;
        TEST_INT_TO_STRING(i, a);
        indperf.m_actualFileName = "normalFile" + a + ".txt";
        indperf.m_actualFileSize = 57;
        indperf.m_metaFileName = "metaFile_0";
        indperf.m_offset = i;
        indperf.m_aggregateFileName = "AggFile" + a + ".ztip";
        indperf.m_aggregatedFileSize = 500;
        indperf.m_createTime = i * 2;
        indperf.m_modifyTime = i * 2;
        indperf.m_metaData = "filemeta" + a;
        indperfVec.emplace_back(indperf);
    }
    DemoSqliteWrite(indperfVec);
    return nullptr;
}

void *myThreadFun2(void *vargp)
{
    std::vector<IndexDetails> indperfVec{};
    for (int i = 1001; i < 2000; i++) {
        IndexDetails indperf{};
        TEST_INT_TO_STRING(i, indperf.m_uuid);
        indperf.m_type = "f";
        std::string a;
        TEST_INT_TO_STRING(i, a);
        indperf.m_actualFileName = "normalFile" + a + ".txt";
        indperf.m_actualFileSize = 57;
        indperf.m_metaFileName = "metaFile_0";
        indperf.m_offset = i;
        indperf.m_aggregateFileName = "AggFile" + a + ".ztip";
        indperf.m_aggregatedFileSize = 500;
        indperf.m_createTime = i * 2;
        indperf.m_modifyTime = i * 2;
        indperfVec.emplace_back(indperf);
    }
    DemoSqliteWrite(indperfVec);
    return nullptr;
}

int QueryAllSqlInfo()
{
    time_t start, ending;
    time(&start);
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr = gsqliteDb->PrepareDb(DBFILE, READ_DB_FLAGS);
    if (sqlInfoPtr == nullptr) {
        cout << "PrepareDb  failed main thread " << endl;
        return -1;
    }

    IndexDetails indperf{};
    std::vector<IndexDetails> perfIndexInfo{};
    int ret = indperf.QueryAllIndexInfo(gsqliteDb, DBFILE, perfIndexInfo, sqlInfoPtr);
    if (ret != 0) {
        cout << "QueryIndexInfoByName  failed  " << endl;
        return -1;
    }

    time(&ending);
    cout << "Query  time  " << difftime(ending, start) << " seconds " << endl;
    PrintSqlInfo(perfIndexInfo);
    return 0;
}

int ParsePara(int argc, char **argv)
{
    for (int i = 1; (i < argc) && (i + 1 < argc); ++i) {
        if (std::string(argv[i]) == "-d") {  // backup destination
            META_PATH = std::string(argv[i + 1]);
            ++i;
        } else {
            std::cout << "Error para" << endl;
            return -1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    int32_t ret = 0;
    ParsePara(argc, argv);
    InitLog(argc, argv);

    // MT threads
    g_idGenerator = make_shared<Snowflake>();
    g_idGenerator->SetMachine(Module::GetMachineId());
    gsqliteDb = make_shared<SQLiteDB>(g_sqliteDBRootPath, g_sqliteDBAliasPath, g_idGenerator);
    pthread_t thread_id1, thread_id2;
    printf("Before Thread1\n");
    pthread_create(&thread_id1, NULL, myThreadFun1, NULL);
    printf("after Thread1\n");
    printf("Before Thread2\n");
    pthread_create(&thread_id2, NULL, myThreadFun2, NULL);
    printf("After Thread2\n");
    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);
    printf("Finalll Thread\n");
    QueryAllSqlInfo();

    IndexDetails ind{};
    ind.m_uuid = "1";
    ind.m_actualFileName = "normalFile1.txt";
    DemoSqliteQuery(ind);

    return 0;
}
