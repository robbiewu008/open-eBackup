/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.service;

import openbackup.system.base.sdk.auth.model.HcsTokenParam;
import openbackup.system.base.sdk.auth.model.TokenParam;

/**
 * op服务化，跨云复制服务类
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/7/26
 */
public interface HcsOpService {
    /**
     * 获取hcs的token
     *
     * @param hcsTokenParam 查询参数
     * @return token
     */
    String getHcsToken(HcsTokenParam hcsTokenParam);

    /**
     * 添加域名映射
     *
     * @param ip 待映射的ip
     * @param host 待映射的host
     * @return 添加域名映射是否成功
     */
    boolean addHostMapping(String ip, String host);

    /**
     * 获取hcs运营面的token，跨云复制使用
     *
     * @param param 获取token参数
     * @return token信息
     */
    String getHcsMoToken(TokenParam param);

    /**
     * 添加iam和sc的域名映射信息到/etc/hosts中
     *
     * @param ip IP地址
     * @param domain 域名
     */
    void addIamAndScMapping(String ip, String domain);

    /**
     * 检查hcs集群的token
     *
     * @param clusterId 集群id
     * @param moToken token
     * @param errorCode 错误码
     */
    void checkHcsToken(int clusterId, String moToken, long errorCode);
}
