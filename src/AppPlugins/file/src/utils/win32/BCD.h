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
#ifndef WIN32_BCD_H
#define WIN32_BCD_H

#include <WinSock2.h>
#include <windows.h>
#include <vector>
#include <comutil.h>
#include "WMI.h"

namespace PluginUtils {
namespace Win32 {

const int TYPE_OFFSET = 24;

/* 取自 https://learn.microsoft.com/en-us/previous-versions/windows/desktop/bcd/bcd-enumerations */
enum class BCDElementType {
    // Boot manager elements
    BCDBOOTMGR_OBJECTLIST_DISPLAYORDER = 0x24000001,
    BCDBOOTMGR_OBJECTLIST_BOOTSEQUENCE = 0x24000002,
    BCDBOOTMGR_OBJECTLIST_DEFAULTOBJECT = 0x23000003,

    // Device object elements
    BCDDEVICE_INTEGER_SDIDEVICE = 0x31000003,

    // Library object elements
    BCDLIBRARY_DEVICE_APPLICATIONDEVICE = 0x11000001,
    BCDLIBRARY_STRING_APPLICATIONPATH = 0x12000002,
    BCDLIBRARY_STRING_DESCRIPTION = 0x12000004,

    // OS Loader element objects
    BCDOSLOADER_DEVICE_OSDEVICE = 0x21000001,
    BCDOSLOADER_STRING_SYSTEMROOT_OSDEVICE = 0x22000002,
    BCDOSLOADER_OBJECT_ASSOCIATEDRESUMEOBJECT = 0x23000003,
};

enum class BCDElementDataType {
    DEVICE = 1,
    STRING = 2,
    OBJECT = 3,
    OBJECTLIST = 4,
    INTEGER = 5,
    BOOLEAN = 6,
    INTEGERARRAY = 7,
};

// 对应Windows BCD WMI Provider的BcdDeviceData类
class BCDDeviceData : public WMIObject {
public:
    BCDDeviceData()
    {}

    BCDDeviceData(const VARIANT *pInterface, const CComPtr<IWbemServices> &pServices) : WMIObject(pInterface, pServices)
    {}

public:
    enum class DeviceType {
        BOOT_DEVICE = 1,
        PARTITION_DEVICE = 2,
        FILE_DEVICE = 3,
        RAMDISK_DEVICE = 4,
        UNKNOWN_DEVICE = 5,
    };

    DeviceType GetDeviceType()
    {
        return (DeviceType)GetPropertyInt(L"DeviceType");
    }

    std::string GetAdditionalOptions()
    {
        return GetPropertyString(L"AdditionalOptions");
    }

    std::string GetPartitionPath()
    {
        return GetPropertyString(L"Path");
    }
};

class BCDElement;

// 对应Windows BCD WMI Provider的BcdObject类
class BCDObject : public WMIObject {
public:
    BCDObject()
    {}

    BCDObject(IUnknown *pUnknown, const CComPtr<IWbemServices> &pServices) : WMIObject(pUnknown, pServices)
    {}

public:
    // 对应BcdObject::Id
    std::string GetID() const
    {
        return GetPropertyString(L"Id");
    }

public:
    /* 取自 https://learn.microsoft.com/en-us/previous-versions/windows/desktop/bcd/bcdobject */
    enum class BCDObjectType {
        GLOBAL_SETTINGS = 0x10100002,     // Windows Boot Manager
        WINDOWS_LOADER = 0x10200003,      // Windows Boot Loader
        HIBERNATE_RESUMER = 0x10200004,   // Resume from Hibernate
        BOOT_APPLICATION = 0x10200005,    // Custom boot application, such as Memory Tester
        LEGACY_OSLOADER = 0x10300006,     // Windows Legacy OS Loader
        MODULE_SETTINGS = 0x20100000,     // EMS Settings, Debugger Settings, RAM defects
        BOOTLDR_SETTINGS = 0x20200003,    // Boot Loader Settings
        RESUMELDR_SETTINGS = 0x20200004,  // Resume Loader Settings
    };

    // 对应BcdObject::type
    BCDObjectType GetType() const
    {
        return (BCDObjectType)GetPropertyInt(L"Type");
    }

    // 返回BcdElement对象
    BCDElement GetElement(BCDElementType type);

