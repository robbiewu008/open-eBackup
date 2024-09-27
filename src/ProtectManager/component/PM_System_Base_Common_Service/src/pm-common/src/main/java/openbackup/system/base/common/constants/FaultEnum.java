/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.constants;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.Locale;

/**
 * 告警枚举定义：告警级别、清除状态、确认状态、告警类型
 *
 * @author p00171530
 * @version [Lego V100R002C10, 2014-12-20]
 * @since 2019-10-28
 */
public class FaultEnum {
    /**
     * 告警确认状态-> 该字段为了配套DPA,变更为告警状态
     *
     * @author h90002262
     * @version [版本号, 2010-9-1]
     * @since 2019-10-28
     */
    public enum ConfirmState {
        /**
         * comment
         */
        ILLEGAL(-1),
        /**
         * 失败
         */
        UNCONFIRMED(0),
        /**
         * 成功
         */
        CONFIRMED(1),

        /**
         * 未恢复
         */
        UNRECOVERY(2),

        /**
         * 已恢复
         */
        RECOVERY(3),

        /**
         * 已清除
         */
        CLEAR(4),

        /**
         * 未清除，多用于告警
         */
        UNCLEAR(5);

        /**
         * comment
         */
        public static final int STATUS_ILLEGAL = -1;

        /**
         * comment
         */
        public static final int STATUS_UNCONFIRMED = 0;

        /**
         * comment
         */
        public static final int STATUS_CONFIRMED = 1;

        /**
         * comment
         */
        public static final int STATUS_UNRECOVERY = 2;

        /**
         * comment
         */
        public static final int STATUS_RECOVERY = 3;

        /**
         * comment
         */
        public static final int STATUS_CLEAR = 4;

        /**
         * comment
         */
        public static final int STATUS_UNCLEAR = 5;

        /**
         * comment
         */
        private final int value;

        ConfirmState(int value) {
            this.value = value;
        }

        /**
         * 根据传入的值获取对应的枚举值
         *
         * @param value value
         * @return ConfirmState
         */
        public static ConfirmState getState(int value) {
            for (ConfirmState state : ConfirmState.values()) {
                if (value == state.value) {
                    return state;
                }
            }

            return ConfirmState.ILLEGAL;
        }

        /**
         * 获取所有确认状态的值集合
         *
         * @return int[]
         */
        public static ConfirmState[] getAllState() {
            return ConfirmState.values();
        }

        /**
         * 获取确认状态国际化字符串
         *
         * @param value 服务
         * @return 告警级别描述
         */
        public static String getI18n(int value) {
            String i18n = "";
            switch (value) {
                case FaultEnum.ConfirmState.STATUS_ILLEGAL:
                    i18n = "lego.lab.fault.illegal.status";
                    break;
                case FaultEnum.ConfirmState.STATUS_UNCONFIRMED:
                    i18n = "lego.lab.fault.fail.status";
                    break;
                case FaultEnum.ConfirmState.STATUS_CONFIRMED:
                    i18n = "lego.lab.fault.success.status";
                    break;
                case FaultEnum.ConfirmState.STATUS_UNRECOVERY:
                    i18n = "lego.lab.fault.unrecovery.status";
                    break;
                case FaultEnum.ConfirmState.STATUS_RECOVERY:
                    i18n = "lego.lab.fault.recovery.status";
                    break;
                case FaultEnum.ConfirmState.STATUS_CLEAR:
                    i18n = "lego.lab.fault.clear.status";
                    break;
                case FaultEnum.ConfirmState.STATUS_UNCLEAR:
                    i18n = "lego.lab.fault.unclear.status";
                    break;
                default:
                    i18n = String.valueOf(value);
            }
            return i18n;
        }

        public int getValue() {
            return value;
        }
    }

    /**
     * 消息类型定义
     *
     * @author h90002262
     * @version [Lego V100R002C10, 2010-7-21]
     * @since 2010-07-21
     */
    public enum MessageTopic {
        /**
         * 命令类消息
         */
        COMMAND,

        /**
         * 配置类消息
         */
        CONFIG,

        /**
         * 数据类消息
         */
        DATA
    }

    /**
     * 告警级别定义 1--提示 2--次要 3--重要 4--紧急
     *
     * @author h90002262
     * @version [版本号, 2010-9-1]
     * @since 2019-10-28
     */
    public enum AlarmSeverity {
        /**
         * 告警级别设为5时，iEMP默认是清楚告警 s00216117添加--2013-1-6
         */
        INVALID(-1),

        /**
         * 通知
         */
        INFO(0),

        /**
         * 警告
         */
        WARNING(1),

        /**
         * 一般，由于DTS2021041706Z9FKP1H00，该告警级别已被废弃，任何继续传入此字段的告警都会被映射为重要级别
         */
        @Deprecated
        MINOR(2),

