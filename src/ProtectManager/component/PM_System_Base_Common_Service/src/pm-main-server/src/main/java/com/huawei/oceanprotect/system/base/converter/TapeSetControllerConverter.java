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
package com.huawei.oceanprotect.system.base.converter;

import com.huawei.oceanprotect.repository.tapelibrary.service.bo.MediaSetBo;
import com.huawei.oceanprotect.repository.tapelibrary.service.repository.TapeStoragePoolRepository;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.aspect.DataConverter;
import openbackup.system.base.common.aspect.OperationLoggingList;
import openbackup.system.base.common.utils.VerifyUtil;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collection;
import java.util.stream.Collectors;

/**
 * MediaSetId2name
 *
 */
@Component
@Slf4j
public class TapeSetControllerConverter implements DataConverter {
    private static final String DISPLAY_NULL = "--";

    @Autowired
    private TapeStoragePoolRepository tapeStoragePoolRepository;

    /**
     * 转化名
     *
     * @return converter name
     */
    @Override
    public String getName() {
        return "mediaSetId2name";
    }

    /**
     * 转化列表
     *
     * @param data data
     * @return 返回类型是Collection<List < String>>
     */
    @Override
    public Collection<?> convert(Collection<?> data) {
        return data.stream().map(this::handleData).collect(Collectors.toList());
    }

    /**
     * 处理数据
     *
     * @param item 进来的参数
     * @return 返回一个指定的List，实现对应多个占位符
     */
    private <T> OperationLoggingList handleData(T item) {
        log.info("entered mediaSetId2name converter");
        if (!(item instanceof String)) {
            return new OperationLoggingList();
        }
        OperationLoggingList result = new OperationLoggingList();
        if (VerifyUtil.isNone((String) item)) {
            result.add(DISPLAY_NULL);
        } else {
            MediaSetBo mediaSetBo = tapeStoragePoolRepository.queryTapeStorageBo((String) item);
            String name = mediaSetBo != null ? mediaSetBo.getMediaSetName() : DISPLAY_NULL;
            result.add(name);
        }
        return result;
    }
}
