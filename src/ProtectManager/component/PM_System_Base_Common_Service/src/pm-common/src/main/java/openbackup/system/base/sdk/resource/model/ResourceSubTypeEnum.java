package openbackup.system.base.sdk.resource.model;

import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.extern.slf4j.Slf4j;

import org.apache.logging.log4j.util.Strings;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Resource type Enum
 *
 * @author l00557046
 * @since 2020-10-14
 */
@Slf4j
public enum ResourceSubTypeEnum {
    DB_BACKUP_AGENT(1, "DBBackupAgent", "AGENT"),
    VM_BACKUP_AGENT(2, "VMBackupAgent", "AGENT"),
    AB_BACKUP_AGENT(3, "ABBackupClient", "AGENT"),
    U_BACKUP_AGENT(4, "UBackupAgent", "AGENT"),
    DWS_BACKUP_AGENT(5, "DWSBackupAgent", "AGENT"),
    PROTECT_AGENT(6, "ProtectAgent", "AGENT"),
    FILESET(7, "Fileset", "Fileset"),
    DFS_FILESET(8, "DFSFileset", "Fileset"),
    ORACLE(9, "Oracle", "Oracle"),
    SQL_SERVER(10, "SQLServer", "SQLServer"),
    SQL_SERVER_INSTANCE(11, "SQLServer-instance", "SQLServer"),
    SQL_SERVER_DATABASE(12, "SQLServer-database", "SQLServer"),
    SQL_SERVER_CLUSTER(13, "SQLServer-cluster", "SQLServer"),
    SQL_SERVER_ALWAYS_ON(14, "SQLServer-alwaysOn", "SQLServer"),
    SQL_SERVER_CLUSTER_INSTANCE(15, "SQLServer-clusterInstance", "SQLServer"),
    DB2(16, "DB2", "DB2"),
    MYSQL(17, "MySQL", "MySQL"),
    MYSQL_SINGLE_INSTANCE(18, "MySQL-instance", "MySQL"),
    MYSQL_CLUSTER(19, "MySQL-cluster", "MySQL"),
    MYSQL_CLUSTER_INSTANCE(20, "MySQL-clusterInstance", "MySQL"),
    MYSQL_DATABASE(21, "MySQL-database", "MySQL"),
    REDIS(22, "Redis", "Redis"),
    CLICK_HOUSE(23, "ClickHouse", "ClickHouse"),
    GAUSSDB(24, "GaussDB", "GaussDB"),
    GAUSSDBT(25, "GaussDBT", "GaussDBT"),
    GAUSSDB_DWS(26, "DWS-cluster", "GaussDB_DWS"),
    GAUSSDB_DWS_DATABASE(27, "DWS-database", "GaussDB_DWS"),
    GAUSSDB_DWS_SCHEMA(28, "DWS-schema", "GaussDB_DWS"),
    GAUSSDB_DWS_TABLE(29, "DWS-table", "GaussDB_DWS"),
    OPENGAUSS(30, "OpenGauss", "OpenGauss"),
    OPENGAUSS_INSTANCE(31, "OpenGauss-instance", "OpenGauss"),
    OPENGAUSS_DATABASE(32, "OpenGauss-database", "OpenGauss"),
    SAP_HANA(33, "SAP HANA", "SAP_HANA"),
    KING_BASE(34, "KingBase", "KingBase"),
    KING_BASE_CLUSTER(35, "KingBaseCluster", "KingBase"),
    KING_BASE_INSTANCE(36, "KingBaseInstance", "KingBase"),
    KING_BASE_CLUSTER_INSTANCE(37, "KingBaseClusterInstance", "KingBase"),
    SYBASE_IQ(38, "Sybase IQ", "Sybase_IQ"),
    INFORMIX(39, "Informix", "Informix"),
    TIMES_TEN(40, "TimesTen", "TimesTen"),
    GBASE(41, "Gbase", "Gbase"),
    DAMENG(42, "Dameng", "Dameng"),
    DAMENG_SINGLE_NODE(43, "Dameng-singleNode", "Dameng"),
    DAMENG_CLUSTER(44, "Dameng-cluster", "Dameng"),
    CASSANDRA(45, "Cassandra", "Cassandra"),
    OSCARDB(46, "OscarDB", "OscarDB"),
    EXCAHANGE(47, "Exchange", "Exchange"),
    VCENTER(48, "VMware vCenter Server", "VMware"),
    ESX(49, "VMware ESX", "VMware"),
    ESXI(50, "VMware ESXi", "VMware"),
    HYPER_V(51, "Hyper-V", "HyperV"),
    MICROSOFT_VIRTUAL_MACHINE(52, "ms.VirtualMachine", "HyperV"),
    MICROSOFT_HOST_SYSTEM(53, "ms.HostSystem", "HyperV"),
    FUSION_SPHERE(54, "FusionSphere", "VMware"),
    CLUSTER_COMPUTE_RESOURCE(55, "vim.ClusterComputeResource", "VMWare"),
    HOST_SYSTEM(56, "vim.HostSystem", "VMware"),
    FOLDER(57, "vim.Folder", "VMware"),
    RESOURCE_POOL(58, "vim.ResourcePool", "VMware"),
    OPEN_STACK(59, "OpenStack", "OpenStack"),
    VMWARE(60, "vim.VirtualMachine", "VMware"),
    HADOOP(61, "Hadoop", "Hadoop"),
    VIRTUAL_APP(62, "vim.VirtualApp", "VMware"),
    FUSION_INSIGHT(63, "FusionInsight", "FusionInsight"),
    HDFS(64, "HDFS", "HDFS"),
    HDFS_FILESET(65, "HDFSFileset", "HDFS"),
    HBASE(66, "HBase", "HBase"),
    HBASE_BACKUPSET(67, "HBaseBackupSet", "HBase"),
    HIVE(68, "Hive", "Hive"),
    HIVE_BACKUPSET(69, "HiveBackupSet", "Hive"),
    ELASTICSEARCH(70, "ElasticSearch", "ElasticSearch"),
    ELASTICSEARCH_BACKUPSET(71, "ElasticSearchBackupSet", "ElasticSearch"),
    POSTGRE_CLUSTER(72, "PostgreCluster", "PostgreSQL"),
    POSTGRE_INSTANCE(73, "PostgreInstance", "PostgreSQL"),
    POSTGRESQL(74, "PostgreSQL", "PostgreSQL"),
    POSTGRE_CLUSTER_INSTANCE(75, "PostgreClusterInstance", "PostgreSQL"),

