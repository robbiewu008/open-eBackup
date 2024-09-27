/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dee.model;

import lombok.Data;

import java.util.Map;

/**
 * 一体机共享路径恢复共享信息
 *
 * @author w00574036
 * @since 2024-04-19
 * @version [OceanCyber 300 1.2.0]
 */
@Data
public class OcLiveMountFsShareInfo {
    /**
     * 共享类型 0: cifs; 1: nfs
     */
    private int type;

    /**
     * 克隆文件系统名称
     */
    private String fileSystemName;

    /**
     * 0:只读; 1:读写
     */
    private int accessPermission;

    /**
     * nfs客户端或cifs用户信息
     * nfs:           |  cifs:
     * clientType:0   |  shareName:Mount_***
     * clientName:*   |  domainType:2
     * squash:1       |  userNames:[user1,user2]
     * rootSquash:1   |
     * portSquash:1   |
     */
    private Map<String, Object> advanceParams;
}
