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
#ifndef AGENT_PREPARE_BACKUP_FS_MEDIA_H
#define AGENT_PREPARE_BACKUP_FS_MEDIA_H
#include "common/Types.h"

class BackupMedium {
public:
    BackupMedium();
    ~BackupMedium();
    
    // initial mount parameter
    mp_void InitFs(const mp_string& vgName, const mp_string& lvName, const mp_string& mountPath,
        const mp_string& fsType);
    mp_void InitFsType(const mp_string& fsType);
    mp_void InitAsm(const mp_string& dgName, const mp_string& asmInstance, const mp_string& asmUser,
        const mp_string& asmPwd, mp_int32 initMetaFs);
    mp_void InitMetaPath(const mp_string& metaPath);
    mp_void SetCreateMediumWhenNoExist();
    mp_void InitExtrasParams(mp_int32 hostRole);

    // create file system backup medium, disk format: {wwn}-{extendflag}-{metaflag}
    mp_int32 CreateFsMedium(const mp_string& diskList);
    // create asm backup medium, disk format: {wwn}-{extendflag}-{metaflag}
    mp_int32 CreateAsmMedium(const mp_string& diskList);

    // get mount path
    mp_string GetFsMountPath();
    mp_string GetAsmMountPath();
    mp_string GetMetaPath();
private:
    // it's used for preparing backup or restore media, if value equal "1",
    // create backup media when no backup media exists, other excute failed
    mp_string createMediaFlag;
    mp_int32 hostRole;

    // when backup asm oracle database, need create new file system to save
    // backup meta data, and keep the same path with file system oracle database
    mp_int32 initMetaDataFlag;

    // FS parameter
    mp_string vgName;
    mp_string lvName;
    mp_string mountPath;
    mp_string fsType;
    // ASM parameter
    mp_string dgName;
    mp_string asmInstance;
    mp_string asmUser;
    mp_string asmPwd;
    // common parameter
    mp_string metaPath;

    mp_string BuildCreateFSParam(const mp_string& diskList);
    mp_string BuildCreateASMParam(const mp_string& diskList);
};

#endif