    // 副本导入
    IMPORT_COPY(76, "ImportCopy", "ImportCopy"),
    REPLICA(77, "Replica", "Replica"),
    NAS_SHARE(78, "NasShare", "NasShare"),
    NAS_FILESYSTEM(79, "NasFileSystem", "NasFileSystem"),
    CLOUD_BACKUP_FILE_SYSTEM(80, "CloudBackupFileSystem", "CloudBackupFileSystem"),
    S3_STORAGE(81, "S3.storage", "S3.storage"),
    KUBERNETES(82, "Kubernetes", "Kubernetes_FlexVolume"),
    KUBERNETES_NAMESPACE(83, "KubernetesNamespace", "Kubernetes_FlexVolume"),
    KUBERNETES_STATEFUL_SET(84, "KubernetesStatefulSet", "Kubernetes_FlexVolume"),
    FUSION_COMPUTE(85, "FusionCompute", "FusionCompute"),
    COMMON(86, "Common", "Common"),
    HUAWEI_CLOUD_STACK(87, "HuaweiCloudStack", "HCSStack"),
    HCS_CONTAINER(88, "HCSContainer", "HCSStack"),
    HCS_TENANT(89, "HCSTenant", "HCSStack"),
    HCS_REGION(90, "HCSRegion", "HCSStack"),
    HCS_PROJECT(91, "HCSProject", "HCSStack"),
    HCS_CLOUD_HOST(92, "HCSCloudHost", "HCSStack"),
    OCEAN_STOR_V6(93, "OceanStorV6", "StorageEquipment"),
    DORADO_V6(94, "DoradoV6", "StorageEquipment"),
    OCEAN_STOR_V5(95, "OceanStorV5", "StorageEquipment"),
    OCEAN_STOR_PACIFIC(96, "OceanStorPacific", "StorageEquipment"),
    NET_APP(97, "NetApp", "StorageEquipment"),
    STORAGE_OTHERS(98, "StorageOthers", "StorageEquipment"),
    CLOUD_BACKUP(99, "OceanStorDoradoV6", "OceanStorDoradoV6"),
    FUSION_COMPUTE_HOST(100, "FusionComputeHost", "FusionCompute"),
    FUSION_COMPUTE_CLUSTER(101, "FusionComputeCluster", "FusionCompute"),
    DB2_TABLESPACE(102, "DB2-tablespace", "DB2"),
    GENERAL_DB(103, "GeneralDb", "GeneralDb"),
    OPENSTACK_CONTAINER(104, "OpenStackContainer", "OpenStack"),
    OPENSTACK_PROJECT(105, "OpenStackProject", "OpenStack"),
    OPENSTACK_CLOUD_SERVER(106, "OpenStackCloudServer", "OpenStack"),
    CYBERENGINE_DORADO_V6(107, "CyberEngineDoradoV6", "CyberEngineDoradoV6"),
    CYBERENGINE_OCEAN_PROTECT(108, "CyberEngineOceanProtect", "CyberEngineOceanProtect"),
    GOLDENDB_CLUSETER_INSTANCE(109, "GoldenDB-clusterInstance", "GoldenDB"),
    GOLDENDB_CLUSTER(110, "GoldenDB-cluster", "GoldenDB"),
    GOLDENDB(111, "GoldenDB", "GoldenDB"),
    DB2_CLUSTER(112, "DB2-cluster", "DB2"),
    DB2_INSTANCE(113, "DB2-instance", "DB2"),
    DB2_CLUSTER_INSTANCE(114, "DB2-clusterInstance", "DB2"),
    DB2_DATABASE(115, "DB2-database", "DB2"),
    ORACLE_CLUSTER(116, "Oracle-cluster", "Oracle"),
    ORACLE_CLUSTER_INSTANCE(117, "Oracle-clusterInstance", "Oracle"),
    ORACLE_CLUSTER_ENV(118, "Oracle-clusterEnv", "Oracle"),
    OPENSTACK_DOMAIN(119, "OpenStackDomain", "OpenStack"),
    HCS_GAUSSDB_PROJECT(120, "HCSGaussDBProject", "HCSGaussDB"),
    HCS_GAUSSDB_INSTANCE(121, "HCSGaussDBInstance", "HCSGaussDB"),
    MONGODB(122, "MongoDB", "MongoDB"),
    MONGODB_SINGLE(123, "MongoDB-single", "MongoDB"),
    MONGODB_CLUSTER(124, "MongoDB-cluster", "MongoDB"),
    TPOPS_GAUSSDB_PROJECT(125, "TPOPSGaussDBProject", "GaussDB"),
    TPOPS_GAUSSDB_INSTANCE(126, "TPOPSGaussDBInstance", "GaussDB"),
    EXCHANGE_GROUP(127, "Exchange-group", "Exchange"),
    EXCHANGE_DATABASE(128, "Exchange-database", "Exchange"),
    NDMP(129, "NDMP", "NasFileSystem"),
    NDMP_BACKUPSET(130, "NDMP-BackupSet", "NasFileSystem"),
    NDMP_SERVER(131, "NDMP-server", "NasFileSystem"),
    SAPHANA_INSTANCE(132, "SAPHANA-instance", "SAP_HANA"),
    SAPHANA_DATABASE(133, "SAPHANA-database", "SAP_HANA"),
    REDHAT_VIRTUALIZATION(134, "RedHatVirtualization", "REDHAT"),
    REDHAT_DATACENTER(135, "RHVDatacenter", "REDHAT"),
    REDHAT_CLUSTER(136, "RHVCluster", "REDHAT"),
    REDHAT_HOST(137, "RHVHost", "REDHAT"),
    REDHAT_VM(138, "RHVVM", "REDHAT"),
    REDHAT_DISK(139, "RHVDisk", "REDHAT"),
    /**
     * 备份集群
     */
    BACKUP_MEMBER_CLUSTER(140, "BackMemberCluster", "BackMemberCluster"),
    INFORMIX_SINGLE_INSTANCE(141, "Informix-singleInstance", "Informix"),
    INFORMIX_CLUSTER_INSTANCE(142, "Informix-clusterInstance", "Informix"),
    INFORMIX_SERVICE(143, "Informix-service", "Informix"),
    HYPER_V_CLUSTER(144, "HyperV.Cluster", "HyperV"),
    HYPER_V_HOST(145, "HyperV.Host", "HyperV"),
    HYPER_V_VM(146, "HyperV.VM", "HyperV"),
    HYPER_V_DISK(147, "HyperV.Disk", "HyperV"),
    HYPER_V_SCVMM(148, "HyperV.SCVMM", "HyperV"),

