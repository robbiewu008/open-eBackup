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
#ifndef WIN32_WMI_H
#define WIN32_WMI_H

#include <WinSock2.h>
#include <windows.h>
#include <map>
#include <optional>
#include <atlcomcli.h>
#include <Wbemidl.h>
#include <comutil.h>
#include <vector>

namespace PluginUtils {
namespace Win32 {

/*
 *  WMIObject实例对应于一个单一的WMI类的对象，允许调用其方法
 */
class WMIObject {
public:
    // 创建一个给定IWbemClassObject实例的WMIObject
    WMIObject(const CComPtr<IWbemClassObject> &pObj, const CComPtr<IWbemServices> &pServices)
        : m_pObj(pObj), m_pServices(pServices)
    {}

    WMIObject() = default;
    WMIObject(const VARIANT *pInterface, const CComPtr<IWbemServices> &pServices);
    WMIObject(IUnknown *pUnknown, const class WMIServices &pServices);
    WMIObject(IUnknown *pUnknown, const CComPtr<IWbemServices> &pServices);

    // 允许直接调用IWbemClassObject的方法
    IWbemClassObject *operator->()
    {
        return m_pObj;
    }

    // 检查WMIObject对象合法性
    bool Valid() const
    {
        return m_pObj != nullptr;
    }

    // 获取WMI对象属性的值，CComVariant类型返回值
    CComVariant GetPropertyVariant(LPCWSTR lpPropertyName) const;

    // 获取WMI对象属性的值，字符串类型返回值
    std::string GetPropertyString(LPCWSTR lpPropertyName) const;

    // 获取WMI对象属性的值，整型返回值
    int GetPropertyInt(LPCWSTR lpPropertyName) const;

public:
    /*
     * 提供通过索引运算符[]来访问WMIObject对象的属性
     */
    class PropertyAccessor {
    public:
        IWbemClassObject *m_pObj;
        LPCWSTR m_lpPropertyName;

    public:
        operator CComVariant()
        {
            CComVariant variant;
            if (m_pObj) {
                m_pObj->Get(m_lpPropertyName, 0, &variant, nullptr, nullptr);
            }
            return variant;
        }

        operator std::string()
        {
            CComVariant variant;
            if (m_pObj) {
                m_pObj->Get(m_lpPropertyName, 0, &variant, nullptr, nullptr);
            }
            if (variant.vt == VT_NULL)
                return std::string();
            if (variant.vt != VT_BSTR)
                variant.ChangeType(VT_BSTR);
            return std::string((_bstr_t)variant.bstrVal);
        }

        operator int()
        {
            CComVariant variant;
            if (m_pObj) {
                m_pObj->Get(m_lpPropertyName, 0, &variant, nullptr, nullptr);
            }
            if (variant.vt == VT_NULL)
                return 0;
            if (variant.vt != VT_INT)
                variant.ChangeType(VT_INT);
            return variant.intVal;
        }

        void operator=(const CComVariant &var)
        {
            if (m_pObj) {
                m_pObj->Put(m_lpPropertyName, 0, &const_cast<CComVariant &>(var), var.vt);
            }
        }

    private:
        PropertyAccessor(IWbemClassObject *pObj, LPCWSTR lpPropertyName)
            : m_pObj(pObj), m_lpPropertyName(lpPropertyName)
        {}

        friend class WMIObject;
    };

public:
    // 允许使用'obj[L"PropertyName"]'的语法
    PropertyAccessor operator[](const LPCWSTR lpStr)
    {
        return PropertyAccessor(m_pObj, lpStr);
    }

public:
    // 执行WMI对象的方法，没有输出参数
    bool ExecMethod(LPCWSTR lpMethodName, CComVariant *pResult = nullptr,
        std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams = nullptr)
    {
        return ExecMethod(lpMethodName, pResult, nullptr, nullptr, pInParams);
    }

    // 执行WMI对象的方法，对应的方法有一个或多个输出参数
    bool ExecMethod(LPCWSTR lpMethodName, CComVariant *pResult,
        std::map<std::string, CComVariant> *pOutParams = nullptr,
        std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams = nullptr)
    {
        return ExecMethod(lpMethodName, pResult, nullptr, pOutParams, pInParams);
    }

    // 执行WMI对象的方法，对应的方法只有一个输出参数
    bool ExecMethod(LPCWSTR lpMethodName, CComVariant *pResult, CComVariant *pFirstOutParam,
        std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams = nullptr)
    {
        return ExecMethod(lpMethodName, pResult, pFirstOutParam, nullptr, pInParams);
    }