        /**
         * 重要
         */
        MAJOR(3),

        /**
         * 致命
         */
        CRITICAL(4);

        /**
         * comment
         */
        public static final int SEVERITY_INVALID = -1;

        /**
         * 建议
         */
        public static final int SEVERITY_INFO = 0;

        /**
         * comment
         */
        public static final int SEVERITY_WARNING = 1;

        /**
         * comment
         */
        public static final int SEVERITY_MINOR = 2;

        /**
         * comment
         */
        public static final int SEVERITY_MAJOR = 3;

        /**
         * comment
         */
        public static final int SEVERITY_CRITICAL = 4;

        private final int value;

        AlarmSeverity(int value) {
            this.value = value;
        }

        /**
         * 根据传入的值获取对应的枚举值
         *
         * @param value value
         * @return AlarmSeverity
         */
        @JsonCreator
        public static AlarmSeverity getSeverity(int value) {
            for (AlarmSeverity severity : AlarmSeverity.values()) {
                if (value == severity.value) {
                    return severity;
                }
            }

            return AlarmSeverity.INVALID;
        }

        /**
         * 获取所有告警级别的值集合
         *
         * @return int[]
         */
        public static AlarmSeverity[] getAllSeverity() {
            AlarmSeverity[] severities = new AlarmSeverity[LegoNumberConstant.FOUR];
            severities[LegoNumberConstant.ZERO] = AlarmSeverity.INFO;
            severities[LegoNumberConstant.ONE] = AlarmSeverity.WARNING;
            severities[LegoNumberConstant.TWO] = AlarmSeverity.MAJOR;
            severities[LegoNumberConstant.THREE] = AlarmSeverity.CRITICAL;
            return severities;
        }

        /**
         * 获取告警级别国际化字符串
         *
         * @param value 服务
         * @return 告警级别描述
         */
        public static String getI18n(int value) {
            String i18n = "";
            switch (value) {
                case SEVERITY_INFO:
                    i18n = "lego.lab.fault.alarmLevel.info";
                    break;
                case SEVERITY_WARNING:
                    i18n = "lego.lab.fault.alarmLevel.warning";
                    break;

                case SEVERITY_MINOR:
                    i18n = "lego.lab.fault.alarmLevel.minor";
                    break;

                case SEVERITY_MAJOR:
                    i18n = "lego.lab.fault.alarmLevel.major";
                    break;

                case SEVERITY_CRITICAL:
                    i18n = "lego.lab.fault.alarmLevel.critical";
                    break;

                default:
                    i18n = String.valueOf(value);
                    break;
            }
            return i18n;
        }

        @JsonValue
        public int getValue() {
            return value;
        }
    }

    /**
     * 告警清除状态
     *
     * @author h90002262
     * @version [版本号, 2010-9-1]
     * @since 2019-10-28
     */
    public enum ClearState {
        /**
         * comment
         */
        INVALID(-1),
        /**
         * 未清除的
         */
        UNCLEARED(0),
        /**
         * 清除的
         */
        CLEARED(1);

        /**
         * comment
         */
        public static final int STATUS_UNCLEARED = 0;

        /**
         * comment
         */
        public static final int STATUS_CLEARED = 1;

        private final int value;

        ClearState(int value) {
            this.value = value;
        }

        /**
         * 根据传入的值获取对应的枚举值
         *
         * @param value value
         * @return ClearState
         */
        public static ClearState getState(int value) {
            for (ClearState state : ClearState.values()) {
                if (value == state.value) {
                    return state;
                }
            }

            return ClearState.INVALID;
        }

        /**
         * 获取所有清除状态的值集合
         *
         * @return int[]
         */
        public static ClearState[] getAllState() {
            return ClearState.values();
        }

        @JsonValue
        public int getValue() {
            return value;
        }
    }

    /**
     * 告警类型定义
     *
     * @author h90002262
     * @version [版本号, 2010-9-1]
     * @since 2019-10-28
     */
    public enum AlarmType {
        /**
         * comment
         */
        INVALID(0),
        /**
         * 沟通
         */
        COMMUNICATION(1),
        /**
         * 环境
         */
        ENVIRONMENT(2),
        /**
         * 设备
         */
        DEVICE(3),
        /**
         * 业务
         */
        BUSINESS(4),
        /**
         * 操作
         */
        OPERATION(5),
        /**
         * 安全
         */
        SECURITY(6);

        /**
         * 通信告警
         */
        public static final int TYPE_COMMUNICATION = 1;

        // 环境告警

        /**
         * comment
         */
        public static final int TYPE_ENVIRONMENT = 2;

        /**
         * 设备告警
         */
        public static final int TYPE_DEVICE = 3;

