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
#include "BCD.h"
#include <atlconv.h>
#include "log/Log.h"

namespace PluginUtils {
namespace Win32 {

BCDStore BCDStore::OpenStore(LPCWSTR lpStoreName)
{
    WMIServices services(L"root\\WMI");
    if (!services.Valid()) {
        return BCDStore();
    }
    WMIObject storeClass = services.GetObject(L"BCDStore");
    if (!storeClass.Valid()) {
        return BCDStore();
    }

    CComVariant res;
    CComVariant objStore;
    std::vector<std::pair<LPCWSTR, CComVariant>> inParams = {{L"File", L""}};
    bool ret = storeClass.ExecMethod(L"OpenStore", &res, &objStore, &inParams);
    if (!ret) {
        return BCDStore();
    }
    if ((objStore.vt != VT_UNKNOWN) || !objStore.punkVal) {
        return BCDStore();
    }
    return BCDStore(objStore.punkVal, services);
}

bool BCDStore::ProvideBcdObjects()
{
    if (!m_bcdObjects.empty()) {
        return false;
    }

    CComVariant res;
    CComVariant objects;
    std::vector<std::pair<LPCWSTR, CComVariant>> inParams = {{L"Type", CComVariant(0, VT_I4)}};
    bool ret = ExecMethod(L"EnumerateObjects", &res, &objects, &inParams);
    if (!ret) {
        return ret;
    }

    if (objects.vt != (VT_ARRAY | VT_UNKNOWN)) {
        ERRLOG("EnumerateObjects result is not array of objects");
        return false;
    }

    m_bcdObjects.clear();
    size_t nElements = objects.parray->rgsabound[0].cElements;
    m_bcdObjects.reserve(nElements);
    for (ULONG i = 0; i < nElements; i++) {
        IUnknown *pUnknown = ((IUnknown **)objects.parray->pvData)[i];
        BCDObject obj(pUnknown, m_pServices);
        if (!obj.Valid()) {
            ERRLOG("EnumerateObjects returned invalid object");
            return false;
        }

        m_bcdObjects.push_back(obj);
    }

    return true;
}

BCDObject BCDStore::CopyObject(const BCDObject &obj)
{
    if (!obj.Valid()) {
        ERRLOG("Cannot copy invalid object");
        return BCDObject();
    }
    std::string objID = obj.GetID();
    if (objID.empty()) {
        ERRLOG("Cannot copy object with empty ID");
        return BCDObject();
    }

    CComVariant res;
    CComVariant newobj;
    std::vector<std::pair<LPCWSTR, CComVariant>> inParams = {
        {L"SourceStoreFile", L""}, {L"SourceId", objID.c_str()}, {L"Flags", 1}};
    bool ret = ExecMethod(L"CopyObject", &res, &newobj, &inParams);
    if (!ret) {
        ERRLOG("CopyObject failed");
        return BCDObject();
    }
    if ((newobj.vt != VT_UNKNOWN) || !newobj.punkVal) {
        ERRLOG("CopyObject returned invalid object");
        return BCDObject();
    }
    return BCDObject(newobj.punkVal, m_pServices);
}

BCDElement BCDObject::GetElement(BCDElementType type)
{
    if (!Valid()) {
        ERRLOG("Cannot get element from invalid object");
        return BCDElement();
    }

    CComVariant res;
    CComVariant bcdElement;
    std::vector<std::pair<LPCWSTR, CComVariant>> inParams = {{L"Type", (long)type}};
    bool ret = ExecMethod(L"GetElement", &res, &bcdElement, &inParams);
    if (!ret) {
        ERRLOG("GetElement failed");
        return BCDElement();
    }

    if ((bcdElement.vt != VT_UNKNOWN) || !bcdElement.punkVal) {
        ERRLOG("GetElement returned invalid element");
        return BCDElement();
    }

    return BCDElement(bcdElement.punkVal, m_pServices, *this, type);
}

std::vector<int> BCDObject::EnumerateElementTypes()
{
    if (!Valid()) {
        ERRLOG("Cannot enumerate element types of invalid object");
        return {};
    }

    CComVariant res;
    CComVariant types;
    std::vector<std::pair<LPCWSTR, CComVariant>> inParams = {};
    bool ret = ExecMethod(L"EnumerateElementTypes", &res, &types, &inParams);
    if (!ret) {
        ERRLOG("EnumerateElementTypes failed");
        return {};
    }

    if (!types.parray) {
        ERRLOG("EnumerateElementTypes result is not array of objects");
        return {};
    }

    std::vector<int> typeVec;
    size_t nElements = types.parray->rgsabound[0].cElements;
    typeVec.reserve(nElements);
    for (ULONG i = 0; i < nElements; i++) {
        typeVec.push_back(((int *)(types.parray->pvData))[i]);
    }

    return std::move(typeVec);
}

bool BCDObject::SetElementHelper(
    BCDElementType type, LPCWSTR pFunctionName, LPCWSTR pParamName, const CComVariant &Value)
{
    if (!pFunctionName || !pParamName) {
        ERRLOG("Internal error: null passed to SetElementHelper");
        return false;
    }
    if (!Valid()) {
        ERRLOG("Cannot set element for invalid object");
        return false;
    }

    CComVariant res;
    std::vector<std::pair<LPCWSTR, CComVariant>> inParams = {{L"Type", (long)type}, {pParamName, Value}};
    bool ret = ExecMethod(pFunctionName, &res, &inParams);
    if (!ret) {
        ERRLOG("ExecMethod: %s failed", pFunctionName);
        return ret;
    }

    if ((res.vt != VT_BOOL) && (res.boolVal != TRUE)) {
        ERRLOG("ExecMethod: %s returned failure", pFunctionName);
        return false;
    }

    return true;
}

bool BCDObjectList::ApplyChanges()
{
    if (!Valid()) {
        ERRLOG("Cannot apply changes to invalid object list");
        return false;
    }

    SAFEARRAY *pArray = SafeArrayCreateVector(VT_BSTR, 0, (ULONG)m_ids.size());
    if (!pArray) {
        ERRLOG("SafeArrayCreateVector failed");
        return false;
    }

    for (size_t i = 0; i < m_ids.size(); i++) {
        ((BSTR *)pArray->pvData)[i] = SysAllocString(CA2W(m_ids[i].c_str()));
    }

    CComVariant objList(pArray);
    SafeArrayDestroy(pArray);

    return m_element.SetElementHelper(L"SetObjectListElement", L"Ids", objList);
}

}  // namespace Win32
}  // namespace PluginUtils
