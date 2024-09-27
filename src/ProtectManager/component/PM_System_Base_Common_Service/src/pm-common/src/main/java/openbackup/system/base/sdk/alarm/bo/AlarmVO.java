/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package openbackup.system.base.sdk.alarm.bo;

import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.sdk.alarm.i18n.I18nMrgUtil;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.Data;

import org.apache.commons.lang3.StringUtils;

import java.util.Arrays;
import java.util.Locale;
import java.util.stream.Collectors;

/**
 * 告警前端对象
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-01-01
 */
@Data
public class AlarmVO {
    private static final long serialVersionUID = -6711903711556380609L;

    private static final long ASC_ID = -1L;

    private static final String[] EMPTY_ARR = new String[0];

    // 远程通知规则里的告警ID，数据库自增ID
    @JsonIgnore
    private long id = ASC_ID;

    // 对应WebNms Alert表的主键entity
    private String entityId;

    private Long sequence;

    // 告警ID
    private String alarmId;

    // 告警级别
    private FaultEnum.AlarmSeverity severity;

    // 发生时间
    @JsonIgnore
    private long firstTime;

    // 第一次发生时间字符串，格式为：yyyy-MM-dd HH:mm:ss
    private String firstTimeStr;

    // 发生时间
    @JsonIgnore
    private long alarmTime;

    // 发生时间字符串，格式为：yyyy-MM-dd HH:mm:ss
    private String alarmTimeStr;

    // 告警名称
    private String alarmName;

    // 告警名称的Key
    @JsonIgnore
    private String alarmNameKey;

    // 告警类型
    private FaultEnum.AlarmType alarmType;

    // 对应ManagedObject表的name属性
    private String alarmSource;

    // 网元显示名称
    @JsonIgnore
    private String moName;

    // 网元id
    @JsonIgnore
    private String moId;

    // 网元主类型
    @JsonIgnore
    private String moMainType;

    // 网元子类型
    @JsonIgnore
    private String moSubType;

    // 网元类型字符,网元主类型和网元子类型的组合，用于界面显示
    @JsonIgnore
    private String moType;

    // 对业务的影响
    private String effect;

    // 告警描述
    private String desc;

    // 修复建议
    private String advice;

    // 清除状态
    private FaultEnum.ClearState clearStatus;

    // 清除时间
    @JsonIgnore
    private long clearTime;

    // 清除用户名
    @JsonIgnore
    private String clearUser;

    // 操作日志状态
    private int confirmStatus;

    // 确认时间
    @JsonIgnore
    private long confirmTime;

    // 确认用户名
    @JsonIgnore
    private String confirmUser;

    // 告警次数
    @JsonIgnore
    private int count;

    // 复杂查询的name值
    @JsonIgnore
    private String alarmComplexName;

    // 起始时间
    @JsonIgnore
    private String createTimeStart;

    // 截至时间
    @JsonIgnore
    private String createTimeEnd;

    // 选中导出的流式号字符串
    @JsonIgnore
    private String exportSelectIds;

    // 排序的字段名
    @JsonIgnore
    private String orderByField;

    // 是否按照升序排列
    @JsonIgnore
    private boolean isOrderByAscend;

    // 是否是精确查找
    @JsonIgnore
    private boolean isExactSearch;

    // 确认时间串
    @JsonIgnore
    private String confirmTimeStr;

    // 清除时间串;
    private String clearTimeStr;

    // 定位信息
    private String location;

    // 分级网管IP
    @JsonIgnore
    private String nmsIp;

    // 国际化对应的动态参数，涉及到告警名称、描述、advice、location
    private String[] params;

    private String sourceType;

    private int type;

    private String deviceName;

    /**
     * 告警发生节点esn
     */
    private String esn;

    /**
     * 告警发生的节点名称
     */
    private String nodeName;

    /**
     * 取英文的国际化(带换行符)
     *
     * @param local 语言，如果传入空默认英文，规则：英文传入Locale.ENGLISH，中文传入Locale.CHINA
     */
    public void updateInternalStatementWithNewLine(Locale local) {
        // 告警名称
        this.alarmName = getInternalStatement(this.alarmName, local, null);

        // 描述
        this.desc = getInternalStatement(this.desc, local, this.params);

        // 建议
        this.advice = getInternalStatementWithNewLine(this.advice, local, this.params);

        // 影响
        this.effect = getInternalStatementWithNewLine(this.effect, local, null);
    }

    private String getInternalStatementWithNewLine(String key, Locale local, String[] param) {
        if (StringUtils.isEmpty(key) || local == null) {
            return "";
        }
        if (param == null || param.length == 0) {
            return I18nMrgUtil.getInstance()
                .getI18nMgr()
                .getString(key, local)
                .replaceAll("<br>", System.lineSeparator());
        }
        String[] paramArr = Arrays.stream(param).map(String::trim).collect(Collectors.toList()).toArray(EMPTY_ARR);
        return I18nMrgUtil.getInstance()
            .getI18nMgr()
            .getString(key, local, paramArr)
            .replaceAll("<br>", System.lineSeparator());
    }

    private String getInternalStatement(String key, Locale local, String[] param) {
        if (StringUtils.isEmpty(key) || local == null) {
            return "";
        }
        if (param == null || param.length == 0) {
            return I18nMrgUtil.getInstance().getI18nMgr().getString(key, local).replaceAll("<br>", "");
        }
        String[] paramArr = Arrays.stream(param).map(String::trim).collect(Collectors.toList()).toArray(EMPTY_ARR);
        return I18nMrgUtil.getInstance().getI18nMgr().getString(key, local, paramArr).replaceAll("<br>", "");
    }
}
