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
#include "WMI.h"
#include "log/Log.h"

namespace PluginUtils {
namespace Win32 {

WMIServices::WMIServices(LPCWSTR lpResourcePath)
{
    CComPtr<IWbemLocator> pLocator;
    HRESULT hR = pLocator.CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER);
    if (!SUCCEEDED(hR)) {
        ERRLOG("Failed to create IWbemLocator object. Error code = 0x%x", hR);
        return;
    }

    hR =
        pLocator->ConnectServer(CComBSTR(lpResourcePath), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &m_pServices);
    if (!SUCCEEDED(hR)) {
        ERRLOG("Could not connect. Error code = 0x%x", hR);
        return;
    }

    hR = CoSetProxyBlanket(m_pServices,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE);
    if (!SUCCEEDED(hR)) {
        ERRLOG("Could not set proxy blanket. Error code = 0x%x", hR);
        return;
    }
}

WMIObject WMIServices::GetObject(LPCWSTR lpObjectName, int flags, IWbemContext *pCtx)
{
    if (!m_pServices) {
        ERRLOG("WMIServices is not initialized");
        return WMIObject();
    }

    CComPtr<IWbemClassObject> pObj;
    HRESULT hR = m_pServices->GetObject(CComBSTR(lpObjectName), flags, pCtx, &pObj, nullptr);
    if (!SUCCEEDED(hR)) {
        ERRLOG("GetObject failed. Error code = 0x%x ", hR);
        return WMIObject();
    }

    return WMIObject(pObj, m_pServices);
}

WMIServices::ObjectCollection WMIServices::FindInstances(LPCWSTR lpClassName, int flags, IWbemContext *pCtx)
{
    if (!m_pServices) {
        ERRLOG("WMIServices is not initialized");
        return ObjectCollection();
    }
    CComPtr<IEnumWbemClassObject> pEnum;
    HRESULT hR = m_pServices->CreateInstanceEnum(CComBSTR(lpClassName), flags, pCtx, &pEnum);
    if (!SUCCEEDED(hR)) {
        ERRLOG("CreateInstanceEnum failed. Error code = 0x%x ", hR);
        return ObjectCollection();
    }
    return ObjectCollection(pEnum, m_pServices);
}

std::string WMIObject::GetPropertyString(LPCWSTR lpPropertyName) const
{
    if (!m_pObj) {
        ERRLOG("WMIObject is not initialized");
        return std::string();
    }

    CComVariant val;
    HRESULT hR = m_pObj->Get(lpPropertyName, 0, &val, nullptr, nullptr);
    if (!SUCCEEDED(hR)) {
        ERRLOG("Get property '%s' failed. Error code = 0x%x", lpPropertyName, hR);
        return std::string();
    }

    if ((val.vt == VT_LPWSTR) || (val.vt == VT_BSTR))
        return std::string((_bstr_t)val.bstrVal);
    val.ChangeType(VT_BSTR);
    return std::string((_bstr_t)val.bstrVal);
}

CComVariant WMIObject::GetPropertyVariant(LPCWSTR lpPropertyName) const
{
    CComVariant val;
    if (!m_pObj) {
        ERRLOG("WMIObject is not initialized");
        return CComVariant();
    }

    HRESULT hR = m_pObj->Get(lpPropertyName, 0, &val, nullptr, nullptr);
    if (!SUCCEEDED(hR)) {
        ERRLOG("Get property '%s' failed. Error code = 0x%x", lpPropertyName, hR);
        return CComVariant();
    }
    return val;
}

int WMIObject::GetPropertyInt(LPCWSTR lpPropertyName) const
{
    if (!m_pObj) {
        ERRLOG("WMIObject is not initialized");
        return -1;
    }
    CComVariant val;
    HRESULT hR = m_pObj->Get(lpPropertyName, 0, &val, nullptr, nullptr);
    if (!SUCCEEDED(hR)) {
        ERRLOG("Get property '%s' failed. Error code = 0x%x", lpPropertyName, hR);
        return -1;
    }

    if (val.vt == VT_INT)
        return val.intVal;
    val.ChangeType(VT_INT);
    return val.intVal;
}

bool WMIObject::ConstructInParamsInstance(
    CComPtr<IWbemClassObject> &pInParamsInstance, std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams)
{
    for (auto &p : *pInParams) {
        HRESULT hR = pInParamsInstance->Put(p.first, 0, &p.second, p.second.vt);
        if (hR == WBEM_E_TYPE_MISMATCH) {
            CIMTYPE type = VT_NULL;
            pInParamsInstance->Get(p.first, 0, nullptr, &type, nullptr);
            hR = pInParamsInstance->Put(p.first, 0, &p.second, type);
            if (hR == WBEM_E_TYPE_MISMATCH) {
                CComVariant typeBugWorkaroundTmp = p.second;
                typeBugWorkaroundTmp.ChangeType(VT_BSTR);
                hR = pInParamsInstance->Put(p.first, 0, &typeBugWorkaroundTmp, type);
            }
        }
        if (!SUCCEEDED(hR)) {
            ERRLOG("Put param '%s' failed. Error code = 0x%x", p.first, hR);
            return false;
        }
    }

    return true;
}

bool WMIObject::GetInParamsInstance(LPCWSTR lpMethodName, CComPtr<IWbemClassObject> &pInParamsInstance,
    std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams)
{
    if (!m_pServices || !m_pObj) {
        ERRLOG("WMIObject is not initialized");
        return false;
    }

    std::string tmpProp = GetPropertyString(L"__CLASS");
    if (tmpProp.empty()) {
        ERRLOG("Get property '__CLASS' failed.");
        return false;
    }

    CComPtr<IWbemClassObject> pClassDefinition;
    HRESULT hR = m_pServices->GetObject(CComBSTR(tmpProp.c_str()), 0, nullptr, &pClassDefinition, nullptr);
    if (!SUCCEEDED(hR)) {
        ERRLOG("GetObject '%s' failed. Error code = 0x%x", tmpProp.c_str(), hR);
        return false;
    }

    CComPtr<IWbemClassObject> pInParamsClass;
    hR = pClassDefinition->GetMethod(lpMethodName, 0, &pInParamsClass, nullptr);
    if (!SUCCEEDED(hR)) {
        ERRLOG("GetMethod '%s' failed. Error code = 0x%x", lpMethodName, hR);
        return false;
    }

    if (pInParamsClass) {
        hR = pInParamsClass->SpawnInstance(0, &pInParamsInstance);
        if (!SUCCEEDED(hR)) {
            ERRLOG("SpawnInstance failed. Error code = 0x%x", hR);
            return false;
        }

        if (!pInParams) {
            return true;
        }

        return ConstructInParamsInstance(pInParamsInstance, pInParams);
    }

    return true;
}

bool WMIObject::ExecMethod(LPCWSTR lpMethodName, CComVariant *pResult, CComVariant *pFirstOutParam,
    std::map<std::string, CComVariant> *pAllOutputParams, std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams)
{
    CComPtr<IWbemClassObject> pInParamsInstance;
    CComPtr<IWbemClassObject> pOutParamsInstance;
    bool res = GetInParamsInstance(lpMethodName, pInParamsInstance, pInParams);
    if (!res) {
        return false;
    }
    std::string tmpProp = GetPropertyString(L"__RELPATH");
    if (tmpProp.empty()) {
        ERRLOG("Get property '__RELPATH' failed.");
        return false;
    }

    HRESULT hR = m_pServices->ExecMethod(
        CComBSTR(tmpProp.c_str()), CComBSTR(lpMethodName), 0, nullptr, pInParamsInstance, &pOutParamsInstance, nullptr);
    if (!SUCCEEDED(hR)) {
        ERRLOG("ExecMethod '%s' failed. Error code = 0x%x", lpMethodName, hR);
        return false;
    }

    if (pResult && pOutParamsInstance) {
        hR = pOutParamsInstance->Get(L"ReturnValue", 0, pResult, nullptr, nullptr);
    }

    if ((!pAllOutputParams || pFirstOutParam) && pOutParamsInstance) {
        hR = pOutParamsInstance->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);
        if (!SUCCEEDED(hR)) {
            ERRLOG("BeginEnumeration failed. Error code = 0x%x", hR);
            return false;
        }

        CComBSTR name;
        CComVariant val;
        for (;;) {
            name.AssignBSTR(nullptr);
            hR = pOutParamsInstance->Next(0, &name, &val, nullptr, nullptr);
            if (!name) {
                break;
            }

            if (name == L"ReturnValue") {
                continue;
            }

            if (pAllOutputParams) {
                (*pAllOutputParams)[std::string((_bstr_t)name)] = val;
            } else if (pFirstOutParam) {
                *pFirstOutParam = val;
                break;
            }
        }
    }

    return true;
}

WMIObject::WMIObject(const VARIANT *pInterface, const CComPtr<IWbemServices> &pServices)
{
    if (!pInterface || (pInterface->vt != VT_UNKNOWN)) {
        return;
    }
    pInterface->punkVal->QueryInterface(&m_pObj);
    m_pServices = pServices;
}

WMIObject::WMIObject(IUnknown *pUnknown, const WMIServices &pServices)
{
    if (!pUnknown) {
        return;
    }
    pUnknown->QueryInterface(&m_pObj);
    m_pServices = pServices.m_pServices;
}

WMIObject::WMIObject(IUnknown *pUnknown, const CComPtr<IWbemServices> &pServices)
{
    if (!pUnknown) {
        return;
    }
    pUnknown->QueryInterface(&m_pObj);
    m_pServices = pServices;
}

}  // namespace Win32
}  // namespace PluginUtils
