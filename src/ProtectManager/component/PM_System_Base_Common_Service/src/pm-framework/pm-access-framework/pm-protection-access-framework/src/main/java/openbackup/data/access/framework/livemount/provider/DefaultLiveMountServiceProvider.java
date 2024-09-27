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
package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.client.sdk.api.framework.dme.DmeBackupClone;
import openbackup.data.access.client.sdk.api.framework.dme.DmeFileSystemShareInfo;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountEnableStatus;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import com.huawei.oceanprotect.exercise.service.ExerciseQueryService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * 功能描述: DefaultLiveMountServiceProvider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-01
 */
@Slf4j
@Component
public class DefaultLiveMountServiceProvider implements LiveMountServiceProvider {
    @Autowired
    private ExerciseQueryService exerciseQueryService;

    @Autowired
    private JobService jobService;

    @Override
    public boolean applicable(String subType) {
        return false;
    }

    @Override
    public DmeBackupClone buildDmeCloneCopyRequest(CloneCopyParam cloneCopyParam) {
        DmeBackupClone clone = new DmeBackupClone();
        clone.setSrcCopyId(cloneCopyParam.getBackupId());
        clone.setTargetCopyId(cloneCopyParam.getCloneBackupId());
        List<DmeFileSystemShareInfo> fileSystemShareInfoList = cloneCopyParam.getFileSystemShareInfo().stream()
                        .map(fileSystemShareInfo -> BeanTools.copy(fileSystemShareInfo, DmeFileSystemShareInfo::new))
                        .collect(Collectors.toList());
        if (CollectionUtils.isNotEmpty(fileSystemShareInfoList)) {
            clone.setFileSystemName(fileSystemShareInfoList.get(0).getFileSystemName());
        }
        return clone;
    }

    @Override
    public boolean isSourceCopyCanBeMounted(Copy copy, boolean isManual) {
        if (copy == null) {
            if (isManual) {
                throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not found available copy");
            } else {
                return false;
            }
        }
        return true;
    }

    @Override
    public LiveMountEntity buildLiveMountEntity(LiveMountObject liveMountObject, ResourceEntity sourceResourceEntity,
            ResourceEntity targetResourceEntity) {
        LiveMountEntity entity = new LiveMountEntity();
        entity.setId(UUID.randomUUID().toString());
        entity.setResourceId(liveMountObject.getSourceResourceId());
        entity.setResourceType(sourceResourceEntity.getType());
        entity.setResourceSubType(sourceResourceEntity.getSubType());
        entity.setResourceName(sourceResourceEntity.getName());
        entity.setResourcePath(Optional.ofNullable(sourceResourceEntity.getPath()).orElse(""));
        entity.setResourceIp(sourceResourceEntity.getEnvironmentEndPoint());
        entity.setPolicyId(liveMountObject.getPolicyId());
        entity.setCopyId(liveMountObject.getCopyId());
        entity.setTargetLocation(liveMountObject.getTargetLocation().getValue());
        entity.setTargetResourceId(targetResourceEntity.getUuid());
        // VMs can be mounted to the same environment resource.
        if (ResourceSubTypeEnum.VMWARE.getType().equals(sourceResourceEntity.getSubType())
                || ResourceSubTypeEnum.FILESET.getType().equals(sourceResourceEntity.getSubType())) {
            entity.setTargetResourceName(String.valueOf(liveMountObject.getParameters().get("name")));
        } else {
            entity.setTargetResourceName(sourceResourceEntity.getName());
        }
        // 此处目标资源路径入库不能为空，必须有值
        entity.setTargetResourcePath(StringUtils.isEmpty(targetResourceEntity.getPath())
                ? liveMountObject.getTargetLocation().getValue()
                : targetResourceEntity.getPath());
        entity.setTargetResourceIp(targetResourceEntity.getEnvironmentEndPoint());
        entity.setParameters(JSONObject.fromObject(liveMountObject.getParameters()).toString());
        entity.setStatus(LiveMountStatus.READY.getName());
        entity.setUserId(getUserId(liveMountObject));
        entity.setEnableStatus(LiveMountEnableStatus.ACTIVATED.getName());
        entity.setFileSystemShareInfo(JSONArray.fromObject(liveMountObject.getFileSystemShareInfoList()).toString());
        String isDeleteOriginalVM = "false";
        if (liveMountObject.getParameters().containsKey("isDeleteOriginalVM")) {
            isDeleteOriginalVM = String.valueOf(liveMountObject.getParameters().get("isDeleteOriginalVM"));
            log.info("Livemount set isDeleteOriginalVM: " + isDeleteOriginalVM);
        }
        entity.setDeleteOriginalVM(Boolean.parseBoolean(isDeleteOriginalVM));
        return entity;
    }

    private String getUserId(LiveMountObject liveMountObject) {
        if (VerifyUtil.isEmpty(liveMountObject.getExerciseId())) {
            return TokenBo.get().getUser().getId();
        }
        return exerciseQueryService.queryExercise(liveMountObject.getExerciseId()).getUserId();
    }

    @Override
    public void processLiveMountTerminate(LiveMountEntity liveMountEntity) {
    }
}