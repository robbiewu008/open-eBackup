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
package openbackup.data.access.framework.restore.listener.v1;

import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Filter;
import openbackup.data.protection.access.provider.sdk.base.Parameter;
import openbackup.data.protection.access.provider.sdk.restore.RestoreObject;
import openbackup.data.protection.access.provider.sdk.restore.RestoreProvider;
import openbackup.data.protection.access.provider.sdk.restore.RestoreTarget;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * Restore msg listener, this lister will consume the restore msg and the specified provider is found by provider
 * registry to complete the restore service logic processing.
 *
 */
@Component
@Slf4j
public class RestoreListener {
    private static final String JOB_STATUS_LOG = "job_status_{payload?.job_status|status}_label";

    private static final String FLR_DOWNLOAD = "download";

    private static final List<String> VM_RESOURCE_TYPES = Collections.unmodifiableList(
        Arrays.asList(ResourceSubTypeEnum.VMWARE.getType(), ResourceSubTypeEnum.FUSION_COMPUTE.getType(),
            ResourceSubTypeEnum.HCS_CLOUD_HOST.getType(), ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType(),
            ResourceSubTypeEnum.APS_INSTANCE.getType(), ResourceSubTypeEnum.CNWARE_VM.getType(),
            ResourceSubTypeEnum.NDMP_BACKUPSET.getType(), ResourceSubTypeEnum.FUSION_ONE_COMPUTE.getType()));

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private FunctionSwitchService functionSwitchService;

    @Autowired
    private JobService jobService;

    /**
     * Consume restore topic message
     *
     * @param msg Restore msg
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.EXECUTE_RESTORE, containerFactory = "retryFactory",
        log = {"job_log_protection_restore_execute_label", JOB_STATUS_LOG})
    public void restore(String msg, Acknowledgment acknowledgment) {
        if (msg == null) {
            log.info("Restore msg is null");
            return;
        }

        Map map = JSONObject.parseObject(msg, Map.class);
        String requestId = String.valueOf(map.get("request_id"));
        RMap<String, String> rMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        RestoreObject restoreObject = convertToRestoreObject(rMap);

        // Oracle SCN和时间点恢复在PM上没有副本
        Copy copy = copyRestApi.queryCopyByID(restoreObject.getCopyId(), false);
        if (copy == null) {
            copy = new Copy();
            copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        }

        restoreObject.setCopyGeneratedBy(copy.getGeneratedBy());
        RestoreProvider provider = providerManager.findProvider(RestoreProvider.class, restoreObject);
        provider.restore(restoreObject);
    }

    private RestoreObject convertToRestoreObject(RMap<String, String> rMap) {
        RestoreObject restoreObject = new RestoreObject();
        restoreObject.setRequestId(rMap.get("request_id"));
        restoreObject.setCopyId(rMap.get("copy_id"));
        restoreObject.setObjectType(rMap.get("object_type"));
        restoreObject.setRestoreLocation(rMap.get("restore_location"));
        restoreObject.setRestoreType(rMap.get("restore_type"));
        // 细粒度恢复，下载文件
        restoreObject.setRecordId(rMap.get("record_id"));
        // set restoreTarget 恢复目的对象
        String target = rMap.get("target");
        RestoreTarget restoreTarget = JSONObject.parseObject(target, RestoreTarget.class);
        restoreObject.setTarget(restoreTarget);
        // set ext_parameters 高级参数
        String parameters = rMap.get("ext_parameters");
        Map map = JSONObject.parseObject(parameters, Map.class);
        restoreObject.setParameters(getParameters(map));
        // set restoreObjects 恢复对象
        String restoreObjects = rMap.get("restore_objects");
        List<String> restoreObjectsList = JSONObject.parseArray(restoreObjects, String.class);
        restoreObject.setRestoreObjects(restoreObjectsList);
        // set filters 过滤条件
        String filters = rMap.get("filters");
        List<Filter> filterList = JSONArray.fromObject(filters).toBean(Filter.class);
        restoreObject.setFilters(filterList);
        return restoreObject;
    }

    private List<Parameter> getParameters(Map map) {
        if (map == null) {
            return Collections.emptyList();
        }
        List<Parameter> parameters = new ArrayList<>();
        map.forEach((key, value) -> {
            Parameter parameter = new Parameter();
            if (key != null && value != null) {
                parameter.setKey(key.toString());
                parameter.setValue(value.toString());
            }
            parameters.add(parameter);
        });
        return parameters;
    }
}
