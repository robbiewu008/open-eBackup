/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oracle.service.impl;

import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.oracle.constants.ScnCopy;
import openbackup.oracle.service.OracleCopyService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.List;
import java.util.stream.Collectors;

/**
 * Oracle副本相关的信息
 *
 * @author c30038333
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-04-04
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
