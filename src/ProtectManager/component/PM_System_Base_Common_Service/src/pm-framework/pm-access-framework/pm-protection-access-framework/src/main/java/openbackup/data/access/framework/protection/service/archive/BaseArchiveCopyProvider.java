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
package openbackup.data.access.framework.protection.service.archive;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import org.springframework.beans.factory.annotation.Autowired;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.stream.Collectors;

/**
 * BaseArchiveCopyProvider
 *
 */
@Slf4j
public abstract class BaseArchiveCopyProvider implements ArchiveCopyProvider {
    /**
     * copyRestApi
     */
    @Autowired
    protected CopyRestApi copyRestApi;

    @Override
    public List<CopyDependencyQueryResponse> queryCopyDependenceChain(List<Copy> copies) {
        if (VerifyUtil.isEmpty(copies)) {
            return Collections.emptyList();
        }
        return copies.stream().map(copy -> {
            CopyDependencyQueryResponse re = new CopyDependencyQueryResponse();
            re.setCopyId(copy.getUuid());
            fillSingleCopyDependency(copy, re);
            return re;
        }).collect(Collectors.toList());
    }

    /**
     * fillSingleCopyDependency
     *
     * @param copy copy
     * @param re re
     */
    protected void fillSingleCopyDependency(Copy copy, CopyDependencyQueryResponse re) {
        Copy presentFullCopy = copyRestApi.queryLatestFullBackupCopies(copy.getResourceId(), copy.getGn(),
            BackupTypeEnum.FULL.getAbbreviation()).orElse(copy);
        int nextGn = presentFullCopy.getNextCopyGn() == 0
            ? presentFullCopy.getGn() + 1
            : presentFullCopy.getNextCopyGn();
        List<Copy> copies = copyRestApi.queryLaterCopiesByGeneratedBy(presentFullCopy.getResourceId(), nextGn,
                presentFullCopy.getGeneratedBy())
            .stream()
            .sorted(Comparator.comparing(Copy::getGn))
            .collect(Collectors.toList());
        Copy nextFullCopy = CopyUtil.getNextFullCopy(copies, presentFullCopy.getGn());
        List<String> resultCopyUuids = CopyUtil.getCopyUuidsBetweenTwoCopy(copies, presentFullCopy, nextFullCopy);
        re.setDependencyCopyUuidList(resultCopyUuids);
    }
}
