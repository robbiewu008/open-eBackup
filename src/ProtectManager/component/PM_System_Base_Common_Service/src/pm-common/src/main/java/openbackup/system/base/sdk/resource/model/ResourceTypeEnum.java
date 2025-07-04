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
package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import openbackup.system.base.util.EnumUtil;

/**
 * Resource type Enum
 *
 */
public enum ResourceTypeEnum {
    // 主机
    HOST("Host"),
    // 主机文件集
    FILESET("Fileset"),
    // 表集
    TABLESET("TableSet"),
    // 无代理框架
    AGENTLESS("Agentless"),
    // DFS文件集
    DFS_FILESET("DFSFileset"),
    // 数据库
    DATABASE("Database"),
    // Application
    APPLICATION("Application"),
    // 虚拟化
    VIRTUAL_PLATFORM("VirtualPlatform"),
    // 云平台
    CLOUD_PLATFORM("CloudPlatform"),
    // 虚拟机
    VM("VM"),
    // 大数据
    BIG_DATA("BigData"),
    // agent
    PROTECT_AGENT("ProtectAgent"),
    // 副本导入
    IMPORT_COPY("ImportCopy"),
    // Nas海量小文件
    STORAGE("Storage"),
    // Nas主机
    STORAGE_EQUIPMENT("StorageEquipment"),
    // 文件系统
    FILE_SYSTEM("FileSystem"),
    // 归档存储
    ARCHIVE_STORAGE("archive_storage"),
    // 平台
    PLATFORM("Platform"),
    // 集群
    CLUSTER("Cluster"),
    // 系统
    SYSTEM("System"),
    // 磁盘
    DISK("Disk"),

    NETWORK_ADAPTER("NetworkAdapter"),

    DATASTORE("Datastore"),
    // HCS云平台
    HCS("HCS"),
    // 租户
    TENANT("Tenant"),
    // 区域
    REGION("Region"),
    // 项目
    PROJECT("Project"),
    // 云主机
    CLOUD_HOST("CloudHost"),
    // 主机池
    HOST_POOL("HostPool"),
    // CNware资源
    CNWARE("CNware"),
    // 阿里云ApsaraStack资源
    APSARA_STACK("ApsaraStack"),
    // Namespace
    NAMESPACE("Namespace"),
    // Openstack
    OPEN_STACK("OpenStack"),
    // Openstack项目
    STACK_PROJECT("StackProject"),
    // 云服务器
    CLOUD_SERVER("CloudServer"),
    // KubernetesCommon
    KUBERNETES_COMMON("KubernetesCommon"),
    // 插件
    PLUGIN("Plugin"),
    // 证书
    CERTIFICATE("Certificate"),
    // 对象存储
    OBJECT_STORAGE("ObjectStorage"),
    // Hyper-V资源
    HYPER_V("Hyper-V"),
    // Virtualization
    VIRTUALIZATION("Virtualization"),
    // Container
    CONTAINER("Container"),
    // Nutanix
    NUTANIX("Nutanix"),
    ;

    /**
     * 资源类型
     */
    private final String type;

    ResourceTypeEnum(String type) {
        this.type = type;
    }

    /**
     * get enum by value
     *
     * @param type type
     * @return enum
     */
    @JsonCreator
    public static ResourceTypeEnum get(String type) {
        return EnumUtil.get(ResourceTypeEnum.class, ResourceTypeEnum::getType, type);
    }

    /**
     * get value
     *
     * @return string
     */
    @JsonValue
    public String getType() {
        return type;
    }

    /**
     * 和字符串对比，是否相同
     *
     * @param otherType 其他类型
     * @return 是否相同
     */
    public boolean equalsType(String otherType) {
        return type.equals(otherType);
    }
}