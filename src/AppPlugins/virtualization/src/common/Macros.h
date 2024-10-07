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
#ifndef VIRT_PLUGIN_MACRO_H
#define VIRT_PLUGIN_MACRO_H

#ifndef WIN32
#include <Python.h>
#endif

#define VIRT_PLUGIN_EXPORT  __declspec(dllexport)

#define VIRT_PLUGIN_NAMESPACE_BEGIN namespace VirtPlugin {
#define VIRT_PLUGIN_NAMESPACE_END }
#define USING_NAMESPACE_VIRT_PLUGIN using namespace VirtPlugin

#ifndef WIN32
#define  PY_INTIALIZE if (!Py_IsInitialized()) { \
        Py_Initialize(); \
        }
#define PY_DECONSTRUCT if (m_objectMap.empty() && Py_IsInitialized()) { \
        Py_Finalize(); \
    }
#endif

#endif  // _VIRT_PLUGIN_MACRO_H_
