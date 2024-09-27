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
package openbackup.data.access.framework.protection.common.util;

import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

import lombok.AccessLevel;
import lombok.RequiredArgsConstructor;

import org.apache.logging.log4j.util.Strings;

import java.util.List;
import java.util.function.Consumer;
import java.util.function.Function;

/**
 * 任务触发策略信息填充工具类
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-04-30
 */
@RequiredArgsConstructor(access = AccessLevel.PRIVATE)
public class JobExtendInfoUtil {
    private static final String PROTECTED_OBJ_EXT_PARAM = "protected_obj_ext_param";

    /**
     * 获取策略对象存入扩展信息中
     *
     * @param insertJob insertJob
     * @param policySupplier 获取策略方法
     * @param policyFiledGetter 获取策略id方法
     * @param policyType 策略类型
     * @param additionalAccess 策略额外操作
     * @param <T> T
     */
    public static <T, R> void fillJobPolicyInfo(Job insertJob, Function<String, R> policySupplier,
        Function<ObjectNode, String> policyFiledGetter, Class<T> policyType, Consumer<T> additionalAccess) {
        if (VerifyUtil.isEmpty(insertJob.getExtendStr())) {
            return;
        }
        ObjectNode jobExtendFiled = JsonUtil.read(insertJob.getExtendStr(), new TypeReference<ObjectNode>() {});
        String policyId = policyFiledGetter.apply(jobExtendFiled);
        if (VerifyUtil.isEmpty(policyId)) {
            return;
        }
        R policy = policySupplier.apply(policyId);
        if (policy == null) {
            return;
        }
        T jobPolicyDetail = JsonUtil.cast(policy, policyType);
        if (additionalAccess != null) {
            additionalAccess.accept(jobPolicyDetail);
        }
        jobExtendFiled.set(JobExtendInfoKeys.TRIGGER_POLICY, JsonUtil.cast(jobPolicyDetail, ObjectNode.class));
        insertJob.setExtendStr(jobExtendFiled.toString());
    }

    /**
     * 在任务扩展字段中根据路径塞值
     *
     * @param insertJob 任务
     * @param path 设置key的路径
     * @param value 设置值
     */
    public static void insertValueToExtStr(Job insertJob, List<String> path, Object value) {
        JSONObject root;
        if (!VerifyUtil.isEmpty(insertJob.getExtendStr())) {
            root = JSONObject.fromObject(insertJob.getExtendStr());
        } else {
            root = new JSONObject();
        }
        JSONObject curNode = root;
        for (int i = 0; i < path.size() - 1; i++) {
            String key = path.get(i);
            if (curNode.containsKey(key)) {
                curNode = curNode.getJSONObject(key);
            } else {
                JSONObject newNode = new JSONObject();
                curNode.put(key, newNode);
                curNode = newNode;
            }
        }
        curNode.set(path.get(path.size() - 1), value);
        insertJob.setExtendStr(root.toString());
    }

    /**
     * 获取任务扩展信息对应filed的值
     *
     * @param jobExtendFiled 任务扩展字段
     * @param filed 字段
     * @return 字段值
     */
    public static String getExtInfo(ObjectNode jobExtendFiled, String filed) {
        JsonNode filedNode = jobExtendFiled.get(filed);
        if (filedNode == null) {
            return Strings.EMPTY;
        }
        return filedNode.textValue();
    }

    /**
     * 获取保护对象的扩展参数
     *
     * @param job 任务
     * @return 保护对象扩展参数
     */
    public static JSONObject getProtectedObjExtParam(JobBo job) {
        JSONObject jobExtendStr = JSONObject.fromObject(job.getExtendStr());
        JSONObject jobTriggerPolicy = jobExtendStr.getJSONObject(JobExtendInfoKeys.TRIGGER_POLICY);
        JSONObject protectedObjExtParam = null;
        if (jobTriggerPolicy != null) {
            protectedObjExtParam = jobTriggerPolicy.getJSONObject(PROTECTED_OBJ_EXT_PARAM);
        }
        return protectedObjExtParam;
    }
}
