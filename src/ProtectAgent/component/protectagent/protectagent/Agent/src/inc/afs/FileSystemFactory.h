/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file FileSystemFactory.h
 * @brief AFS - Analyze file system factory class.
 *
 */

#ifndef FILESYSTEMFACTORY_H_INCLUDED
#define FILESYSTEMFACTORY_H_INCLUDED

#include "afs/AfsObject.h"
#include "afs/FileSystem.h"

/**
 * @brief 文件系统工厂类
 */
class filesystemFactory : public afsObjectFacotry {
public:
    filesystemHandler *createObject(int fstype);
};

#endif // FILESYSTEMFACTORY_H_INCLUDED
