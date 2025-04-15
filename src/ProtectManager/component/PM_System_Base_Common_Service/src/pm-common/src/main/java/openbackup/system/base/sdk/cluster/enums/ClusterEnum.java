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
package openbackup.system.base.sdk.cluster.enums;

import lombok.Getter;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Cluster enum class
 *
 */
public class ClusterEnum {
    /**
     * enum type
     *
     */
    public enum TypeEnum {
        /**
         * obtaining all clusters
         */
        ALL(0),
        /**
         * Obtaining All Clusters
         */
        LOCAL(1),
        /**
         * Obtaining the Target Cluster
         */
        REMOTE(2);

        private final int type;

        TypeEnum(int type) {
            this.type = type;
        }

        public int getType() {
            return type;
        }
    }

    /**
     * enum status
     *
     */
    public enum StatusEnum {
        /**
         * 未知
         */
        UNKNOWN(0),
        /**
         * normal
         */
        NORMAL(1),
        /**
         * running
         */
        RUNNING(2),

        /**
         * 备份存储单元升级中
         */
        UPGRADING(25),
        /**
         * 初始化，用于备份成员节点创建
         */
        SETTING(26),
        /**
         * online
         */
        ONLINE(27),
        /**
         * offline
         */
        OFFLINE(28),
        /**
         * 删除中
         */
        DELETING(29),

        /**
         * Partially offline
         */
        PARTIALLY_ABNORMAL(30);

        /**
         * 状态排序规则
         */
        public static final List<Integer> STATUS_ORDER_LIST = Collections.unmodifiableList(
            Arrays.asList(ONLINE.status, OFFLINE.status, SETTING.status, UNKNOWN.status, NORMAL.status, RUNNING.status,
                DELETING.status, PARTIALLY_ABNORMAL.status));

        private final int status;

        StatusEnum(int status) {
            this.status = status;
        }

        /**
         * 是否在线
         *
         * @param status 状态
         * @return true-在线，false-离线
         */
        public static boolean isOnline(int status) {
            return status == ONLINE.status;
        }

        public int getStatus() {
            return status;
        }

        /**
         * 是否是异常状态
         *
         * @param status 状态值
         * @return 是否是异常状态(OFFLINE, PARTIALLY_ABNORMAL)
         */
        public static boolean isAbnormalStatus(int status) {
            return status == OFFLINE.status || status == PARTIALLY_ABNORMAL.status;
        }
    }

    /**
     * enum type
     *
     */
    public enum ModifyType {
        /**
         * modify name only
         */
        NAME(0),
        /**
         * modify auth param only, like username or password
         */
        AUTH(1),

        /**
         * modify ip or port
         */
        DIRECT(2),

        /**
         * not modified
         */
        NOTMODIFY(3);

        private final int type;

        ModifyType(int type) {
            this.type = type;
        }

        public int getType() {
            return type;
        }
    }

    /**
     * dorado eth_port type
     *
     */
    public enum DoradoSelectTypeEnum {
        /**
         * host port/service port
         */
        HOST("0"),
        /**
         * management port
         */
        MANAGEMENT("2"),
        /**
         * maintenance port
         */
        MAINTENANCE("4");

        private final String type;

        DoradoSelectTypeEnum(String type) {
            this.type = type;
        }

        public String getType() {
            return type;
        }
    }

    /**
     * enum flag
     *
     */
    public enum VerifyFlag {
        /**
         * auth valid
         */
        AUTH_VALID("T"),
        /**
         * auth invalid
         */
        AUTH_INVALID("F");

        private final String flag;

        VerifyFlag(String flag) {
            this.flag = flag;
        }

        public String getFlag() {
            return flag;
        }
    }

    /**
     * Enum operate type
     *
     */
    public enum OperateType {
        /**
         * add cluster source and target relation
         */
        ADD_RELATION("A"),
        /**
         * delete cluster source and target relation
         */
        DELETE_RELATION("D"),
        /**
         * modify
         */
        MODIFY_RELATION("M");

        private final String operate;

        OperateType(String operate) {
            this.operate = operate;
        }

        public String getOperate() {
            return operate;
        }
    }

    /**
     * Enum relation status
     *
     */
    public enum EnableRelation {
        /**
         * enable
         */
        ENABLE(1),
        /**
         * disable
         */
        DISABLE(0);

        private final int relation;

        EnableRelation(int relation) {
            this.relation = relation;
        }

        public int getRelation() {
            return relation;
        }
    }

    /**
     * Enum request type
     *
     */
    public enum RequestType {
        /**
         * Internal interface
         */
        INTERNAL("internal"),

        /**
         * External Interfaces
         */
        EXTERNAL("external");

