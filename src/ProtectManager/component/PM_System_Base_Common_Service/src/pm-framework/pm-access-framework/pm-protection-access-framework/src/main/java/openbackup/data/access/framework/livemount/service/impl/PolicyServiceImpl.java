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
package openbackup.data.access.framework.livemount.service.impl;

import com.huawei.oceanprotect.system.base.user.common.enums.ResourceSetScopeModuleEnum;
import com.huawei.oceanprotect.system.base.user.entity.ResourceSetResourceBo;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.LambdaUpdateWrapper;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.livemount.bo.PolicyBo;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.enums.ScheduledUnit;
import openbackup.data.access.framework.livemount.controller.policy.request.CreatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.request.UpdatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.response.LiveMountPolicyVo;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.data.access.framework.livemount.service.PolicyService;
import openbackup.data.protection.access.provider.sdk.livemount.PolicyServiceApi;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.DateFormatUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.query.PageQueryParam;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.sql.Timestamp;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.UUID;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * 更新策略接口实现
 *
 */
@Slf4j
@Service
public class PolicyServiceImpl implements PolicyService, PolicyServiceApi {
    private static final String DEFAULT_SORTING = "created_time desc";

    private static final String COMMON_NAME_REGEX = "[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-]{0,63}$";

    private static final int MAX_POLICY_NUM = IsmNumberConstant.HUNDRED_TWENTY_EIGHT;

    @Autowired
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @Autowired
    private LiveMountService liveMountService;

    @Autowired
    private PageQueryService pageQueryService;

    @Autowired
    private ResourceSetApi resourceSetApi;

    @Override
    public void createPolicy(CreatePolicyRequest createRequest, TokenBo.UserBo user) {
        // check create request
        PolicyBo policyBo = new PolicyBo();
        BeanUtils.copyProperties(createRequest, policyBo);
        checkPolicyRequest(policyBo);
        LiveMountPolicyEntity entity = liveMountPolicyEntityDao.selectOne(
            new QueryWrapper<LiveMountPolicyEntity>().eq("name", policyBo.getName()));

        int count = liveMountPolicyEntityDao.selectCount(new QueryWrapper<>()).intValue();
        if (count >= MAX_POLICY_NUM) {
            throw new LegoCheckedException(CommonErrorCode.LIVE_MOUNT_POLICY_COUNT_OVER_LIMIT,
                "policy count reach limit 128.");
        }
        // check duplicate name
        if (!VerifyUtil.isEmpty(entity)) {
            throw new LegoCheckedException(CommonErrorCode.LIVE_MOUNT_POLICY_NAME_DUPLICATE,
                "policy name is duplicate.");
        } else {
            entity = new LiveMountPolicyEntity();
        }

        BeanUtils.copyProperties(policyBo, entity);
        entity = convertToPolicyEntity(entity, policyBo);
        entity.setPolicyId(UUID.randomUUID().toString());
        liveMountPolicyEntityDao.insert(entity);
        createResourceSetRelation(entity, user);
    }

    private void createResourceSetRelation(LiveMountPolicyEntity entity, TokenBo.UserBo user) {
        ResourceSetResourceBo resourceSetResourceBo = new ResourceSetResourceBo();
        resourceSetResourceBo.setResourceObjectId(entity.getPolicyId());
        resourceSetResourceBo.setType(ResourceSetTypeEnum.LIVE_MOUNT_POLICY);
        resourceSetResourceBo.setScopeModule(ResourceSetScopeModuleEnum.LIVEMOUNT_POLICY.getType());
        resourceSetResourceBo.setDomainIdList(Collections.singletonList(user.getDomainId()));
        resourceSetApi.addResourceSetRelation(resourceSetResourceBo);
    }

