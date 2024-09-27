package openbackup.gaussdbdws.protection.access.util;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.constant.DwsErrorCode;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.ObjectUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述: 校验 DWS各种参数的基本格式等信息
 *
 * @author l00570077
 * @version [OceanProtect x8000 1.2.1]
 * @since 2022-06-24
 */
@Slf4j
public class DwsValidator {
    /**
     * 检查已经接入的 dws cluster是否已经超过规格
     *
     * @param existingEnvironments 已接入的Dws环境列表
     */
    public static void checkDwsCount(List<ProtectedEnvironment> existingEnvironments) {
        if (existingEnvironments.size() >= DwsConstant.GAUSSDB_DWS_CLUSTER_MAX_COUNT) {
            throw new LegoCheckedException(DwsErrorCode.DWS_RESOURCE_REACHED_THE_UPPER_LIMIT,
                new String[] {String.valueOf(DwsConstant.GAUSSDB_DWS_CLUSTER_MAX_COUNT)});
        }
    }

    /**
     * 通过 dws集群 id检查dws集群是否重复接入
     *
     * @param existingEnvironments 已接入的Dws cluster列表
     * @param dwsClusterId 新接入的 Dws id
     */
    public static void checkDwsResourceExistById(List<ProtectedEnvironment> existingEnvironments, String dwsClusterId) {
        for (ProtectedEnvironment environment : existingEnvironments) {
            if (Objects.equals(dwsClusterId, environment.getUuid())) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                    "The dws cluster id: " + dwsClusterId + " already exist.");
            }
        }
    }

    /**
     * 判断是否存在相同agent的集群uuid或者主机uuid;
     *
     * @param existingEnvironments 已接入的Dws cluster列表
     * @param uuidList 集群和主机的uuid集合
     */
    public static void checkDwsExistSameClusterOrHost(List<ProtectedEnvironment> existingEnvironments,
        List<String> uuidList) {
        for (ProtectedEnvironment environment : existingEnvironments) {
            Optional.ofNullable(environment.getDependencies().get(DwsConstant.DWS_CLUSTER_AGENT))
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "name is not exist"))
                .stream()
                .map(ProtectedResource::getUuid)
                .forEach(uuid -> checkExistUuid(uuidList, uuid));
            Optional.ofNullable(environment.getDependencies().get(DwsConstant.HOST_AGENT))
                .orElse(new ArrayList<>())
                .stream()
                .map(ProtectedResource::getUuid)
                .forEach(uuid -> checkExistUuid(uuidList, uuid));
        }
    }

    /**
     * 判断是否存在相同agent的uuid;
     *
     * @param uuidList 集群和主机的uuid集合
     * @param uuid 单个agent的uuid
     */
    public static void checkExistUuid(List<String> uuidList, String uuid) {
        if (uuidList.contains(uuid)) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                "The dws cluster id: " + uuid + " already exist.");
        }
    }

    /**
     * 通过 dws集群 endpoint检查dws集群是否重复接入
     *
     * @param existingEnvironments 已接入的Dws cluster列表
     * @param endpoint controller ip
     */
    public static void checkDwsResourceExistByEndpoint(List<ProtectedEnvironment> existingEnvironments,
        String endpoint) {
        for (ProtectedEnvironment environment : existingEnvironments) {
            if (Objects.equals(endpoint, environment.getEndpoint())) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                    "The dws cluster endpoint: " + endpoint + " already exist.");
            }
        }
    }

    /**
     * 校验 入参的值是否为空
     *
     * @param value 入参的值
     */
    public static void checkDwsValue(String value) {
        if (VerifyUtil.isEmpty(value)) {
            log.error("dws value: {} check is empty.", value);
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "dws value: " + value + " check is empty");
        }
    }

    /**
     * 检查需要备份 的名称
     *
     * @param name 备份集名称
     */
    public static void checkDwsNameFormat(String name) {
        if (VerifyUtil.isEmpty(name) || !ValidateUtil.match(RegexpConstants.NAME_STR, name)) {
            log.error("dws BackupSet check, name is illegal, name:{}", name);
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "BackupSet name is illegal.");
        }
    }

    /**
     * 检查需要备份对象是否存在相同数据库
     *
     * @param tableInfos 备份对象列表
     */
    public static void checkDwsExistSameDatabaseName(String[] tableInfos) {
        List<String> dwsDatabaseNameList = Arrays.stream(tableInfos)
            .map(table -> table.split("/")[0])
            .distinct()
            .collect(Collectors.toList());
        if (dwsDatabaseNameList.size() != 1) {
            log.error("dws BackupSet check, dws database size:{}", dwsDatabaseNameList.size());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "dws BackupSet check, dws database size: " + dwsDatabaseNameList.size());
        }
    }

    /**
     * 检查需要备份对象是否存在相同数据库
     *
     * @param tableInfos 备份对象列表比如: database1/schema1/table1,database1/schema1/table2,database1/schema1/table3,
     *                                  or database1/schema1/table1,database1/schema1
     */
    public static void checkDwsExistSameSchemaName(List<String> tableInfos) {
        List<String> dwsSchemaList = tableInfos.stream()
            .filter(table -> (table.split("/").length == 2))
            .map(table -> table.split("/")[1])
            .distinct()
            .collect(Collectors.toList());
        List<String> dwsTableSchemaList = tableInfos.stream()
            .filter(table -> (table.split("/").length == 3))
            .map(table -> table.split("/")[1])
            .distinct()
            .collect(Collectors.toList());
        if (hasSameItem(dwsTableSchemaList, dwsSchemaList)) {
            log.error("dws BackupSet check, dwsTableSchemaList size :{} , dwsSchemaList size :{}",
                dwsTableSchemaList.size(), dwsSchemaList.size());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "dws BackupSet check, dws exist same schema");
        }
    }

    /**
     * 检查备份对象创建相同table表
     *
     * @param tableInfos table列表
     */
    public static void checkDwsExistName(List<String> tableInfos) {
        long count = tableInfos.stream().distinct().count();
        if (count != tableInfos.size()) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "dws BackupSet check, dws database size: ");
        }
    }

    /**
     * 检查需要备份对象是否存在不符合规则name
     *
     * @param tableInfos 备份对象列表
     * @param subType 子资源类型
     */
    public static void checkDwsNoExistName(String[] tableInfos, String subType) {
        List<String> tableList = Arrays.stream(tableInfos)
            .filter(table -> (table.split("/").length == 2))
            .collect(Collectors.toList());
        if (ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType().equals(subType)) {
            tableList.addAll(
                Arrays.stream(tableInfos).filter(table -> (table.split("/").length == 3)).collect(Collectors.toList()));
        }
        if (tableList.size() != tableInfos.length) {
            log.error("dws BackupSet check, tableInfo No conformance ask tableInfos size:{}, filter tableList: {}",
                tableInfos.length, tableList.size());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "dws BackupSet check, tableInfo No conformance ask tableInfos size: " + tableInfos.length
                    + ", filter tableList: " + tableList.size());
        }
    }

    private static boolean hasSameItem(List<String> oldList, List<String> newList) {
        return !Collections.disjoint(oldList, newList);
    }

    /**
     * 检查 备份对象列表是否大于设定数据
     *
     * @param tableInfos 备份对象列表
     * @param size 大小
     */
    public static void checkDwsSize(String[] tableInfos, int size) {
        if (tableInfos.length > size) {
            log.error("Dws backupSet size > {}.", size);
            throw new LegoCheckedException(DwsErrorCode.CHECK_RESOURCES_SIZE_ERROR, new String[] {String.valueOf(size)},
                "Dws table > " + size + ".");
        }
    }

    /**
     * 检查ProtectedEnvironment是否存在并在线
     *
     * @param environments environment
     */
    public static void checkEnvironment(PageListResponse<ProtectedResource> environments) {
        if (environments.getRecords().size() == 0) {
            log.error("dws BackupSet check, the dws cluster is not exist.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "The dws cluster is not exist.");
        }

        ProtectedResource record = environments.getRecords().stream().findFirst().get();
        if (!(record instanceof ProtectedEnvironment)) {
            log.error("dws BackupSet check, the Dws cluster is illegal.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "The dws cluster is illegal.");
        }

        ProtectedEnvironment protectedEnvironment = (ProtectedEnvironment) record;
        if (!LinkStatusEnum.ONLINE.getStatus().toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment))) {
            log.error("dws BackupSet check, the Dws cluster is offline, uuid:{}", protectedEnvironment.getUuid());
            throw new LegoCheckedException(CommonErrorCode.HOST_NOT_EXIST, "The dws cluster is offline.");
        }
    }

    /**
     * 检查是否存在相同的表
     *
     * @param extendInfoValues 表集合String
     * @param tableInfos 表集合数组
     * @param subType 子资源类型
     */
    public static void checkSameTable(List<String> extendInfoValues, String[] tableInfos, String subType) {
        if (ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType().equals(subType)) {
            return;
        }
        for (String tableInfo : tableInfos) {
            if (extendInfoValues.contains(tableInfo)) {
                log.error("dws exit same table: {}.", tableInfo);
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "dws exit same table");
            }
        }
    }

    /**
     * 检查传入的Name是否相同
     *
     * @param oldName 老名字
     * @param newName 新名字
     */
    public static void checkTheSameName(String oldName, String newName) {
        if (ObjectUtils.notEqual(oldName, newName)) {
            log.error("{} and {} is not equal name", oldName, newName);
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                oldName + " and " + newName + " is not equal name");
        }
    }
}