        /**
         * 业务质量告警
         */
        public static final int TYPE_BUSINESS = 4;

        /**
         * 处理出错告警
         */
        public static final int TYPE_OPERATION = 5;

        /**
         * 安全告警
         */
        public static final int TYPE_SECURITY = 6;

        private final int value;

        AlarmType(int value) {
            this.value = value;
        }

        /**
         * 根据传入的值获取对应的枚举值
         *
         * @param value value value
         * @return AlarmSeverity
         */
        public static AlarmType getType(int value) {
            for (AlarmType type : AlarmType.values()) {
                if (value == type.value) {
                    return type;
                }
            }

            return AlarmType.INVALID;
        }

        /**
         * 获取所有告警级别的值集合
         *
         * @return int[]
         */
        public static AlarmType[] getAllType() {
            return AlarmType.values();
        }

        /**
         * 获取告警类型国际化字符串
         *
         * @param value 服务
         * @return 告警级别描述
         */
        public static String getI18n(int value) {
            String i18n = "";
            switch (value) {
                case TYPE_COMMUNICATION:
                    i18n = "lego.dic.warningtype.comm";
                    break;

                case TYPE_ENVIRONMENT:
                    i18n = "lego.dic.warningtype.env";
                    break;

                case TYPE_DEVICE:
                    i18n = "lego.dic.warningtype.dev";
                    break;

                case TYPE_BUSINESS:
                    i18n = "lego.dic.warningtype.business";
                    break;

                case TYPE_OPERATION:
                    i18n = "lego.dic.warningtype.operation";
                    break;

                case TYPE_SECURITY:
                    i18n = "lego.dic.warningtype.security";
                    break;

                default:
                    i18n = String.valueOf(value);
                    break;
            }
            return i18n;
        }

        public int getValue() {
            return value;
        }
    }

    /**
     * 告警Trap类型定义 -1-无效类型 0-事件 1-故障 2-恢复告警 3-操作日志 4-运行日志
     *
     * @author h90002262
     * @version [版本号, 2010-11-17]
     * @since 2019-10-28
     */
    public enum TrapType {
        /**
         * 默认
         */
        INVALID(-1),
        /**
         * 事件
         */
        EVENT(0),
        /**
         * 故障
         */
        FAULT(1),
        /**
         * 恢复告警
         */
        RESTORE(2),
        /**
         * 操作日志
         */
        OPERATION_LOG(3),
        /**
         * 运行日志
         */
        RUN_LOG(4);

        /**
         * 非法
         */
        public static final int TYPE_INVALID = -1;

        /**
         * 事件
         */
        public static final int TYPE_EVENT = 0;

        /**
         * 故障
         */
        public static final int TYPE_FAULT = 1;

        /**
         * 恢复告警
         */
        public static final int TYPE_RESTORE = 2;

        /**
         * 操作日志
         */
        public static final int TYPE_OPERATION_LOG = 3;

        /**
         * 运行日志
         */
        public static final int TYPE_RUN_LOG = 4;

        private final int value;

        TrapType(int value) {
            this.value = value;
        }

        /**
         * 根据传入的值获取对应的枚举值
         *
         * @param value value
         * @return TrapType
         */
        public static TrapType getType(int value) {
            for (TrapType trapType : TrapType.values()) {
                if (value == trapType.value) {
                    return trapType;
                }
            }

            return TrapType.INVALID;
        }

        public int getValue() {
            return value;
        }
    }

    /**
     * 北向告警Trap类型定义 -1-无效类型 1-故障 2-恢复告警 3-事件 4-确认告警 5-反确认告警
     *
     * @author h90002262
     * @version [版本号, 2010-11-17]
     * @since 2019-10-28
     */
    public enum NorthTrapType {
        /**
         * comment
         */
        INVALID(-1),
        /**
         * 故障
         */
        FAULT(1),
        /**
         * 恢复告警
         */
        RESTORE(2),
        /**
         * 事件
         */
        EVENT(3),
        /**
         * 确认告警
         */
        CONFIRM(4),
        /**
         * 反确认告警
         */
        ANTICONFIRM(5);


        private final int value;

        NorthTrapType(int value) {
            this.value = value;
        }

        /**
         * 根据传入的值获取对应的枚举值
         *
         * @param value value
         * @return TrapType
         */
        public static NorthTrapType getType(int value) {
            for (NorthTrapType trapType : NorthTrapType.values()) {
                if (value == trapType.value) {
                    return trapType;
                }
            }

            return NorthTrapType.INVALID;
        }

        public int getValue() {
            return value;
        }
    }

    /**
     * snmp north 枚举
     *
     * @author y30000858
     * @version [8.0]
     * @since 2020-07-03
     */
    public enum SnmpTrapEnum {
        /**
         * IP地址
         */
        IP("NORTHIP"),
        /**
         * 端口
         */
        PORT("PORT");