    @Override
    public BasePage<LiveMountPolicyEntity> getPolices(int page, int size, String conditions, List<String> orders) {
        JSONObject conditionsObject = JSONObject.fromObject(conditions);
        fillPolicyConditions(conditionsObject);
        return pageQueryService.pageQuery(LiveMountPolicyEntity.class, liveMountPolicyEntityDao::page,
            new PageQueryParam(page, size, conditionsObject.toString(), orders), "-created_time",
            "policy_id");
    }
    private void fillPolicyConditions(JSONObject conditions) {
        if (VerifyUtil.isEmpty(conditions) || !conditions.containsKey("resourceSetId")) {
            return;
        }
        String resourceSetId = getValueFromCondition("resourceSetId", conditions);
        if (!VerifyUtil.isEmpty(resourceSetId)) {
            List<String> resourceIdList = resourceSetApi.getAllRelationsByResourceSetId(resourceSetId,
                ResourceSetTypeEnum.LIVE_MOUNT_POLICY.getType());
            if (VerifyUtil.isEmpty(resourceIdList)) {
                resourceIdList.add(UUID.randomUUID().toString());
            }
            conditions.put("policyId", resourceIdList);
            conditions.remove("resourceSetId");
        }
    }

    private String getValueFromCondition(String key, JSONObject conditions) {
        String value;
        Object resourceSetIdObj = conditions.get(key);
        if (resourceSetIdObj instanceof String) {
            value = (String) resourceSetIdObj;
        } else {
            value = StringUtils.EMPTY;
        }
        return value;
    }
    @Override
    public LiveMountPolicyVo getPolicy(String id) {
        LiveMountPolicyEntity entity = liveMountPolicyEntityDao.selectPolicy(id);
        if (VerifyUtil.isEmpty(entity)) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST);
        }
        return displayDataByPolicyType(entity);
    }

    @Override
    @Transactional
    public void updatePolicy(String policyId, UpdatePolicyRequest updateRequest) {
        PolicyBo policyBo = new PolicyBo();
        checkPolicyName(updateRequest.getName());

        BeanUtils.copyProperties(updateRequest, policyBo);
        checkPolicyRequest(policyBo);
        // whether policy name is duplicate
        List<LiveMountPolicyEntity> historyPolicy = liveMountPolicyEntityDao.selectList(
            new QueryWrapper<LiveMountPolicyEntity>().eq("name", updateRequest.getName()));
        if (!VerifyUtil.isEmpty(historyPolicy)) {
            if (historyPolicy.stream().anyMatch(entity -> !entity.getPolicyId().equals(policyId))) {
                throw new LegoCheckedException(CommonErrorCode.LIVE_MOUNT_POLICY_NAME_DUPLICATE,
                    "policy name is duplicate.");
            }
        }

        LiveMountPolicyEntity entity = new LiveMountPolicyEntity();
        BeanUtils.copyProperties(policyBo, entity);
        entity.setPolicyId(policyId);
        entity = convertToPolicyEntity(entity, policyBo);
        liveMountPolicyEntityDao.updateById(entity);
        // get the number of live mounts associated with a policy.
        List<LiveMountEntity> liveMountEntities = liveMountService.queryLiveMountEntitiesByPolicyId(policyId);

        LiveMountPolicyEntity policyEntity = selectPolicyById(policyId);
        // 如果周期执行时间有改变，重新初始化周期调度
        if (checkPeriodScheduleHasChanged(policyEntity, updateRequest)) {
            // update live mount scheduled
            liveMountEntities.forEach(item -> liveMountService.initialAndUpdateLiveMountSchedule(item, policyId));
        }
    }

    /**
     * 判断策略按周期调度是否有修改
     *
     * @param policyEntity policy entity
     * @param updateRequest update request
     * @return changed status
     */
    private boolean checkPeriodScheduleHasChanged(LiveMountPolicyEntity policyEntity,
        UpdatePolicyRequest updateRequest) {
        if (VerifyUtil.isEmpty(updateRequest.getSchedulePolicy()) || !ScheduledType.PERIOD_SCHEDULE.getName()
            .equals(updateRequest.getSchedulePolicy().getName())) {
            return false;
        }

        return !Objects.equals(policyEntity.getScheduleStartTime(),
            Timestamp.valueOf(updateRequest.getScheduleStartTime())) || !Objects.equals(
            policyEntity.getScheduleInterval(), (updateRequest.getScheduleInterval())) || !Objects.equals(
            policyEntity.getScheduleIntervalUnit(), (updateRequest.getScheduleIntervalUnit().getName()));
    }

    @Transactional
    @Override
    public void deletePolicy(String policyId) {
        // get the number of live mounts associated with a policy.
        List<LiveMountEntity> liveMountEntities = liveMountService.queryLiveMountEntitiesByPolicyId(policyId);
        if (!VerifyUtil.isEmpty(liveMountEntities)) {
            throw new LegoCheckedException(CommonErrorCode.LIVE_MOUNT_POLICY_BOUND_RESOURCE,
                "the policy has association live mount.");
        }

        // delete one
        liveMountPolicyEntityDao.deleteById(policyId);
        resourceSetApi.deleteResourceSetRelation(policyId, ResourceSetTypeEnum.LIVE_MOUNT_POLICY);
    }

    @Override
    public LiveMountPolicyEntity selectPolicyById(String policyId) {
        LiveMountPolicyEntity policyEntity = liveMountPolicyEntityDao.selectById(policyId);
        if (policyEntity == null) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "policy is not exist. policy id is " + policyId);
        }
        return policyEntity;
    }

    @Override
    public void revokeLiveMountPolicyUserId(String userId) {
        LambdaUpdateWrapper<LiveMountPolicyEntity> wrapper = new LambdaUpdateWrapper<>();
        wrapper.eq(LiveMountPolicyEntity::getUserId, userId).set(LiveMountPolicyEntity::getUserId, null);

        liveMountPolicyEntityDao.update(null, wrapper);
    }

    private void checkPolicyRequest(PolicyBo createRequest) {
        // interval
        if (ScheduledType.PERIOD_SCHEDULE.equals(createRequest.getSchedulePolicy())) {
            if (VerifyUtil.isEmpty(createRequest.getScheduleIntervalUnit())) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "");
            }
            if (VerifyUtil.isEmpty(createRequest.getScheduleInterval())
                || createRequest.getScheduleInterval() < IsmNumberConstant.ONE) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "");
            }
            // Check whether the schedule value matches the unit.
            if (!checkScheduledValueByUnit(createRequest.getScheduleInterval(),
                createRequest.getScheduleIntervalUnit())) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "");
            }
            if (!isTimestamp(createRequest.getScheduleStartTime())) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "");
            }
        }

        // keep for
        if (RetentionType.FIXED_TIME.equals(createRequest.getRetentionPolicy())) {
            if (VerifyUtil.isEmpty(createRequest.getRetentionUnit())) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "");
            }
            if (VerifyUtil.isEmpty(createRequest.getRetentionValue())
                || createRequest.getRetentionValue() < IsmNumberConstant.ONE) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "");
            }
            // Check whether the retention value matches the unit.
            if (!checkRetentionValueByUnit(createRequest.getRetentionValue(), createRequest.getRetentionUnit())) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "");
            }
        }
    }

    private LiveMountPolicyEntity convertToPolicyEntity(LiveMountPolicyEntity policyEntity, PolicyBo policyBo) {
        LiveMountPolicyEntity entity = new LiveMountPolicyEntity();
        BeanUtils.copyProperties(policyEntity, entity);

        if (!VerifyUtil.isEmpty(policyBo.getScheduleStartTime())) {
            entity.setScheduleStartTime(Timestamp.valueOf(policyBo.getScheduleStartTime()));
        }

        if (!VerifyUtil.isEmpty(policyBo.getScheduleIntervalUnit())) {
            entity.setScheduleIntervalUnit(policyBo.getScheduleIntervalUnit().getName());
        }

        if (!VerifyUtil.isEmpty(policyBo.getRetentionUnit())) {
            entity.setRetentionUnit(policyBo.getRetentionUnit().getName());
        }

        if (!ScheduledType.PERIOD_SCHEDULE.equals(policyBo.getSchedulePolicy())) {
            entity.setScheduleIntervalUnit(null);
            entity.setScheduleInterval(null);
            entity.setScheduleStartTime(null);
        }

        if (!RetentionType.FIXED_TIME.equals(policyBo.getRetentionPolicy())) {
            entity.setRetentionUnit(null);
            entity.setRetentionValue(null);
        }
        entity.setCopyDataSelectionPolicy(policyBo.getCopyDataSelectionPolicy().getName());
        entity.setRetentionPolicy(policyBo.getRetentionPolicy().getName());
        entity.setSchedulePolicy(policyBo.getSchedulePolicy().getName());
        return entity;
    }

    private Boolean checkScheduledValueByUnit(Integer val, ScheduledUnit unit) {
        switch (unit) {
            case HOUR:
                if (val >= 1 && val <= 23) {
                    return true;
                }
                break;
            case DAY:
                if (val >= 1 && val <= 30) {
                    return true;
                }
                break;
            case WEEK:
                if (val >= 1 && val <= 4) {
                    return true;
                }
                break;
            default:
                return false;
        }
        return false;
    }

    private Boolean checkRetentionValueByUnit(Integer val, RetentionUnit unit) {
        switch (unit) {
            case DAY:
                if (val >= 1 && val <= 365) {
                    return true;
                }
                break;
            case WEEK:
                if (val >= 1 && val <= 54) {
                    return true;
                }
                break;
            case MONTH:
                if (val >= 1 && val <= 24) {
                    return true;
                }
                break;
            case YEAR:
                if (val >= 1 && val <= 10) {
                    return true;
                }
                break;
        }
        return false;
    }

    private static Boolean isTimestamp(String str) {
        if (VerifyUtil.isEmpty(str)) {
            return false;
        }
        try {
            Timestamp.valueOf(str);
            return true;
        } catch (IllegalArgumentException e) {
            return false;
        }
    }

    private LiveMountPolicyVo displayDataByPolicyType(LiveMountPolicyEntity entity) {
        LiveMountPolicyVo policyVo = new LiveMountPolicyVo();
        BeanUtils.copyProperties(entity, policyVo);
        if (!VerifyUtil.isEmpty(entity.getScheduleStartTime())) {
            policyVo.setScheduleStartTime(
                DateFormatUtil.format(Constants.SIMPLE_DATE_FORMAT, entity.getScheduleStartTime()));
        }

        if (!entity.getSchedulePolicy().equals(ScheduledType.PERIOD_SCHEDULE.getName())) {
            policyVo.setScheduleStartTime(null);
            policyVo.setScheduleInterval(null);
            policyVo.setScheduleIntervalUnit(null);
        }

        if (!entity.getRetentionPolicy().equals(RetentionType.FIXED_TIME.getName())) {
            policyVo.setRetentionValue(null);
            policyVo.setRetentionUnit(null);
        }
        return policyVo;
    }

    private void checkPolicyName(String name) {
        if (name != null) {
            boolean isMatch = Pattern.matches(COMMON_NAME_REGEX, name);
            if (!isMatch) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "The name validation failed, The correct format is: The value is a string of 1 to 64 characters "
                        + "and can contain only digits, letters, underscores (_), and hyphens (-). "
                        + "It must start with a letter, Chinese character, or underscore (_).");
            }
        }
    }

    @Override
    public boolean existPolicy(String policyId) {
        LiveMountPolicyEntity policyEntity = liveMountPolicyEntityDao.selectById(policyId);
        if (policyEntity == null) {
            return false;
        }
        return true;
    }

    @Override
    public List<String> getPolicyIdList() {
        LambdaQueryWrapper<LiveMountPolicyEntity> wrapper = new LambdaQueryWrapper<>();
        return liveMountPolicyEntityDao.selectList(wrapper)
            .stream()
            .map(LiveMountPolicyEntity::getPolicyId)
            .collect(Collectors.toList());
    }

    @Override
    public Integer getAllCount() {
        return liveMountPolicyEntityDao.getAllCount();
    }
}
