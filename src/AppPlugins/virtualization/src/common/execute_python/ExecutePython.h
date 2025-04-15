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
#ifndef EXECUTEPYTHON_H
#define EXECUTEPYTHON_H

#include <memory>
#include <string>
#include <fstream>
#include <tuple>
#include <Python.h>
#include "log/Log.h"
#include "common/JsonHelper.h"
#include "common/Constants.h"
#include "common/Structs.h"

namespace VirtPlugin {
const std::string DEFAULT_PY_APSARA = "apsara_sdk";

class PyAllowThreads {
public:
	PyAllowThreads()
    {
        PY_INTIALIZE
	}

    ~PyAllowThreads()
    {
        if (Py_IsInitialized()) {
            Py_Finalize();
        }
    }
};

class ExecutePython {
public:
    static ExecutePython* GetInstance();
    ~ExecutePython();
    void StartPyMachine();
    void ClosePyMachine();
    template<typename T1, typename T2>
    int32_t CallFunction(const std::string &functionName, T1 &functionPara, T2 &result);

    int32_t InitObject(const std::string &className, PyObject* &pArgs, PyObject* &pObject);

    int32_t CallObjFunction(const std::string &className, const std::string &functionName,
        PyObject* pFunctionArgs, std::string &result);

    void PrintErr(std::string &erroInfo);

    void SetScriptPath(const std::string &path)
    {
        m_scriptPath = path;
    }

    std::string GetScriptPath()
    {
        return m_scriptPath;
    }

    void SetScriptName(const std::string &fileName)
    {
        m_scriptFileName = fileName;
    }

    std::string GetScriptName()
    {
        return m_scriptFileName;
    }

    void SetClassPara(PyObject* para)
    {
        m_classArgs = para;
    }

    static std::mutex m_pythonMutex;

private:
    explicit ExecutePython();
    ExecutePython(const ExecutePython &) = delete;
    ExecutePython& operator=(const ExecutePython &) = delete;
    ExecutePython(ExecutePython &&) = delete;
    ExecutePython& operator=(ExecutePython &&) = delete;

    int32_t CallPython(const std::string &functionName, const std::string &input, std::string &output);
    int32_t GetMethod(PyObject* &pClass, const std::string &className, const std::string &fileName);
    
    std::string m_scriptPath;
    // 仅包含文件名，不含后缀，即test.py，仅设置test
    std::string m_scriptFileName = "apsara_sdk";
    int32_t m_logLevel;
    PyObject* m_classArgs = nullptr;
};
} // end namespace VirtPlugin
#endif // EXECUTEPYTHON_H
