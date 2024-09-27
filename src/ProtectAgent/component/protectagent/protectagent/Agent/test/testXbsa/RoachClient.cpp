#include "RoachClient.h"
#include <string.h>
#include <memory>
#include "CommonDefine.h"
#include "FileTest.h"
#include <string.h>

RoachClient::RoachClient(const std::string libPath)
{
    m_libPath = libPath;
    m_parallelBackup = 0;
    m_parallelRecover = 0;
    m_parallelDel = 0;
    m_nameIndex = 0;
    m_newestObjectName = "";
    m_objectNamePrefix = "";
    m_currentBsaHandle = 0;
    m_operateFile = false;
    m_serialNum = "";
}

void RoachClient::initSymbol()
{
    int flag = (RTLD_NOW | RTLD_LOCAL);
    m_handle = dlopen(m_libPath.c_str(), flag);
    if (m_handle == NULL)
    {
        Log("open failed:%s", dlerror());
        return;
    }

    BSABeginTxn_fun = (SYMBOY_COMMON)dlsym(m_handle, "BSABeginTxn");
    BSAEndData_fun = (SYMBOY_COMMON)dlsym(m_handle, "BSAEndData");
    BSATerminate_fun = (SYMBOY_COMMON)dlsym(m_handle, "BSATerminate");
    BSACreateObject_fun = (SYMBOY_CREATE_OBJECT)dlsym(m_handle, "BSACreateObject");
    BSAInit_fun = (SYMBOY_BSAINIT)dlsym(m_handle, "BSAInit");
    BSADeleteObj_fun = (SYMBOY_DELETE_OBJ)dlsym(m_handle, "BSADeleteObject");
    BSAEndTxn_fun = (SYMBOY_END_TXN)dlsym(m_handle, "BSAEndTxn");
    BSAGetData_fun = (SYMBOY_GET_DATA)dlsym(m_handle, "BSAGetData");
    BSAGetEnv_fun = (SYMBOL_GET_ENV)dlsym(m_handle, "BSAGetEnvironment");
    BSAGetLastErr_fun = (SYMBOL_GET_LASTERR)dlsym(m_handle, "BSAGetLastError");
    BSAGetNexQueryObj_fun = (SYMBOL_GET_NEX_QUERY_OBJ)dlsym(m_handle, "BSAGetNextQueryObject");
    BSAGetObj_fun = (SYMBOL_GET_OBJ)dlsym(m_handle, "BSAGetObject");
    BSAQueryVerion_fun = (SYMBOL_GET_VERSION)dlsym(m_handle, "BSAQueryApiVersion");
    BSAQueryObj_fun = (SYMBOL_QUERY_OBJ)dlsym(m_handle, "BSAQueryObject");
    BSASendData_fun = (SYMBOL_SEND_DATA)dlsym(m_handle, "BSASendData");
    BSAQuerySerProvider_fun = (SYMBOL_QUERY_SERVICE_PRIVIDE)dlsym(m_handle, "BSAQueryServiceProvider");
}

int RoachClient::testBSABeginTxn(long handle)
{
    return BSABeginTxn_fun(handle);
}

int RoachClient::testBSAInit(void)
{
    long handle = 0;
    BSA_ObjectOwner objectOwner = {"roach", "roach"};
    char *env[6] = {NULL};
    for (int i = 0; i < 6; i++) {
        env[i] = (char *)malloc(60);
    }

    strcpy(env[0], "BSA_API_VERSION=1.1.0");

    int ret= BSAInit_fun(&handle, NULL, &objectOwner, env);
    Log("ret=%d,handle=%ld.",ret, handle);

    return ret;
}

int RoachClient::BSAInit(long &handle)
{
    BSA_ObjectOwner objectOwner = {"roach", "roach"};
    char *env[6] = {NULL};
    for (int i = 0; i < 6; i++) {
        env[i] = (char *)malloc(60);
    }

    strcpy(env[0], "BSA_API_VERSION=1.1.0");

    int ret= BSAInit_fun(&handle, NULL, &objectOwner, env);
    return ret;
}

