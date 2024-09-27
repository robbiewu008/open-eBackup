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
#ifndef __PLUGINCFG_MANAGER_H__
#define __PLUGINCFG_MANAGER_H__

#define private public
#define protected public

#include <stdlib.h>
#include <vector>
#include "pluginfx/PluginManager.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "securec.h"
#include "pluginfx/iplugin.h"
#include "gtest/gtest.h"
#include "stub.h"
#include <vector>
using namespace std;

static Stub *m_stub = NULL;
mp_void StubCMpPluginManagerTestLogVoid(mp_void* pthis);

class CMpPluginManagerTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub;
        m_stub->set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubCMpPluginManagerTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        if(m_stub != NULL) {
            delete m_stub;
        }
    }
    Stub stub;
};


//******************************************************************************

class CPluginManagerCBTest: public IPluginCallback
{
public:
    CPluginManagerCBTest();
    ~CPluginManagerCBTest();
 
    //IPluginCallback 虚方法实现
    virtual mp_bool CanUnload(IPlugin& pOldPlg);
    virtual mp_void OnUpgraded(IPlugin& pOldPlg, IPlugin& pNewPlg);
    virtual mp_void SetOptions(IPlugin& plg);
    virtual mp_string GetReleaseVersion(const mp_string& pszLib, mp_string& pszVer);
};

mp_void StubDlibClose(mp_handle_t hDlib){
    return;
}

mp_void* StubDlibDlsym0(mp_handle_t hDlib, const mp_string& pszFname){
    return NULL;
}

mp_void* StubDlibDlsym(mp_handle_t hDlib, const mp_string& pszFname){
    
    return (mp_void*)"test";
}

mp_handle_t StubDlibOpenEx0(const mp_string& pszLibName, mp_bool bLocal){
    return NULL;
}

mp_handle_t StubDlibOpenEx(const mp_string& pszLibName, mp_bool bLocal){
    return (mp_handle_t)"test";
}

mp_bool StubSearch0(const mp_string& pszFile, vector<mp_string>& flist){
    return MP_TRUE;
}

mp_bool StubSearch(const mp_string& pszFile, vector<mp_string>& flist){
    return MP_FALSE;
}

CModule* StubLoad(const mp_string& pszName){
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_string pszName1;
    mp_string pszVersion;
    pModule = new CModule(m_FileSearcher,pszName1,pszVersion);
    
    return pModule;
}

CModule* StubLoad0(mp_string pszName){    
    return NULL;
}

CModule* StubGetModule(mp_void* pthis, const mp_string& pszName){
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_string pszName1;
    mp_string pszVersion;
    pModule = new CModule(m_FileSearcher,pszName1,pszVersion);
    
    return pModule;
}

CModule* StubGetModulet(mp_void* pthis, mp_string pszName){
    pszName = "test";
    CModule* pModule;
    CFileSearcher m_FileSearcher;
    mp_string pszName1;
    mp_string pszVersion;
    pModule = new CModule(m_FileSearcher,pszName1,pszVersion);
    
    return pModule;
}

CModule* StubGetModule0(mp_void* pthis, const mp_string& pszName){    
    return NULL;
}

mp_void StubCMpPluginManagerTestLogVoid(mp_void* pthis){
    return;
}

#endif


