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
#include "ExecutePython.h"
#include "common/Macros.h"

namespace VirtPlugin {
std::mutex ExecutePython::m_pythonMutex;

ExecutePython::ExecutePython()
{
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    m_scriptPath = agentHomedir + "/DataBackup/ProtectClient/Plugins/VirtualizationPlugin/script/";
}

ExecutePython::~ExecutePython()
{
    if (Py_IsInitialized()) {
        Py_Finalize();
    }
}

ExecutePython* ExecutePython::GetInstance()
{
    static ExecutePython executePython;
    return &executePython;
}

void ExecutePython::StartPyMachine()
{
    PY_INTIALIZE
}

void ExecutePython::ClosePyMachine()
{
    int32_t nStatus = PyGILState_Check();
    PyGILState_STATE gstate;
    if (!nStatus) {
        gstate = PyGILState_Ensure();
    }
    Py_Finalize();
}

template<typename T1, typename T2>
int32_t ExecutePython::CallFunction(const std::string &functionName, T1 &functionPara, T2 &result)
{
    return SUCCESS;
}

int32_t ExecutePython::CallPython(const std::string &functionName, const std::string &input, std::string &output)
{
    return SUCCESS;
}

int32_t ExecutePython::InitObject(const std::string &className, PyObject* &pArgs, PyObject* &pObject)
{
    PyObject* pClass = nullptr;
    PyRun_SimpleString("import sys");
    std::string addScriptPath = "sys.path.append('" + m_scriptPath + "')";
    PyRun_SimpleString(addScriptPath.c_str());
    std::string erroInfo;
    PrintErr(erroInfo);
    if (GetMethod(pClass, className, m_scriptFileName) != SUCCESS) {
        ERRLOG("Get python class %s failed.", className.c_str());
        return FAILED;
    }
    PyObject* pConstruct = PyInstanceMethod_New(pClass);
    Py_DECREF(pClass);
    if (pConstruct == nullptr) {
        ERRLOG("Get pConstruct %s null!!!", m_scriptFileName.c_str());
        PrintErr(erroInfo);
        return FAILED;
    }
    pObject = PyObject_CallObject(pConstruct, pArgs);
    if (pObject == nullptr) {
        PrintErr(erroInfo);
        ERRLOG("Get pObject %s null!!!", m_scriptFileName.c_str());
        return FAILED;
    }
    DBGLOG("Init python class %s success!", className.c_str());
    Py_DECREF(pConstruct);
    return SUCCESS;
}

int32_t ExecutePython::GetMethod(PyObject* &pClass, const std::string &className, const std::string &fileName)
{
    PyObject* pModule = nullptr;
    PyObject* pDict = nullptr;
    // 在使用这个函数的时候，只需要写文件的名称就可以了。不用写后缀。
    pModule = PyImport_ImportModule(fileName.c_str());
    std::string erroInfo = "";
    if (pModule == nullptr) {
        ERRLOG("Import %s module failed.", fileName.c_str());
        PrintErr(erroInfo);
        return FAILED;
    }
    pDict = PyModule_GetDict(pModule);
    Py_DECREF(pModule);
    if (pDict == nullptr) {
        ERRLOG("Get Dict %s failed.", fileName.c_str());
        PrintErr(erroInfo);
        return FAILED;
    }
    pClass = PyDict_GetItemString(pDict, className.c_str());
    Py_DECREF(pDict);
    if (pClass == nullptr) {
        ERRLOG("Get class %s from %s failed.", className.c_str(), fileName.c_str());
        PrintErr(erroInfo);
        return FAILED;
    }
    DBGLOG("GetMethod %s success!", fileName.c_str());
    return SUCCESS;
}

int32_t ExecutePython::CallObjFunction(const std::string &className, const std::string &functionName,
    PyObject* pFunctionArgs, std::string &result)
{
    std::lock_guard<std::mutex> guard(m_pythonMutex);
    PyAllowThreads thread;
    PyObject* pObject = nullptr;
    PyObject* pReturn = nullptr;
    if (InitObject(className, m_classArgs, pObject) == FAILED) {
        ERRLOG("Init class %s failed.", className.c_str());
        return FAILED;
    }
    if (pFunctionArgs == nullptr) {
        pReturn = PyObject_CallMethod(pObject, functionName.c_str(), "");
        DBGLOG("PyObject CallMethodNoArgs, instance name: %s", functionName.c_str());
    } else {
        pReturn = PyObject_CallMethod(pObject, functionName.c_str(), "O", pFunctionArgs);
        DBGLOG("PyObject CallMethod success, instance name: %s", functionName.c_str());
    }
    PrintErr(result);
    if (pReturn == nullptr) {
        ERRLOG("Call Method null, instance name: %s", functionName.c_str());
        return FAILED;
    }
    char* nResult = nullptr;
    PyArg_Parse(pReturn, "s", &nResult);
    PrintErr(result);
    result = nResult;
    DBGLOG("Execute python class %s, return:%s", functionName.c_str(), result.c_str());
    Py_DECREF(pReturn);
    return SUCCESS;
}

void ExecutePython::PrintErr(std::string &erroInfo)
{
    PyObject *type;
    PyObject *value;
    PyObject *traceback;
    if (PyErr_Occurred()) {
        PyErr_Fetch(&type, &value, &traceback);
        PyObject* pystr = PyObject_Str(value);
        char* err = nullptr;
        PyArg_Parse(pystr, "s", &err);
        if (err == nullptr) {
            ERRLOG("Null ptr.");
            return;
        }
        ERRLOG("Execute python failed, err info: %s", err);
        erroInfo.assign(err);
        PyErr_Restore(type, value, traceback);
    }
}
}