    // 执行WMI对象的方法，对应的方法只有一个输出参数，且为WMIObject类型
    bool ExecMethod(LPCWSTR lpMethodName, CComVariant *pResult, WMIObject *pFirstOutParam,
        std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams = nullptr)
    {
        CComVariant var;
        bool res = ExecMethod(lpMethodName, pResult, &var, nullptr, pInParams);
        if (!res || !pFirstOutParam || (var.vt != VT_UNKNOWN) || !var.punkVal) {
            return res;
        }

        pFirstOutParam->~WMIObject();
        pFirstOutParam->WMIObject::WMIObject(&var, m_pServices);

        return true;
    }

protected:
    bool ConstructInParamsInstance(
        CComPtr<IWbemClassObject> &pInParamsInstance, std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams);
    bool GetInParamsInstance(LPCWSTR lpMethodName, CComPtr<IWbemClassObject> &pInParamsInstance,
        std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams);

    // 执行WMI对象的方法
    bool ExecMethod(LPCWSTR lpMethodName, CComVariant *pResult, CComVariant *pFirstOutParam,
        std::map<std::string, CComVariant> *pAllOutputParams = nullptr,
        std::vector<std::pair<LPCWSTR, CComVariant>> *pInParams = nullptr);

protected:
    CComPtr<IWbemClassObject> m_pObj;
    CComPtr<IWbemServices> m_pServices;
};

/*
    WMIServices实例代表一个根服务对象，用于创建或查找WMI类的实例
*/
class WMIServices {
public:
    explicit WMIServices(LPCWSTR lpResourcePath = L"ROOT\\CIMV2");
    ~WMIServices() = default;

    // 检查WMIServices是否有效
    bool Valid()
    {
        return m_pServices != nullptr;
    }

    // 允许直接调用底层IWbemServices的方法
    IWbemServices *operator->()
    {
        return m_pServices;
    }

public:
    // 允许迭代遍历WMI对象集合
    class ObjectCollection {
    public:
        ObjectCollection(const CComPtr<IEnumWbemClassObject> &pEnum, const CComPtr<IWbemServices> &pServices)
            : m_pStartEnum(pEnum), m_pServices(pServices)
        {}

        ObjectCollection()
        {}

        bool Valid()
        {
            return m_pStartEnum != NULL;
        }

    public:
        class iterator {
        public:
            iterator(const CComPtr<IEnumWbemClassObject> &pStartEnum, const CComPtr<IWbemServices> &pServices)
                : m_pServices(pServices)
            {
                if (!pStartEnum) {
                    return;
                }
                pStartEnum->Clone(&m_pEnum);
                if (m_pEnum) {
                    ULONG done = 0;
                    if (m_pEnum->Next(0, 1, &m_pObject, &done) != S_OK)
                        m_pEnum = nullptr;
                }
            }

            iterator(const iterator &iter)
            {
                if (iter.m_pEnum) {
                    iter.m_pEnum->Clone(&m_pEnum);
                }
                m_pObject = iter.m_pObject;
            }

            iterator &operator=(const iterator &other)
            {
                if (this != &other) {
                    if (other.m_pEnum) {
                        other.m_pEnum->Clone(&m_pEnum);
                    }
                    m_pObject = other.m_pObject;
                }

                return *this;
            }

            iterator()
            {}

            void operator++()
            {
                if (m_pEnum) {
                    m_pObject = nullptr;
                    ULONG done = 0;
                    if (m_pEnum->Next(0, 1, &m_pObject, &done) != S_OK)
                        m_pEnum = nullptr;
                }
            }

            WMIObject operator*()
            {
                return WMIObject(m_pObject, m_pServices);
            }

            bool operator!=(const iterator &right)
            {
                if (!m_pEnum || !right.m_pEnum) {
                    return m_pEnum != right.m_pEnum;
                }
                return m_pObject != right.m_pObject;
            }

        private:
            CComPtr<IEnumWbemClassObject> m_pEnum;
            CComPtr<IWbemClassObject> m_pObject;
            CComPtr<IWbemServices> m_pServices;
        };

        iterator begin()  // 支持基于范围的for循环
        {
            if (!Valid()) {
                return iterator();
            }
            return iterator(m_pStartEnum, m_pServices);
        }

        iterator end()
        {
            return iterator();
        }

    private:
        CComPtr<IEnumWbemClassObject> m_pStartEnum;
        CComPtr<IWbemServices> m_pServices;
    };

public:
    /* 返回特定CIM类对象的所有实例.
       示例:
        WMIServices srv(L"root\\CIMV2");
        for (WMIObject obj : srv.FindInstances(L"Win32_LogicalDisk"))
        {
            std::string str = obj[L"DeviceID"];
            auto diskId = obj.GetPropertyString(L"DeviceID");
            if (diskId = L"X:") dosomething();
        }
    */
    ObjectCollection FindInstances(LPCWSTR lpClassName, int flags = 0, IWbemContext *pCtx = nullptr);

    // 返回一个WMI对象
    WMIObject GetObject(LPCWSTR lpObjectName, int flags = 0, IWbemContext *pCtx = nullptr);

private:
    CComPtr<IWbemServices> m_pServices;
    bool m_comInitialized = false;
    friend class WMIObject;
};

}  // namespace Win32
}  // namespace PluginUtils

#endif