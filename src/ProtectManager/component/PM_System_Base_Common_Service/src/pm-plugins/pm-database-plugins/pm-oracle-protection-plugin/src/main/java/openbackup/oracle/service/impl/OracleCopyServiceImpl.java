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
package openbackup.oracle.service.impl;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.oracle.constants.ScnCopy;
import openbackup.oracle.service.OracleCopyService;

import org.springframework.stereotype.Service;

import java.util.List;
import java.util.stream.Collectors;

/**
 * Oracle副本相关的信息
 *
 */
@Service
@Slf4j
public class OracleCopyServiceImpl implements OracleCopyService {
    private static final String FILTER_TYPE = "scn";
    private static final String COPY_YPE = "full,increment,diff";

    private final DmeUnifiedRestApi dmeUnifiedRestApi;

    public OracleCopyServiceImpl(DmeUnifiedRestApi dmeUnifiedRestApi) {
        this.dmeUnifiedRestApi = dmeUnifiedRestApi;
    }

    @Override
    public List<ScnCopy> listCopiesInfo(String resourceId, String filterValue) {
        List<DmeCopyInfo> copyInfos = dmeUnifiedRestApi.listCopiesInfo(resourceId, FILTER_TYPE, filterValue, COPY_YPE);
        return copyInfos.stream().map(copy -> new ScnCopy(copy.getId(), copy.getTimestamp()))
                .collect(Collectors.toList());
    }
}