    /**
     * SanClient代理主机subType
     */
    S_BACKUP_AGENT(149, "SBackupAgent", "AGENT"),
    TDSQL_CLUSTERINSTANCE(150, "TDSQL-clusterInstance", "TDSQL"),
    TDSQL_CLUSTER(151, "TDSQL-cluster", "TDSQL"),
    OCEAN_BASE_CLUSTER(152, "OceanBase-cluster", "OceanBase"),
    OCEAN_BASE_TENANT(153, "OceanBase-tenant", "OceanBase"),
    /**
     * TiDB-clusterInstance
     */
    TIDB_CLUSTERINSTANCE(154, "TiDB-clusterInstance", "TiDB"),

    /**
     * TiDB-cluster
     */
    TIDB_CLUSTER(155, "TiDB-cluster", "TiDB"),
    /**
     * TiDB-table
     */
    TIDB_TABLE(156, "TiDB-table", "TiDB"),

    /**
     * TiDB-数据库类型
     */
    TIDB_DATABASE(157, "TiDB-database", "TiDB"),

    /**
     * tidb
     */
    TIDB(158, "TiDB", "TiDB"),

    /**
     * GAUSSDBT_SINGLE
     */
    GAUSSDBT_SINGLE(159, "GaussDBT-single", "GaussDBT"),