        private final String value;

        SnmpTrapEnum(String value) {
            this.value = value;
        }

        public String getValue() {
            return value;
        }
    }

    /**
     * 告警排序枚举
     *
     * @author y30000858
     * @version [8.0]
     * @since 2020-07-03
     */
    public enum OrderBy {
        /**
         * 安全
         */
        SEVERITY("SEVERITY"),
        /**
         * 来源
         */
        SOURCE_TYPE("SOURCE_TYPE"),
        /**
         * 告警类型
         */
        ALARM_TYPE("ALARM_TYPE"),
        /**
         * 告警ID
         */
        ALARM_ID("ALARM_ID"),
        /**
         * 确认时间
         */
        MODIFY_TIME("MOD_TIME"),
        /**
         * 创建时间
         */
        CREATE_TIME("CREATE_TIME"),

        /**
         * 开始时间
         */
        START_TIME("START_TIME");

        private final String value;

        OrderBy(String value) {
            this.value = value;
        }

        /**
         * get order by field
         *
         * @param value value
         * @return value
         */
        public static OrderBy get(String value) {
            return EnumUtil.get(OrderBy.class, OrderBy::getValue, value.toUpperCase(Locale.ENGLISH));
        }

        @JsonValue
        public String getValue() {
            return value;
        }
    }

    /**
     * 分页排序枚举
     *
     * @author y30000858
     * @version [8.0]
     * @since 2020-07-03
     */
    public enum OrderType {
        /**
         * 正序
         */
        ASC("ASC"),
        /**
         * 逆序
         */
        DESC("DESC");

        private final String value;

        OrderType(String value) {
            this.value = value;
        }

        /**
         * get order type
         *
         * @param value value
         * @return value
         */
        public static OrderType get(String value) {
            return EnumUtil.get(OrderType.class, OrderType::getValue, value);
        }

        @JsonValue
        public String getValue() {
            return value;
        }
    }

    /**
     * 告警来源类型
     *
     * @author y30000858
     * @version [8.0]
     * @since 2020-07-03
     */
    @Getter
    @AllArgsConstructor
    public enum AlarmResourceType {
        /**
         * 用户
         */
        USER("user"),

        /**
         * 三方告警
         */
        ALARM("alarm"),

        /**
         * 事件
         */
        EVENT("event"),

        /**
         * 提醒
         */
        NOTIFY("notify"),

        /**
         * 存储库
         */
        RESOURCE("resource"),

        /**
         * 保护
         */
        PROTECTION("protection"),

        /**
         * 恢复
         */
        RECOVERY("recovery"),

        /**
         * 备份集群
         */
        BACKUP_CLUSTER("backupCluster"),

        /**
         * 网元实体
         */
        NETWORK_ENTITY("networkEntity"),

        /**
         * 证书
         */
        CERTIFICATE("certificate"),

        /**
         * 集群
         */
        CLUSTER("cluster"),

        /**
         * License
         */
        LICENSE("license"),

        /**
         * 存储库
         */
        REPOSITORY("repository"),

        /**
         * 秘钥管理
         */
        KMS("kms"),

        /**
         * SLA
         */
        SLA("sla"),

        /**
         * 受保护对象
         */
        PROTECTED_OBJECT("protectedObject"),

        /**
         * 调度器
         */
        SCHEDULER("scheduler"),

        /**
         * 即时挂载
         */
        LIVE_MOUNT("liveMount"),

        /**
         * 恢复
         */
        RESTORE("restore"),

        /**
         * 副本目录
         */
        COPY_CATALOG("copyCatalog"),

        /**
         * 复制
         */
        REPLICATION("replication"),

        /**
         * 本地存储
         */
        LOCAL_STORAGE("localStorage"),

        /**
         * 勒索软件检测
         */
        ANTI_DETECTION("detection"),

        /**
         * 任务
         */
        JOB("JOB");

        @JsonValue
        private String value;

        /**
         * 通过value获取AlarmResourceType
         *
         * @param value 值
         * @return 目标告警对象
         */
        @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
        public static AlarmResourceType getAlarmResourceType(String value) {
            if (value == null) {
                return ALARM;
            }
            for (AlarmResourceType alarmResourceType : values()) {
                if (value.equals(alarmResourceType.getValue())) {
                    return alarmResourceType;
                }
            }
            return ALARM;
        }
    }

    /**
     * 告警MoName类型
     *
     * @author y30046482
     * @version [OceanProtect DataBackup 1.6.0]
     * @since 2024-07-17
     */
    @Getter
    @AllArgsConstructor
    public enum MoNameType {
        CLUSTER_MANAGER("ClusterManager");

        private final String value;
    }
}
