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
package openbackup.openstack.adapter.service;

import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import openbackup.openstack.adapter.constants.OpenStackErrorCodes;
import openbackup.openstack.adapter.dto.OpenStackCopyDto;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.unit.CapabilityUnitType;
import openbackup.system.base.common.utils.unit.UnitConvert;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 副本相关操作管理器
 *
 */
@Slf4j
@Component
public class OpenStackCopyManager {
    private static final String FILTER_KEY_RESOURCE_ID = "resource_id";

    private static final String ORDER_KEY_TIMESTAMP_ASC = "+timestamp";

    // 默认缩减前数据量，单位B
    private static final int DEFAULT_DATA_BEFORE_REDUCTION = 1024 * 1024 * 1024;

    // 默认副本大小，单位G
    private static final int DEFAULT_COPY_SIZE = 1;

    private final CopyRestApi copyRestApi;

    private final OpenStackUserManager userManager;

    public OpenStackCopyManager(CopyRestApi copyRestApi, OpenStackUserManager userManager) {
        this.copyRestApi = copyRestApi;
        this.userManager = userManager;
    }

    /**
     * 删除副本
     *
     * @param copyId 副本id
     */
    public void deleteCopy(String copyId) {
        String userId = userManager.obtainUserId();
        try {
            copyRestApi.deleteCopy(copyId, userId, false, false, JobTypeEnum.COPY_DELETE.getValue());
            log.info("Delete copy: {} of user: {} success.", copyId, userId);
        } catch (LegoUncheckedException | FeignException exception) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                String.format("Delete copy: %s fail.", copyId));
        }
    }

    /**
     * 查询指定副本信息
     *
     * @param copyId 副本id
     * @return 副本信息
     */
    public OpenStackCopyDto queryCopy(String copyId) {
        try {
            Copy copy = copyRestApi.queryCopyByID(copyId, false);
            if (copy == null) {
                throw new OpenStackException(OpenStackErrorCodes.NOT_FOUND,
                    String.format("Copy: %s is not exists.", copyId));
            }
            List<Copy> copies = queryCopiesByResourceId(copy.getResourceId());
            List<String> copyIds = copies.stream().map(Copy::getUuid).collect(Collectors.toList());
            OpenStackCopyDto result = generateOpenStackCopyDto(copy, copyIds);
            log.info("Query copy: {} success, index: {}.", copyId, result.getGenerateId());
            return result;
        } catch (LegoUncheckedException | FeignException exception) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, String.format("Query copy: %s fail.", copyId),
                exception);
        }
    }

    /**
     * 查询资源下所有备份副本
     *
     * @param resourceId 资源id
     * @return 所有备份副本信息
     */
    public List<OpenStackCopyDto> queryCopies(String resourceId) {
        List<Copy> copies = queryCopiesByResourceId(resourceId);
        List<String> copyIds = copies.stream().map(Copy::getUuid).collect(Collectors.toList());
        List<OpenStackCopyDto> result = new ArrayList<>();
        for (Copy copy : copies) {
            result.add(generateOpenStackCopyDto(copy, copyIds));
        }
        log.info("Openstack resource: {} has: {} copies.", resourceId, result.size());
        return result;
    }

    private List<Copy> queryCopiesByResourceId(String resourceId) {
        Map<String, Object> filters = new HashMap<>();
        filters.put(FILTER_KEY_RESOURCE_ID, resourceId);
        // 按timestamp升序查询
        List<String> orders = Collections.singletonList(ORDER_KEY_TIMESTAMP_ASC);

        List<Copy> result = new ArrayList<>();
        BasePage<Copy> response;
        int pageNo = 0;
        int pageSize = 100;
        do {
            response = copyRestApi.queryCopies(pageNo, pageSize, filters, orders);
            result.addAll(response.getItems());
            pageNo++;
        } while (response.getItems().size() == pageSize);
        return result;
    }

    private OpenStackCopyDto generateOpenStackCopyDto(Copy copy, List<String> copyIds) {
        OpenStackCopyDto copyDto = new OpenStackCopyDto();
        copyDto.setId(copy.getUuid());
        copyDto.setBackupJobId(copy.getResourceId());
        copyDto.setGenerateTime(copy.getGeneratedTime());
        // 副本大小使用哪个值待确定
        copyDto.setSize(getCopySize(copy.getProperties()));

        int index = copyIds.indexOf(copy.getUuid());
        copyDto.setGenerateId(index + 1);
        copyDto.setLatest(isLast(index, copyIds.size()));
        return copyDto;
    }

    private int getCopySize(String properties) {
        if (StringUtils.isBlank(properties)) {
            return DEFAULT_COPY_SIZE;
        }
        int copySize = JSONObject.fromObject(properties)
            .getInt(JobExtendInfoKeys.DATA_BEFORE_REDUCTION, DEFAULT_DATA_BEFORE_REDUCTION);
        log.info("Openstack copy size: {} B.", copySize);
        return (int) Math.ceil(UnitConvert.convert(copySize, CapabilityUnitType.BYTE, CapabilityUnitType.GB, 2));
    }

    private boolean isLast(int index, int size) {
        return index == (size - 1);
    }
}