    /**
     * k8s集群，1.5.0新增CSI备份
     */
    KUBERNETES_CLUSTER_COMMON(160, "KubernetesClusterCommon", "Kubernetes_CSI"),

    /**
     * k8s命名空间，1.5.0新增CSI备份
     */
    KUBERNETES_NAMESPACE_COMMON(161, "KubernetesNamespaceCommon", "Kubernetes_CSI"),

    /**
     * k8s dataset，1.5.0新增CSI备份
     */
    KUBERNETES_DATASET_COMMON(162, "KubernetesDatasetCommon", "Kubernetes_CSI"),

    /**
     * HCS适配OP服务化
     */
    HCS_ENV_OP(163, "HcsEnvOp", "HcsEnvOp"),

    /**
     * dws1.5.0新增项目模块
     */
    GAUSSDB_DWS_PROJECT(164, "DWS-project", "GaussDB_DWS"),

    /**
     * TDSQL分布式实例
     */
    TDSQL_CLUSTERGROUP(165, "TDSQL-clusterGroup", "TDSQL"),

    /**
     * 系统卷
     */
    SYSTEM_VOLUME(166, "Volume", "Volume"),

    /**
     * 对象存储
     */
    OBJECT_STORAGE(167, "ObjectStorage", "ObjectStorage"),

    /**
     * 无代理： 通用共享
     */
    COMMON_SHARE(168, "CommonShare", "CommonShare"),

    /**
     * OBS对象集合
     */
    OBJECT_SET(169, "ObjectSet", "ObjectStorage"),

    /**
     * AD域备份
     */
    AD(170, "ADDS", "ADDS"),

    /**
     * 无代理： 通用共享
     */
    AGENT_LESS(171, "AgentLess", "AgentLess"),

    /**
     * CNware环境主机，1.6.0新增CNware备份
     */
    CNWARE(174, "CNware", "CNware"),

    /**
     * CNware环境集群，1.6.0新增CNware备份
     */
    CNWARE_CLUSTER(175, "CNwareCluster", "CNware"),

    /**
     * CNware环境主机，1.6.0新增CNware备份
     */
    CNWARE_HOST(176, "CNwareHost", "CNware"),

    /**
     * CNware环境主机池，1.6.0新增CNware备份
     */
    CNWARE_HOST_POOL(177, "CNwareHostPool", "CNware"),

    /**
     * CNware环境虚拟机，1.6.0新增CNware备份
     */
    CNWARE_VM(178, "CNwareVm", "CNware"),

    /**
     * CNware环境虚拟机磁盘，1.6.0新增CNware备份
     */
    CNWARE_DISK(179, "CNwareDisk", "CNware"),

    /**
     * CNware环境存储池，1.6.0新增CNware备份
     */
    CNWARE_STORAGE_POOL(180, "CNwareStoragePool", "CNware"),

    /**
     * 阿里云ApsaraStack，1.6.0新增阿里云ApsaraStack备份
     */
    APSARA_STACK(181, "ApsaraStack", "ApsaraStack"),

    /**
     * 阿里云 APS-organization，1.6.0新增阿里云ApsaraStack备份
     */
    APS_ORGANIZATION(182, "APS-organization", "ApsaraStack"),

    /**
     * 阿里云 APS-resourceSet，1.6.0新增阿里云ApsaraStack备份
     */
    APS_RESOURCESET(183, "APS-resourceSet", "ApsaraStack"),

    /**
     * 阿里云 APS-region，1.6.0新增阿里云ApsaraStack备份
     */
    APS_REGION(184, "APS-region", "ApsaraStack"),