void RoachClient::initBsaObjDesc(BSA_ObjectDescriptor &objDesc, std::string objName)
{
    memset(objDesc.objectOwner.app_ObjectOwner, 0, BSA_MAX_APPOBJECT_OWNER);
    memset(objDesc.objectOwner.bsa_ObjectOwner, 0, BSA_MAX_BSAOBJECT_OWNER);
    memset(objDesc.objectName.objectSpaceName, 0, BSA_MAX_OBJECTSPACENAME);
    memset(objDesc.objectName.pathName, 0, BSA_MAX_PATHNAME);
    memset(objDesc.resourceType, 0, BSA_MAX_RESOURCETYPE);

    memcpy(objDesc.objectOwner.app_ObjectOwner, "roach", 5);
    memcpy(objDesc.objectOwner.bsa_ObjectOwner, "roach", 5);
    memcpy(objDesc.objectName.objectSpaceName, "roach", 5);
    memcpy(objDesc.objectName.pathName, objName.c_str(), objName.size());
    memcpy(objDesc.resourceType, "resource", 8);

    objDesc.copyType = BSA_CopyType::BSA_CopyType_BACKUP;
    objDesc.objectType = BSA_ObjectType::BSA_ObjectType_FILE;
    objDesc.objectStatus = BSA_ObjectStatus::BSA_ObjectStatus_ANY;
}

void RoachClient::sendData(std::string filePath)
{

}

void RoachClient::setObjectName(const std::string &objNamePrefix)
{
    m_objectNamePrefix = objNamePrefix;
}
int RoachClient::simulateBackup()
{
    Log("new backup business");
    // 1. BSAInit
    long bsaHandle = 0;
    int ret = BSAInit(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsaInit failed, ret =%d ",ret);
        return ret;
    }
    m_currentBsaHandle = bsaHandle;
    Log("bsaHandle: %ld",bsaHandle);


    // 2. 初始化事务
    ret = BSABeginTxn_fun(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsa init trans %ld failed, ret = %d", bsaHandle , ret);
        BSATerminate_fun(bsaHandle);
        return ret;
    }

    // 3. 创建对象
    BSA_DataBlock32 bsaBlock;
    BSA_ObjectDescriptor objDes;
    std::string objectName = m_objectNamePrefix + std::to_string(m_nameIndex) + "/metadata.tar.gz";
    m_newestObjectName = objectName;
    m_nameIndex ++;
    initBsaObjDesc(objDes, objectName);
    ret = BSACreateObject_fun(bsaHandle, &objDes, &bsaBlock);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsa create object failed for bsahandle %ld failed, ret = %d", bsaHandle , ret);
        TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
        return ret;
    }

    // 4. 发送数据
    if (m_operateFile){
        // 拷贝数据
        std::string tmpFileName = std::to_string(m_nameIndex-1) + "_" + m_serialNum + "_metadata.tar.gz";
        if (!CopyFile(m_testFileName, TEST_BACKUP_DIR + "/" + tmpFileName)) {
            Log("Copy file %s to %s failed.", m_testFileName.c_str(), objectName.c_str());
            TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
            return BSA_RC_ACCESS_FAILURE;
        }
        // 读取并发送数据
        FileTest testFile;
        if (!testFile.OpenFile(m_testFileName)) {
            Log("Open file %s failed.", tmpFileName.c_str());
            TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
            return BSA_RC_INVALID_DATABLOCK;            
        }

        char tempData[2*1024*1024 + 1];
        memset(tempData, 0, 2*1024*1024 + 1);

        BSA_DataBlock32 dataBlockPtr;
        dataBlockPtr.numBytes = 0;
        dataBlockPtr.shareId = 0;
        dataBlockPtr.shareOffset = 0;
        dataBlockPtr.bufferLen = 2*1024*1024;
        dataBlockPtr.bufferPtr = tempData;
        while (testFile.Read(&dataBlockPtr)) {
            if (dataBlockPtr.numBytes == 0) {
                break;
            }

            ret = BSASendData_fun(bsaHandle, &dataBlockPtr);
            if (BSA_RC_SUCCESS != ret)
            {
                Log("bsa send data failed for bsahandle %ld failed, ret = %d", bsaHandle , ret);
                TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
                //DeleteFile(TEST_BACKUP_DIR + "/" + tmpFileName);
                return ret;
            }           
        }
        //DeleteFile(TEST_BACKUP_DIR + "/" + tmpFileName);
        
    }
    
    // 5. 结束数据
    ret = BSAEndData_fun(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsa end data failed for bsahandle %ld failed, ret = %d", bsaHandle , ret);
        TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
        return ret;
    }

    // 6. 结束事务
    Log("new backup business end %ld with ret %d", bsaHandle, ret);
    TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_COMMIT);
    return ret;
}

