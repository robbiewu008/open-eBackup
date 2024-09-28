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
package openbackup.data.access.framework.restore.controller.req;

import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.core.common.enums.v2.filter.ResourceFilter;
import openbackup.data.access.framework.restore.controller.RestoreController;
import openbackup.data.access.framework.restore.validator.CreateRestoreTaskRequestValidator;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreScript;
import openbackup.system.base.sdk.copy.model.Copy;

import com.fasterxml.jackson.annotation.JsonIgnore;

import org.hibernate.validator.constraints.Length;

import java.util.List;
import java.util.Map;

import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

/**
 * 创建恢复任务请求对象，用于Controller中使用
 *
 **/
public class CreateRestoreTaskRequest {
    /**
     * 副本ID
     */
    @NotBlank
    @Length(min = 32, max = 64)
    private String copyId;

    /**
     * 副本的详细信息，rest接口中需要忽略。
     * copy对象在参数校验器设置{@link CreateRestoreTaskRequestValidator}
     * copy对象在controller中操作日志记录时使用{@link RestoreController}
     */
    @JsonIgnore
    private Copy copy;

    /**
     * 恢复类型
     */
    @NotNull
    private RestoreTypeEnum restoreType;

    /**
     * 恢复位置
     */
    @NotNull
    private RestoreLocationEnum targetLocation;

    /**
     * 恢复子对象列表，细粒度恢复时指定要具体恢复的子对象，如：
     * 1、文件系统恢复时可以指定只恢复哪些目录
     * 2、虚拟机恢复时指定恢复哪些磁盘
     * 3、数据库实例恢复时指定恢复实例下的某个数据库
     */
    private List<String> subObjects;

    /**
     * 恢复的目标环境的ID
     */
    @NotBlank
    @Length(min = 1, max = 64)
    private String targetEnv;

    /**
     * 恢复的目标对象ID，如：
     * 1、虚拟机恢复时可以目标主机或集群的ID，或虚拟机ID
     * 2、文件系统恢复时，为目标的目录地址
     * 3、数据库恢复时为目标实例ID
     */
    @Length(min = 1, max = 256)
    private String targetObject;

    /**
     * 恢复时保护代理的ID列表
     */
    private List<String> agents;

    /**
     * 恢复参数，以key/value键值对存放
     */
    private Map<String, String> extendInfo;

    /**
     * 恢复时对某些要恢复的对象进行过滤，比如：
     * 比如根据一定的过滤规则只恢复某些类型文件或目录
     */
    private List<ResourceFilter> filters;

    /**
     * 在Agent上执行的相关恢复脚本
     */
    private RestoreScript scripts;

    private String exerciseId;

    private String exerciseJobId;

    public RestoreScript getScripts() {
        return scripts;
    }

    public void setScripts(RestoreScript scripts) {
        this.scripts = scripts;
    }

    public String getCopyId() {
        return copyId;
    }

    public void setCopyId(String copyId) {
        this.copyId = copyId;
    }

    public RestoreTypeEnum getRestoreType() {
        return restoreType;
    }

    public void setRestoreType(RestoreTypeEnum restoreType) {
        this.restoreType = restoreType;
    }

    public List<String> getSubObjects() {
        return subObjects;
    }

    public void setSubObjects(List<String> subObjects) {
        this.subObjects = subObjects;
    }

    public String getTargetEnv() {
        return targetEnv;
    }

    public void setTargetEnv(String targetEnv) {
        this.targetEnv = targetEnv;
    }

    public String getTargetObject() {
        return targetObject;
    }

    public void setTargetObject(String targetObject) {
        this.targetObject = targetObject;
    }

    public List<String> getAgents() {
        return agents;
    }

    public void setAgents(List<String> agents) {
        this.agents = agents;
    }

    public Map<String, String> getExtendInfo() {
        return extendInfo;
    }

    public void setExtendInfo(Map<String, String> extendInfo) {
        this.extendInfo = extendInfo;
    }

    public List<ResourceFilter> getFilters() {
        return filters;
    }

    public void setFilters(List<ResourceFilter> filters) {
        this.filters = filters;
    }

    public RestoreLocationEnum getTargetLocation() {
        return targetLocation;
    }

    public void setTargetLocation(RestoreLocationEnum targetLocation) {
        this.targetLocation = targetLocation;
    }

    public Copy getCopy() {
        return copy;
    }

    public void setCopy(Copy copy) {
        this.copy = copy;
    }

    public String getExerciseJobId() {
        return exerciseJobId;
    }

    public void setExerciseJobId(String exerciseJobId) {
        this.exerciseJobId = exerciseJobId;
    }

    public String getExerciseId() {
        return exerciseId;
    }

    public void setExerciseId(String exerciseId) {
        this.exerciseId = exerciseId;
    }
}