    // 列举BcdObject下的所有BcdElement对象类型
    std::vector<int> EnumerateElementTypes();

public:
    // 设置字符串类型的BcdElement属性值
    bool SetElement(BCDElementType type, LPCWSTR value)
    {
        return SetElementHelper(type, L"SetStringElement", L"String", CComVariant(value));
    }

    // 设置整数类型的BcdElement属性值
    bool SetElement(BCDElementType type, ULONGLONG value)
    {
        return SetElementHelper(type, L"SetIntegerElement", L"Integer", value);
    }

    // 设置bool类型的BcdElement属性值
    bool SetElement(BCDElementType type, bool value)
    {
        return SetElementHelper(type, L"SetBooleanElement", L"Boolean", value);
    }

    // 设置BcdObject类型的BcdElement属性值
    bool SetElement(BCDElementType type, const BCDObject &value)
    {
        std::string objId = value.GetID();
        if (objId.empty()) {
            return false;
        }
        return SetElementHelper(type, L"SetObjectElement", L"Id", objId.c_str());
    }

    bool operator==(const BCDObject &rhs)
    {
        return GetID() == rhs.GetID();
    }

    bool operator!=(const BCDObject &rhs)
    {
        return GetID() != rhs.GetID();
    }

private:
    bool SetElementHelper(BCDElementType type, LPCWSTR pFunctionName, LPCWSTR pParamName, const CComVariant &value);
    friend class BCDElement;
};

// 对应Windows BCD WMI Provider的BcdElement类
class BCDElement : public WMIObject {
public:
    BCDElement()
    {}

    BCDElement(
        IUnknown *pUnknown, const CComPtr<IWbemServices> &pServices, const BCDObject &owner, BCDElementType elementType)
        : WMIObject(pUnknown, pServices), m_elementType(elementType), m_owner(owner)
    {}

public:
    CComVariant ToVariant()
    {
        int elType = GetPropertyInt(L"Type");
        switch ((elType & 0x0F000000) >> TYPE_OFFSET) {
            case static_cast<int>(BCDElementDataType::DEVICE):  // Device
                return GetPropertyVariant(L"Device");
            case static_cast<int>(BCDElementDataType::STRING):  // String
                return GetPropertyVariant(L"String");
            case static_cast<int>(BCDElementDataType::OBJECT):  // Object
                return GetPropertyVariant(L"Id");
            case static_cast<int>(BCDElementDataType::OBJECTLIST):  // Object List
                return GetPropertyVariant(L"Ids");
            case static_cast<int>(BCDElementDataType::INTEGER):  // 64-bit Integer
                return GetPropertyVariant(L"Integer");
            case static_cast<int>(BCDElementDataType::BOOLEAN):  // Boolean
                return GetPropertyVariant(L"Boolean");
            case static_cast<int>(BCDElementDataType::INTEGERARRAY):  // Array of 64-bit integers
                return GetPropertyVariant(L"Integers");
            default:
                return CComVariant();
        }
        return CComVariant();
    }

    std::string ToString()
    {
        CComVariant var = ToVariant();
        if (var.vt == VT_EMPTY) {
            return std::string();
        }
        if (var.vt != VT_BSTR) {
            var.ChangeType(VT_BSTR);
        }
        if (!var.bstrVal) {
            return std::string();
        }
        return std::string((_bstr_t)var.bstrVal);
    }

    std::string GetString()
    {
        return GetPropertyString(L"String");
    }

    ULONGLONG ToInteger()
    {
        CComVariant var = ToVariant();
        if (var.vt == VT_EMPTY) {
            return -1LL;
        }
        if (var.vt != VT_UI8) {
            var.ChangeType(VT_UI8);
        }
        return var.ullVal;
    }

    bool ToBoolean()
    {
        CComVariant var = ToVariant();
        if (var.vt == VT_EMPTY) {
            return false;
        }
        if (var.vt != VT_BOOL) {
            var.ChangeType(VT_BOOL);
        }
        return var.boolVal != FALSE;
    }

    //! Converts a BCD property to a BCD object list
    inline class BCDObjectList GetObjectList();

    //! Converts a BCD property to a BCDDeviceData object
    BCDDeviceData ToDeviceData()
    {
        CComVariant data = GetPropertyVariant(L"Device");
        return BCDDeviceData(&data, m_pServices);
    }

private:
    bool SetElementHelper(LPCWSTR pFunctionName, LPCWSTR pParamName, const CComVariant &value)
    {
        return m_owner.SetElementHelper(m_elementType, pFunctionName, pParamName, value);
    }
    friend class BCDObjectList;

private:
    BCDElementType m_elementType;
    BCDObject m_owner;
};

// 对应Windows BCD WMI Provider的BcdObjectListElement类
class BCDObjectList {
public:
    BCDObjectList()
    {}