int RoachClient::simulateDelete()
{
    Log("new Delete business");
    int ret = simulateBackup();
    if (BSA_RC_SUCCESS != ret)
    {
        Log("Backup failed, ret = %d", ret);
        return ret;       
    }
    Log("start to query object %s", m_newestObjectName.c_str());
    std::vector<BSA_UInt64> objList;
    ret = QueryObject(objList, m_newestObjectName);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("Query objects failed for ret %d", ret);
        return ret;
    }

    Log("start to delete object %s", m_newestObjectName.c_str());
    // 1. BSAInit
    long bsaHandle = 0;
    ret = BSAInit(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsaInit failed, ret =%d ",ret);
        return ret;
    }
    m_currentBsaHandle = bsaHandle;
    Log("bsaHandle: %ld",bsaHandle);


    // 2. 初始化事务
    ret = BSABeginTxn_fun(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsa init trans %ld failed, ret = %d", bsaHandle , ret);
        BSATerminate_fun(bsaHandle);
        return ret;
    }
    // 4. 删除对象
    for (size_t i = 0; i<objList.size(); i++)
    {
        ret = BSADeleteObj_fun(bsaHandle, objList[i]);
        if (BSA_RC_SUCCESS != ret)
        {
            Log("bsa delete copyId.right %d copyid.left %d in session bsahandle "
                "%ld failed, ret = %d", objList[i].right, objList[i].left, bsaHandle , ret);
            TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
            return ret;
        }
    }

    Log("Delete all objects");
    // 5. 结束事务
    Log("new delete business end %ld with ret %d", bsaHandle, ret);
    TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_COMMIT);
    return ret;
}

int RoachClient::getCurrentBsaHandle()
{
    return m_currentBsaHandle;
}

int RoachClient::QueryObject(std::vector<BSA_UInt64> &objList, const std::string &objName)
{
    Log("start to query object.")
    // 1. BSAInit
    long bsaHandle = 0;
    int ret = BSAInit(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsaInit failed, ret =%d ",ret);
        return ret;
    }
    m_currentBsaHandle = bsaHandle;
    Log("bsaHandle: %ld",bsaHandle);


    // 2. 初始化事务
    ret = BSABeginTxn_fun(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsa init trans %ld failed, ret = %d", bsaHandle , ret);
        BSATerminate_fun(bsaHandle);
        return ret;
    }

    // 3. 查询对象
    BSA_QueryDescriptor queryDes;
    {
        memset(queryDes.objectOwner.app_ObjectOwner, 0, BSA_MAX_APPOBJECT_OWNER);
        memset(queryDes.objectOwner.bsa_ObjectOwner, 0, BSA_MAX_BSAOBJECT_OWNER);
        memset(queryDes.objectName.objectSpaceName, 0, BSA_MAX_OBJECTSPACENAME);
        memset(queryDes.objectName.pathName, 0, BSA_MAX_PATHNAME);
 
        memcpy(queryDes.objectOwner.app_ObjectOwner, "roach", 5);
        memcpy(queryDes.objectOwner.bsa_ObjectOwner, "roach", 5);
        memcpy(queryDes.objectName.objectSpaceName, "roach", 5);  
        Log("Query object like %s[%d]", objName.c_str(), objName.length());
        memcpy(queryDes.objectName.pathName, objName.c_str(), objName.length());
        queryDes.copyType = BSA_CopyType::BSA_CopyType_BACKUP;
        queryDes.objectType = BSA_ObjectType::BSA_ObjectType_FILE;
        queryDes.objectStatus = BSA_ObjectStatus::BSA_ObjectStatus_ANY;
    }
    BSA_ObjectDescriptor queryRet;
    ret = BSAQueryObj_fun(bsaHandle, &queryDes, &queryRet);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("query object failed for bsahandle %ld failed, ret = %d", bsaHandle , ret);
        TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
        return ret;
    }
    objList.push_back(queryRet.copyId);
    while (BSAGetNexQueryObj_fun(bsaHandle, &queryRet) != BSA_RC_NO_MORE_DATA)
    {
        objList.push_back(queryRet.copyId);
    }
    // 4. 结束事务
    Log("new query bus end %ld with ret %d", bsaHandle, ret);
    TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_COMMIT);
    return ret;
}

