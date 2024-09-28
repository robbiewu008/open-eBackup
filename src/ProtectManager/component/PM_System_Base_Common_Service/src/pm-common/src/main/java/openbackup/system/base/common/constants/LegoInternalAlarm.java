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
package openbackup.system.base.common.constants;

import lombok.Data;

/**
 * Lego告警
 *
 */
@Data
public class LegoInternalAlarm {
    // 对应WebNms Alert表的主键entity
    private String entity;

    // 告警ID
    private String alarmId;

    // 告警级别
    private int alarmLevel = FaultEnum.AlarmSeverity.INVALID.getValue();

    // 告警产生时间
    private long alarmTime = System.currentTimeMillis() / IsmNumberConstant.THOUSAND;

    // 告警流水号
    private long alarmSequence = -IsmNumberConstant.ONE;

    // 对应Lego ManagedObject对象的MOID属性
    private String moId ;

    // 网元显示名称
    private String moName;

    // 网元IP
    private String moIp = "127.0.0.1";

    // 告警参数
    private String[] alarmParam;

    // 告警产生对象
    private String sourceId;

    // 对象类型：资源、业务对象
    private String sourceType;

    // 造成告警用户ID
    private String userId;

    // 告警关联资源ID
    private String resourceId;

    /**
     * sourceId
     *
     * @return 返回 sourceId
     */
    public String getSourceId() {
        return sourceId;
    }

    /**
     * sourceId
     *
     * @param sourceId 对sourceId进行赋值
     */
    public void setSourceId(String sourceId) {
        this.sourceId = sourceId;
    }

    /**
     * sourceType
     *
     * @return 返回 sourceType
     */
    public String getSourceType() {
        return sourceType;
    }

    /**
     * sourceType
     *
     * @param sourceType 进行赋值
     */
    public void setSourceType(String sourceType) {
        this.sourceType = sourceType;
    }

    /**
     * getNmsSequence
     *
     * @return 返回 nmsSequence
     */

    /**
     * getAlarmId
     *
     * @return 返回 alarmId
     */
    public String getAlarmId() {
        return alarmId;
    }

    /**
     * setAlarmId
     *
     * @param alarmId 对alarmId进行赋值
     */
    public void setAlarmId(String alarmId) {
        this.alarmId = alarmId;
    }

    /**
     * getAlarmLevel
     *
     * @return 返回 alarmLevel
     */
    public int getAlarmLevel() {
        return alarmLevel;
    }

    /**
     * setAlarmLevel
     *
     * @param alarmLevel 对alarmLevel进行赋值
     */
    public void setAlarmLevel(FaultEnum.AlarmSeverity alarmLevel) {
        this.alarmLevel = alarmLevel.getValue();
    }

    /**
     * getAlarmTime
     *
     * @return 返回 alarmTime
     */
    public long getAlarmTime() {
        return alarmTime;
    }

    /**
     * setAlarmTime
     *
     * @param alarmTime 对alarmTime进行赋值
     */
    public void setAlarmTime(long alarmTime) {
        this.alarmTime = alarmTime;
    }

    /**
     * getAlarmSequence
     *
     * @return 返回 alarmSequence
     */
    public long getAlarmSequence() {
        return alarmSequence;
    }

    /**
     * setAlarmSequence
     *
     * @param alarmSequence 对alarmSequence进行赋值
     */
    public void setAlarmSequence(long alarmSequence) {
        this.alarmSequence = alarmSequence;
    }

    /**
     * getMoId
     *
     * @return 返回 moId
     */
    public String getMoId() {
        return moId;
    }

    /**
     * setMoId
     *
     * @param moId 对moIdyt进行赋值
     */
    public void setMoId(String moId) {
        this.moId = moId;
    }

    /**
     * getMoName
     *
     * @return 返回 moName
     */
    public String getMoName() {
        return moName;
    }

    /**
     * setMoName
     *
     * @param moNamey 对moNamey进行赋值
     */
    public void setMoName(String moNamey) {
        this.moName = moNamey;
    }

    /**
     * getMoIP
     *
     * @return 返回 moIp
     */
    public String getMoIp() {
        return moIp;
    }

    /**
     * setMoIP
     *
     * @param moIp 对moIP进行赋值
     */
    public void setMoIp(String moIp) {
        this.moIp = moIp;
    }

    /**
     * getAlarmParam
     *
     * @return 返回 alarmParam
     */
    public String[] getAlarmParam() {
        return alarmParam;
    }

    /**
     * setAlarmParam
     *
     * @param alarmParam 对alarmParam进行赋值
     */
    public void setAlarmParam(String[] alarmParam) {
        this.alarmParam = alarmParam;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getResourceId() {
        return resourceId;
    }

    public void setResourceId(String resourceId) {
        this.resourceId = resourceId;
    }
}