    /**
     * 阿里云 APS-zone，1.6.0新增阿里云ApsaraStack备份
     */
    APS_ZONE(185, "APS-zone", "ApsaraStack"),

    /**
     * 阿里云 APS-instance，1.6.0新增阿里云ApsaraStack备份
     */
    APS_INSTANCE(186, "APS-instance", "ApsaraStack"),

    /**
     * 阿里云 APS-disk，1.6.0新增阿里云ApsaraStack备份
     */
    APS_DISK(187, "APS-disk", "ApsaraStack"),

    /**
     * Exchange 1.6.0新增单机备份
     */
    EXCHANGE_SINGLE_NODE(188, "Exchange-single-node", "Exchange"),

    /**
     * exchange 邮箱
     */
    EXCHANGE_MAILBOX(189, "Exchange-mailbox", "Exchange"),

    /**
     * Exercise 演练默认类型
     */
    EXERCISE_DEFAULT_DATABASE(190, "ExerciseDefaultDatabase", "ExerciseDefaultDatabase"),

    /**
     * HyperV
     */
    HYPERV(191, "HyperV", "HyperV"),

    /**
     * LUN
     */
    LUN(192, "LUN", "LUN"),

    /**
     * FusionOne Compute
     */
    FUSION_ONE_COMPUTE(193, "FusionOneCompute", "FusionOneCompute");


    private static final List<String> COMMON_AGENT_LIST = Collections.unmodifiableList(
        Arrays.asList(U_BACKUP_AGENT.getType(), S_BACKUP_AGENT.getType(), VM_BACKUP_AGENT.getType()));

    private final Integer order;

    /**
     * 资源子类型
     */
    private final String subType;

    /**
     * rbac所属模块特性
     */
    private final String scopeModule;

    ResourceSubTypeEnum(int order, String subType, String scopeModule) {
        this.order = order;
        this.subType = subType;
        this.scopeModule = scopeModule;
    }

    /**
     * get enum by value
     *
     * @param subType subType
     * @return enum
     */
    @JsonCreator
    public static ResourceSubTypeEnum get(String subType) {
        return EnumUtil.get(ResourceSubTypeEnum.class, ResourceSubTypeEnum::getType, subType);
    }

    /**
     * get order by value
     *
     * @param subType subType
     * @return order
     */
    public static Integer getOrderBySubTypeSilent(String subType) {
        ResourceSubTypeEnum resourceSubTypeEnum = EnumUtil.get(ResourceSubTypeEnum.class, ResourceSubTypeEnum::getType,
            subType, false, true);
        return resourceSubTypeEnum == null ? -1 : resourceSubTypeEnum.getOrder();
    }

    /**
     * get order by value
     *
     * @param subType subType
     * @return order
     */
    public static String getScopeModuleBySubType(String subType) {
        if (VerifyUtil.isEmpty(subType)) {
            return Strings.EMPTY;
        }
        for (ResourceSubTypeEnum resourceSubTypeEnum : ResourceSubTypeEnum.values()) {
            if (resourceSubTypeEnum.getType().equals(subType)) {
                return resourceSubTypeEnum.getScopeModule();
            }
        }
        if (subType.contains("Plugin")) {
            return "AGENT_PLUGIN";
        }
        return Strings.EMPTY;
    }

    /**
     * 根据所属模块查询所有子类列表
     *
     * @param scopeModule 所属模块
     * @return 子类列表
     */
    public static List<String> getSubTypeListByScopeModule(String scopeModule) {
        List<String> subTypeList = new ArrayList<>();
        for (ResourceSubTypeEnum resourceSubTypeEnum : ResourceSubTypeEnum.values()) {
            if (resourceSubTypeEnum.getScopeModule().equals(scopeModule)) {
                subTypeList.add(resourceSubTypeEnum.getType());
            }
        }
        return subTypeList;
    }

    /**
     * 判断是否选择备份代理provider
     *
     * @param subType subType
     * @return boolean
     */
    public static boolean isSelectAgent(String subType) {
        return COMMON_AGENT_LIST.contains(subType);
    }

    /**
     * get value
     *
     * @return string
     */
    @JsonValue
    public String getType() {
        return subType;
    }

    public Integer getOrder() {
        return order;
    }

    public String getScopeModule() {
        return scopeModule;
    }

    /**
     * 和字符串对比，是否相同
     *
     * @param otherSubType 其他子类型
     * @return 是否相同
     */
    public boolean equalsSubType(String otherSubType) {
        return subType.equals(otherSubType);
    }
}