int RoachClient::simulateRecover()
{
    Log("new recover business");
    int ret = simulateBackup();
    if (BSA_RC_SUCCESS != ret)
    {
        Log("Backup failed, ret = %d", ret);
        return ret;       
    }
    Log("start to query object %s", m_newestObjectName.c_str());
    std::vector<BSA_UInt64> objList;
    ret = QueryObject(objList, m_newestObjectName);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("Query objects failed for ret %d", ret);
        return ret;
    }    
    Log("start to recover object %s", m_newestObjectName.c_str());
    // 1. BSAInit
    long bsaHandle = 0;
    ret = BSAInit(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsaInit failed, ret =%d ",ret);
        return ret;
    }
    m_currentBsaHandle = bsaHandle;
    Log("bsaHandle: %ld",bsaHandle);

    // 2. 初始化事务
    ret = BSABeginTxn_fun(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsa init trans %ld failed, ret = %d", bsaHandle , ret);
        BSATerminate_fun(bsaHandle);
        return ret;
    }

    //3. get object
    BSA_ObjectDescriptor getRet;
    BSA_DataBlock32 dataBlock;
    dataBlock.bufferPtr = nullptr;
    dataBlock.bufferPtr = malloc(1024*1024);
    Log("dataBlock.bufferPtr:%d", dataBlock.bufferPtr);
    for (size_t i = 0; i<objList.size(); i++)
    {
        getRet.copyId = objList[i];
        Log("start to call BSAGetObj_fun");
        ret = BSAGetObj_fun(bsaHandle, &getRet, &dataBlock);
        Log("Get object ret %d", ret);
        if (BSA_RC_SUCCESS != ret)
        {
            Log("Get object (id: left %d, right %d) in session %ld failed, ret = %d", 
                objList[i].left, objList[i].right, bsaHandle , ret);
            TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
            return ret;
        }

        //4. 获取数据
        char tempData[2*1024*1024 + 1];
        memset(tempData, 0, 2*1024*1024 + 1);
        BSA_DataBlock32 dataBlockPtr;
        dataBlockPtr.numBytes = 0;
        dataBlockPtr.shareId = 0;
        dataBlockPtr.shareOffset = 0;
        dataBlockPtr.bufferLen = 2*1024*1024;
        dataBlockPtr.bufferPtr = tempData;
        ret = BSAGetData_fun(bsaHandle, &dataBlockPtr);
        if (BSA_RC_SUCCESS != ret)
        {
            Log("bsa get data failed for bsahandle %ld failed, ret = %d", bsaHandle , ret);
            TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
            return ret;
        }  
        while (ret != BSA_RC_NO_MORE_DATA) {
            ret = BSAGetData_fun(bsaHandle, &dataBlockPtr);
            if ((BSA_RC_SUCCESS != ret) && (ret != BSA_RC_NO_MORE_DATA))
            {
                Log("bsa get data failed for bsahandle %ld failed, ret = %d", bsaHandle , ret);
                TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_ABORT);
                return ret;
            }            
        }
    }
    Log("Recover all objects");
    // 5. 结束事务
    Log("new recover bus end %ld with ret %d", bsaHandle, ret);
    TerminateTxnAndSession(bsaHandle, BSA_Vote::BSA_Vote_COMMIT);
    return ret;
}

void RoachClient::setTestFileName(const std::string &fileName)
{
    m_testFileName = fileName;
    m_operateFile = true;
}
int RoachClient::TerminateTxnAndSession(long bsaHandle, BSA_Vote vote)
{
    // 1. 结束事务
    int ret = BSAEndTxn_fun(bsaHandle, vote);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsa end txn failed for bsahandle %ld failed, ret = %d", bsaHandle , ret);
        return ret;
    }

    // 2. 结束会话
    ret = BSATerminate_fun(bsaHandle);
    if (BSA_RC_SUCCESS != ret)
    {
        Log("bsa end session failed for bsahandle %ld failed, ret = %d", bsaHandle , ret);
        return ret;
    }

    return ret;
}

void RoachClient::setSerialNum(std::string serial)
{
    m_serialNum = serial;
}