    explicit BCDObjectList(const BCDElement &owningElement)
    {
        if (!owningElement.Valid()) {
            return;
        }

        int elType = owningElement.GetPropertyInt(L"Type");
        if (((elType & 0x0F000000) >> TYPE_OFFSET) != static_cast<int>(BCDElementDataType::OBJECTLIST)) {
            return;
        }

        CComVariant rawList = owningElement.GetPropertyVariant(L"Ids");
        if ((rawList.vt != (VT_ARRAY | VT_BSTR)) || !rawList.parray) {
            return;
        }

        size_t nElements = rawList.parray->rgsabound[0].cElements;

        m_ids.reserve(nElements);
        for (ULONG i = 0; i < nElements; i++) {
            m_ids.push_back(std::string((_bstr_t)(((BSTR *)rawList.parray->pvData)[i])));
        }

        // BCDElement is just a wrapper around CComPtr, so copying it is cheap
        m_element = owningElement;
    }

    bool Valid()
    {
        return m_element.Valid();
    }

    // Converts a list of object GUIDs to a string form
    std::string ToString(std::string separator = "\r\n")
    {
        if (m_ids.empty()) {
            return std::string();
        }

        size_t sepLen = separator.length();

        std::string ret;
        ret.reserve((m_ids[0].length() + sepLen) * m_ids.size());

        for (size_t i = 0; i < m_ids.size(); i++) {
            if (i) {
                ret += separator;
            }
            ret += m_ids[0];
        }
        return ret;
    }

public:
    unsigned GetElementCount()
    {
        return (unsigned)m_ids.size();
    }

    // Returns a single BCD object GUID from the list
    std::string operator[](unsigned idx)
    {
        if (idx >= m_ids.size()) {
            return std::string();
        }
        return m_ids[idx];
    }

    // Inserts a new BCD object in the list by its ID
    bool InsertObject(const std::string &objID, unsigned insertBefore = -1)
    {
        if (objID.empty()) {
            return false;
        }
        if (insertBefore > m_ids.size()) {
            insertBefore = (unsigned)m_ids.size();
        }
        m_ids.insert(m_ids.begin() + insertBefore, objID);
        return ApplyChanges();
    }

    // Inserts a new BCD object in the list
    bool InsertObject(const BCDObject &object, unsigned insertBefore = -1)
    {
        return InsertObject(object.GetID(), insertBefore);
    }

private:
    bool ApplyChanges();

private:
    std::vector<std::string> m_ids;
    BCDElement m_element;
};

// 对应Windows BCD WMI Provider的BcdStore类
class BCDStore : public WMIObject {
public:
    static BCDStore OpenStore(LPCWSTR lpStoreName = L"");

    unsigned GetObjectCount()
    {
        if (!ProvideBcdObjects()) {
            return 0;
        }
        return (unsigned)m_bcdObjects.size();
    }

    // 返回BcdStore下的所有BcdObject对象
    const std::vector<BCDObject> &GetObjects()
    {
        ProvideBcdObjects();
        return m_bcdObjects;
    }

    BCDObject CopyObject(const BCDObject &obj);

    // 返回指定类型的第一个BCDObject对象
    BCDObject GetFirstObjectOfType(BCDObject::BCDObjectType type)
    {
        ProvideBcdObjects();
        for (const BCDObject &obj : m_bcdObjects) {
            if (obj.GetType() == type) {
                return obj;
            }
        }
        return BCDObject();
    }

    // 返回指定ID的BCDObject对象
    BCDObject GetObject(const std::string &objectID)
    {
        ProvideBcdObjects();
        for (const BCDObject &obj : m_bcdObjects) {
            if (obj.GetID() == objectID) {
                return obj;
            }
        }
        return BCDObject();
    }

    BCDStore()
    {}

    BCDStore(IUnknown *pUnknown, const WMIServices &pServices) : WMIObject(pUnknown, pServices)
    {}

protected:
    bool ProvideBcdObjects();

protected:
    std::vector<BCDObject> m_bcdObjects;
};

}  // namespace Win32
}  // namespace PluginUtils

#endif