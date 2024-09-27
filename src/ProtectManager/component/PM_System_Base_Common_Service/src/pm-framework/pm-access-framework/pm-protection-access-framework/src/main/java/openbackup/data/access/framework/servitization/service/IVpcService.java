package openbackup.data.access.framework.servitization.service;

import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;

import java.util.List;
import java.util.Set;

/**
 * IVpcService
 *
 * @author l30044826
 * @since 2023-08-11
 */
public interface IVpcService {
    /**
     * 保存vpcInfo
     *
     * @param projectId 项目id
     * @param vpcId vpcId
     * @param markId markId
     * @return id
     */
    String saveVpcInfo(String projectId, String vpcId, String markId);

    /**
     * 查询vpc信息
     *
     * @return vpcInfos
     */
    List<VpcInfoEntity> getVpcInfos();

    /**
     * 删除vpc
     *
     * @param markId markId
     * @return 是否删除成功
     */
    boolean deleteVpcInfo(String markId);

    /**
     * 根据markId查询
     *
     * @param markId markId
     * @return vpc信息
     */
    VpcInfoEntity getProjectIdByMarkId(String markId);

    /**
     * 根据userId查询
     *
     * @param userId userId
     * @return vpc信息列表
     */
    List<VpcInfoEntity> getVpcInfoEntityByProjectId(String userId);

    /**
     * 根据vpcId查询vpc
     *
     * @param vpcIds vpcIds
     * @return vpc信息
     */
    List<VpcInfoEntity> getVpcByVpcIds(Set<String> vpcIds);
}
