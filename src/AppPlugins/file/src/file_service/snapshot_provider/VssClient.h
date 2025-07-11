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
#ifndef WIN32_VSS_CLIENT_UTIL_H
#define WIN32_VSS_CLIENT_UTIL_H

#include <string>
#include <iostream>
#include <iterator>
#include <optional>
#include <vector>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <objbase.h>
#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <comdef.h>

/**
 * This util is a simple wrapper for Windows Volume Shadow Service,
 * providing snapshot creation/query/delete,
 * involving no writers
 */
namespace Win32VSSWrapper {

/**
 * Detail information of a VSS Snapshot
 */
class VssSnapshotProperty {
public:
    VssSnapshotProperty(const VSS_SNAPSHOT_PROP &prop);

    uint64_t CreateTime() const; /* snapshot creation time in Windows timestamp (from 1960) */
    VSS_SNAPSHOT_STATE Status() const;
    uint64_t SnapshotsCount() const;

    uint64_t SnapshotAttributes() const;
    bool IsClientAccessible() const;
    bool IsExposedLocally() const;
    bool IsExposedRemotely() const;
    bool IsTransportable() const;
    bool IsNoAutoRelease() const;
    bool IsPersistent() const;
    bool IsHardwareAssisted() const;
    bool IsNoWriters() const;
    bool IsImported() const;
    bool IsPlex() const;
    bool IsDifferential() const;

    /* API for UTF-16 */
    std::wstring SnapshotIDW() const;
    std::wstring SnapshotSetIDW() const;
    std::wstring SnapshotDeviceObjectW() const;
    std::wstring OriginVolumeNameW() const;
    std::wstring OriginatingMachineW() const;
    std::wstring ServiceMachineW() const;
    std::wstring ExposedNameW() const;
    std::wstring ExposedPathW() const;
    std::wstring ProviderIDW() const;

    /* API for UTF-8 */
    std::string SnapshotID() const;
    std::string SnapshotSetID() const;
    std::string ProviderID() const;
    std::string SnapshotDeviceObject() const;
    std::string OriginVolumeName() const;
    std::string OriginatingMachine() const;
    std::string ServiceMachine() const;
    std::string ExposedName() const;
    std::string ExposedPath() const;
private:
    VSS_ID m_snapshotID;
    VSS_ID m_shapshotSetID;
    uint64_t m_snapshotsCount;
    std::wstring m_wSnapshotDeviceObject;
    std::wstring m_wOriginVolumeName;
    std::wstring m_wOriginatingMachine;
    std::wstring m_wServiceMachine;
    std::wstring m_wExposedName;
    std::wstring m_wExposedPath;
    VSS_ID m_providerID;
    uint64_t m_snapshotAttributes;
    uint64_t m_createTime;
    VSS_SNAPSHOT_STATE m_status;
};

/**
 * Snapshot creation result,
 * each creation operation will return a snapshot set ID, and few snapshots ID,
 * VSS using VSS_ID (GUID) as the unique ID of a snapshot/snapshot set,
 * while API here using std::wstring to represent it
 */
class SnapshotSetResult {
public:
    std::vector<std::wstring> m_wSnapshotIDList;
    std::wstring m_wSnapshotSetID;
public:
    std::vector<std::string> SnapshotIDList() const;
    std::string SnapshotSetID() const;
    std::vector<std::wstring> SnapshotIDListW() const;
    std::wstring SnapshotSetIDW() const;
};

/**
 * The class providing snapshot creation/delete/query
 * Not thread-safe
 */
class VssClient {
public:
    VssClient();
    ~VssClient();
    std::optional<SnapshotSetResult> CreateSnapshots(const std::vector<std::string>& volumePathList,
        const std::string& snapshotPercent);
    std::optional<SnapshotSetResult> CreateSnapshotsW(const std::vector<std::wstring>& wVolumePathList,
        const std::string& snapshotPercent);
    bool isVolumeFull;
    bool DeleteSnapshotW(const std::wstring& wSnapshotID);
    bool DeleteSnapshot(const std::string& snapshotID);
    bool DeleteSnapshotSetW(const std::wstring& wSnapshotSetID);
    bool DeleteSnapshotSet(const std::string& snapshotSetID);
    bool DeleteAllSnapshots();
    std::optional<VssSnapshotProperty> GetSnapshotPropertyW(const std::wstring& wSnapshotID);
    std::optional<VssSnapshotProperty> GetSnapshotProperty(const std::string& snapshotID);
    std::vector<VssSnapshotProperty> QuerySnapshotSetW(const std::wstring& wSnapshotSetID);
    std::vector<VssSnapshotProperty> QuerySnapshotSet(const std::string& snapshotSetID);
    std::vector<VssSnapshotProperty> QueryAllSnapshots();
    bool ExposeSnapshotLocallyW(const std::wstring& snapshotID, const std::wstring& wPath);
    bool ExposeSnapshotLocally(const std::string& snapshotID, const std::string& path);
private:
    bool InitializeCom(); /* register this thread to Win32 COM */
    void UninitializeCom();
    bool InitializeBackupContect(const VSS_SNAPSHOT_CONTEXT& context);
    bool WaitAndCheckForAsyncOperation(IVssAsync* pAsync);
    bool PrepareForBackupSync();
    bool DoSnapshotSetSync();
    bool InitializeVssComponent();
private:
    bool m_comInitialized = false;
    IVssBackupComponents* m_pVssObject = nullptr;
};

/**
 * An Util Class, Used to automatically release a CoTaskMemAlloc allocated pointer
 * when the instance of this class goes out of scope（RAII)
 * (even if an exception is thrown)
 **/
class CAutoComPointer {
public:
    CAutoComPointer(LPVOID ptr): m_ptr(ptr) {};
    ~CAutoComPointer() { ::CoTaskMemFree(m_ptr); }
private:
    LPVOID m_ptr;
};

}

#endif
#endif