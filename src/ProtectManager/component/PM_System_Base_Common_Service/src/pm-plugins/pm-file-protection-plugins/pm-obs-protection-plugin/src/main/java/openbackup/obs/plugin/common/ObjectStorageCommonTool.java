package openbackup.obs.plugin.common;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.obs.plugin.common.constants.EnvironmentConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

import java.util.List;

/**
 * OBS对象集合备份公共类
 *
 * @author l00370588
 * @since 2023-12-25
 */
@Slf4j
public class ObjectStorageCommonTool {
    /**
     * repository role 角色 master
     */
    public static final int MASTER_ROLE = 0;

    /**
     * ens key 名称
     */
    public static final String REPOSITORIES_KEY_ENS = "esn";

    /**
     * 校验endpoint
     *
     * @param endpoint 对象存储EndPoin
     * @throws LegoCheckedException 参数校验异常
     */
    public static void checkEndPoint(String endpoint) {
        if (StringUtils.isEmpty(endpoint)) {
            log.error("endpoint is empty.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "endpoint illegal.");
        }
        // https默认端口443，校验规则：有端口校验端口是否在65535以内
        if (!EnvironmentConstant.IP_PORT_PATTERN.matcher(endpoint).matches()
            && !EnvironmentConstant.DOMAIN_PORT_PATTERN.matcher(endpoint).matches()) {
            log.error("endpoint port is invalid.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "endpoint or port illegal.");
        }
    }

    /**
     * 给Repository修改 角色
     *
     * @param repositorys repository仓库列表
     * @param esn 本地esn号
     */
    public static void addRepositoryRole(List<StorageRepository> repositorys, String esn) {
        repositorys.forEach(repository -> {
            if (StringUtils.equals(repository.getId(), esn)) {
                repository.setRole(MASTER_ROLE);
            }
        });
    }
}
