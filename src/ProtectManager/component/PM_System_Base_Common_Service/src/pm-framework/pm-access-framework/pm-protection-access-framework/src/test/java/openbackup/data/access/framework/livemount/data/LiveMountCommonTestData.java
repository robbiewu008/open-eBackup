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
package openbackup.data.access.framework.livemount.data;

import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import java.util.Collections;

/**
 * live mount service test
 *
 * @author h30003246
 * @since 2021-02-25
 */
public class LiveMountCommonTestData {
    /**
     *  disk id
     */
    public static final String DISK_ID = "83445bf0-f601-4509-b6c1-15534318206h";

    private static final String LIVE_MOUNT_ID = "83445bf0-f601-4509-b6c1-05534318206d";
    private static final String MOUNTED_COPY_ID = "83445bf0-f601-4509-b6c1-05534318206b";
    private static final String SOURCE_ID = "83445bf0-f601-4509-b6c1-05534318206c";
    private static final String RESOURCE_NAME = "83445bf0-f601-4509-b6c1-05534318206e";
    private static final String SOURCE_TYPE = "83445bf0-f601-4509-b6c1-05534318206e";
    private static final String SOURCE_SUB_TYPE = "83445bf0-f601-4509-b6c1-05534318206f";
    private static final String USER_ID = "83445bf0-f601-4509-b6c1-05534318206g";
    private static final String ROLE_ID = "83445bf0-f601-4509-b6c1-05534318206h";
    private static final String TARGET_RESOURCE_ID = "83445bf0-f601-4509-b6c1-055343182064";

    /**
     * get clone copy
     *
     * @return copy
     */
    public static Copy getCloneCopy() {
        Copy copy = new Copy();
        copy.setStatus("Normal");
        copy.setResourceId("uuid0");
        copy.setGeneration(1);
        copy.setFeatures(14);
        copy.setResourceSubType("HCSCloudHost");
        copy.setTimestamp(Long.toString(System.currentTimeMillis()));
        copy.setProperties("{\"filesystemName\":\"zc_veeam_1\",\"snapshotId\":\"1480@efc1cedd-f2e2-4d5d-870b-c45214f9a68c\",\"dataAfterReduction\":0,\"format\":0,\"isSanClient\":\"false\",\"size\":0,\"tenantName\":\"System_vStore\",\"verifyStatus\":\"3\",\"repositories\":[{\"id\":\"\",\"type\":1,\"protocol\":1,\"role\":1,\"remotePath\":[{\"type\":1,\"path\":\"/zc_veeam_1\",\"id\":\"1480\",\"parentId\":\"0\"}],\"extendInfo\":{\"esn\":\"2102355MFD10P4100001\",\"copy_format\":1},\"path\":\"\",\"auth\":null,\"extendAuth\":{\"authType\":2,\"authKey\":\"admin\",\"authPwd\":\"Admin@storage1\",\"extendInfo\":{}},\"endpoint\":{\"id\":\"\",\"ip\":\"8.40.162.82\",\"port\":8088,\"agentOS\":\"\",\"wwpns\":[],\"iqns\":[],\"sanClients\":[],\"advanceParams\":{}},\"proxy\":null,\"transProtocol\":\"\",\"local\":true,\"isLocal\":true}],\"snapshotName\":\"efc1cedd-f2e2-4d5d-870b-c45214f9a68c\",\"dataBeforeReduction\":0,\"tenantId\":\"0\",\"multiFileSystem\":\"false\",\"filesystemId\":\"1480\",\"backup_id\":\"1bbe4077-a6bc-4efe-80f9-4ecab8f4e786\",\"fileSystemShareInfo\":[{\"type\":1,\"fileSystemName\":\"Mount_1700490661941\",\"accessPermission\":1,\"advanceParams\":{\"clientType\":0,\"clientName\":\"*\",\"squash\":1,\"rootSquash\":1,\"portSecure\":1}}]}");
        return copy;
    }

    /**
     * get token
     *
     * @return token
     */
    public static TokenBo getToken() {
        TokenBo.RoleBo roleBo = new TokenBo.RoleBo();
        roleBo.setId(ROLE_ID);
        roleBo.setName("Role_sys_admin");

        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId(USER_ID);
        userBo.setRoles(Collections.singletonList(roleBo));
        userBo.setName("sysadmin");

        long exp = 144444444444L;
        long created = 144444444444L;
        return TokenBo.builder().user(userBo).exp(exp).created(created).build();
    }

    /**
     * get live mount entity
     *
     * @return live mount entity
     */
    public static LiveMountEntity getLiveMountEntity() {
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setId(LIVE_MOUNT_ID);
        liveMountEntity.setStatus("available");
        liveMountEntity.setMountedCopyId(MOUNTED_COPY_ID);
        liveMountEntity.setResourceName("test2");
        liveMountEntity.setResourceType(ResourceTypeEnum.HOST.getType());
        liveMountEntity.setResourceSubType(ResourceSubTypeEnum.ESX.getType());
        liveMountEntity.setTargetResourceName("test4");
        liveMountEntity.setTargetResourceId(TARGET_RESOURCE_ID);
        return liveMountEntity;
    }
}
