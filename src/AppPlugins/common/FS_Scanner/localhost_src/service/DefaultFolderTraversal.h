/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * @file DefaultFolderTraversal.h
 * @date 6/22/2022
 * @author z30016470
 * @brief
 */

#ifndef FS_SCANNER_DEFAULT_FOLDER_TRAVERSAL_H
#define FS_SCANNER_DEFAULT_FOLDER_TRAVERSAL_H
#include "FolderTraversal.h"

class DefaultFolderTraversal : public FolderTraversal {
public:
    explicit DefaultFolderTraversal(std::shared_ptr<StatisticsMgr> statsMgr);
    ~DefaultFolderTraversal() override;

    SCANNER_STATUS Enqueue(const std::string& directory,
        const std::string& prefix = "", uint8_t filterFlag = 0) override;
    SCANNER_STATUS Start() override;
    SCANNER_STATUS Poll() override;
    SCANNER_STATUS Suspend() override;
    SCANNER_STATUS Resume() override;
    SCANNER_STATUS Abort() override;
    SCANNER_STATUS Destroy() override;
    void ProcessCheckPointContainers() override;
};

#endif