        private final String type;

        RequestType(String type) {
            this.type = type;
        }

        public String getType() {
            return type;
        }
    }

    /**
     * Enum create user type
     *
     */
    public enum DoradoCreateUserType {
        // create device admin
        DEVICE_ADMIN_TYPE(0),

        // create user defined type
        USER_DEFINED_TYPE(1);

        private final int userType;

        DoradoCreateUserType(int userType) {
            this.userType = userType;
        }

        public int getUserType() {
            return userType;
        }
    }

    /**
     * Enum target cluster role type
     *
     */
    public enum RoleType {
        /**
         * 1.2.1版本复制去除管理面依赖后，复制目标集群角色（使用数据面业务IP）
         */
        REPLICATION(1),

        /**
         * 管理集群
         */
        MANAGEMENT(2),

        /**
         * 备份集群
         */
        BACKUP(3),

        /**
         * 备份成员节点
         */
        BACKUP_MEMBER(5),

        /**
         * HA成员节点
         */
        HA_MEMBER(6),

        /**
         * 主节点
         */
        BACKUP_PRIMARY(7),

        /**
         * 内部使用
         */
        HCS(8);

        /**
         * 节点排序规则
         */
        public static final List<Integer> ROLE_TYPE_ORDER_LIST = Collections.unmodifiableList(
            Arrays.asList(BACKUP_PRIMARY.roleType, HA_MEMBER.roleType, BACKUP_MEMBER.roleType));

        private final int roleType;

        RoleType(int roleType) {
            this.roleType = roleType;
        }

        /**
         * 是否是查询复制目标集群列表场景
         *
         * @param roleList 集群类型
         * @return 是否是查询复制目标集群列表场景(REP, REPLICATION)
         */
        public static boolean isReplicationCluster(List<Integer> roleList) {
            return CollectionUtils.isNotEmpty(roleList) && roleList.contains(REPLICATION.roleType);
        }

        /**
         * 是否是多域集群列表场景
         *
         * @param roleList 集群类型
         * @return 是否是多域集群列表场景(MANAGEMENT)
         */
        public static boolean isManagementCluster(List<Integer> roleList) {
            return CollectionUtils.isNotEmpty(roleList) && roleList.contains(MANAGEMENT.roleType);
        }

        /**
         * 是否是查询备份集群场景
         *
         * @param roleList 集群类型
         * @return 是否是查询备份集群场景(BACKUP)
         */
        public static boolean isBackupCluster(List<Integer> roleList) {
            return CollectionUtils.isNotEmpty(roleList) && roleList.contains(BACKUP.roleType);
        }

        /**
         * 是否是查询备份集群列表场景
         *
         * @param roleList 集群类型
         * @return 是否是查询备份集群列表场景(BACKUP_MEMBER, HA_MEMBER, BACKUP_PRIMARY)
         */
        public static boolean isMemberCluster(List<Integer> roleList) {
            return CollectionUtils.isNotEmpty(roleList) && (roleList.contains(BACKUP_MEMBER.roleType)
                || roleList.contains(HA_MEMBER.roleType) || roleList.contains(BACKUP_PRIMARY.roleType));
        }

        public int getRoleType() {
            return roleType;
        }
    }

    /**
     * 集群端口枚举类
     *
     */
    public enum PortType {
        /**
         * gui通信端口
         */
        GUI_PORT(25080),
        /**
         * 管理IP端口
         */
        MANAGEMENT(25081),

        /**
         * 备份IP端口
         */
        BACKUP(30068);

        private final int portType;

        PortType(int portType) {
            this.portType = portType;
        }

        public int getPortType() {
            return portType;
        }
    }

    /**
     * 复制集群类型枚举类
     *
     */
    public enum ClusterType {
        /**
         * gui通信端口
         */
        OCEAN_PROTECT_X(0),
        /**
         * 管理IP端口
         */
        HCS(1);


        private final int clusterType;

        ClusterType(int clusterType) {
            this.clusterType = clusterType;
        }

        public int getClusterType() {
            return clusterType;
        }
    }

    /**
     * 备份节点角色
     */
    public enum BackupRoleTypeEnum {
        /**
         * 主节点
         */
        PRIMARY("PRIMARY", RoleType.BACKUP_PRIMARY.roleType),

        /**
         * 成员节点
         */
        MEMBER("MEMBER", RoleType.BACKUP_MEMBER.roleType),

        /**
         * HA成员
         */
        STANDBY("STANDBY", RoleType.HA_MEMBER.roleType);

        /**
         * 排序list
         */
        public static final List<String> BACKUP_ROLE_ORDER_LIST = Collections.unmodifiableList(
            Arrays.asList(PRIMARY.getBackupRoleType(), STANDBY.getBackupRoleType(), MEMBER.getBackupRoleType()));

        private final String backupRoleType;

        private final Integer roleType;

        BackupRoleTypeEnum(String backupRoleType, Integer roleType) {
            this.backupRoleType = backupRoleType;
            this.roleType = roleType;
        }

        /**
         * 根据集群角色查询备份节点角色
         *
         * @param roleType 集群角色
         * @return 节点角色
         */
        public static String getBackupRoleTypeByRoleType(Integer roleType) {
            for (BackupRoleTypeEnum type : BackupRoleTypeEnum.values()) {
                if (type.getRoleType().equals(roleType)) {
                    return type.getBackupRoleType();
                }
            }
            return StringUtils.EMPTY;
        }

        /**
         * 根据集群角色查询备份节点角色
         *
         * @param backupRoleType 集群角色
         * @return 节点角色
         */
        public static String getBackupRoleTypeByRoleType(String backupRoleType) {
            for (BackupRoleTypeEnum type : BackupRoleTypeEnum.values()) {
                if (type.getBackupRoleType().equals(backupRoleType)) {
                    return type.getBackupRoleType();
                }
            }
            return StringUtils.EMPTY;
        }

        /**
         * 根据集群角色查询备份节点角色
         *
         * @param backupRoleType 集群角色
         * @return 节点角色
         */
        public static Integer getRoleTypeByBackupRoleType(String backupRoleType) {
            for (BackupRoleTypeEnum type : BackupRoleTypeEnum.values()) {
                if (type.getBackupRoleType().equals(backupRoleType)) {
                    return type.getRoleType();
                }
            }
            return 0;
        }

        public String getBackupRoleType() {
            return backupRoleType;
        }

        public Integer getRoleType() {
            return roleType;
        }
    }

    /**
     * HA操作类型
     */
    public enum HaOperationTypeEnum {
        /**
         * 添加HA成团员
         */
        ADD("add"),

        /**
         * 修改HA配置
         */
        MODIFY("modify"),

        /**
         * 移除HA配置
         */
        REMOVE("remove"),

        /**
         * 后置任务
         */
        POST("post");

        private final String type;

        HaOperationTypeEnum(String type) {
            this.type = type;
        }

        /**
         * 查询类型
         *
         * @return 类型
         */
        public String getType() {
            return type;
        }
    }

    /**
     * 存储策略枚举类
     */
    public enum StorageStrategyEnum {
        /**
         * 默认策略(多集群场景)
         */
        DEFAULT(0, "default"),

        /**
         * 缩减率优先（多集群场景）
         */
        REDUCTION_RATE_PRIORITY(1, "ReductionRatePriority"),
        /**
         * 智能均衡(多集群场景)
         */
        INTELLIGENT_EQUALIZATION(2, "IntelligentEqualization");

        private final Integer type;

        private final String name;

        StorageStrategyEnum(Integer type, String name) {
            this.type = type;
            this.name = name;
        }

        /**
         * 判断是否属于此枚举类
         *
         * @param type type
         * @return boolean 是否属于此枚举类
         */
        public static boolean contains(Integer type) {
            for (StorageStrategyEnum storageStrategyEnum : StorageStrategyEnum.values()) {
                if (storageStrategyEnum.type.equals(type)) {
                    return true;
                }
            }
            return false;
        }

        /**
         * 根据type获取枚举对象
         *
         * @param type type
         * @return StorageStrategyEnum
         */
        public static StorageStrategyEnum getStorageStrategyEnumByType(Integer type) {
            for (StorageStrategyEnum strategyEnum : StorageStrategyEnum.values()) {
                if (strategyEnum.type.equals(type)) {
                    return strategyEnum;
                }
            }
            // 未找到时，使用默认策略
            return DEFAULT;
        }

        /**
         * getType
         *
         * @return type
         */
        public Integer getType() {
            return this.type;
        }
    }

    /**
     * 备份存储单元添加方式枚举类
     */
    public enum GeneratedTypeEnum {
        MANUAL(1, "Manual"),
        AUTO(2, "Auto");

        private final Integer type;

        private final String name;

        GeneratedTypeEnum(Integer type, String name) {
            this.type = type;
            this.name = name;
        }

        public Integer getType() {
            return this.type;
        }
    }

    /**
     * 存储机器状态枚举
     */
    @Getter
    public enum StorageStatusEnum {
        /**
         * 正常运行的机器状态
         */
        STATUS_RUNNING("10");

        private final String status;

        StorageStatusEnum(String status) {
            this.status = status;
        }
    }
}