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
package openbackup.database.base.plugin.util;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.util.StreamUtils;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;

/**
 * 测试conf辅助类
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-29
 */
public class TestConfHelper {
    public static final String SAP_HANA = "hana";

    public static String getHanaConf(){
        InputStream inputStream = TestConfHelper.class.getClassLoader().getResourceAsStream("hana/conf.json");
        try {
            return  StreamUtils.copyToString(inputStream, StandardCharsets.UTF_8);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * env
     * ----children
     * ---------host
     *
     * @param isSingle 是否是单实例
     * @return env
     */
    public static ProtectedEnvironment mockInstance(boolean isSingle){
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        // env properties
        protectedEnvironment.setEndpoint("1.1.1.1");
        Authentication auth = new Authentication();
        auth.setAuthPwd("password");
        auth.setExtendInfo(new HashMap<String, String>() {{
            put("a", "b");
        }});
        protectedEnvironment.setAuth(auth);
        protectedEnvironment.setDependencies(new HashMap<>());
        protectedEnvironment.setName("test");
        protectedEnvironment.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY, SAP_HANA);
        protectedEnvironment.setExtendInfoByKey(GeneralDbConstant.EXTEND_FIRST_CLASSIFICATION_KEY,
            GeneralDbConstant.GENERAL_DB_INSTANCE);

        if (isSingle) {
            protectedEnvironment.setExtendInfoByKey(GeneralDbConstant.EXTEND_INSTANCE_TYPE_KEY,
                GeneralDbConstant.INSTANCE_SINGLE);
            ProtectedResource host1 = new ProtectedEnvironment();
            host1.setUuid("host1");
            protectedEnvironment.getDependencies()
                .put(GeneralDbConstant.DEPENDENCY_HOST_KEY, new ArrayList<>(Collections.singletonList(host1)));
        } else {
            protectedEnvironment.setExtendInfoByKey(GeneralDbConstant.EXTEND_INSTANCE_TYPE_KEY,
                GeneralDbConstant.INSTANCE_CLUSTER);
            // node
            ProtectedResource node1 = new ProtectedResource();
            node1.setName("node1");
            node1.setSubType(ResourceSubTypeEnum.GENERAL_DB.getType());

            ProtectedResource node2 = new ProtectedResource();
            node2.setName("node2");
            node2.setSubType(ResourceSubTypeEnum.GENERAL_DB.getType());
            protectedEnvironment.getDependencies()
                .put(GeneralDbConstant.DEPENDENCY_CLUSTER_NODE_KEY, new ArrayList<ProtectedResource>() {{
                    add(node1);
                    add(node2);
                }});

            // host
            ProtectedResource host1 = new ProtectedEnvironment();
            host1.setUuid("host1");
            node1.setDependencies(new HashMap<>());
            node1.getDependencies().put(GeneralDbConstant.DEPENDENCY_HOST_KEY, new ArrayList<>(Collections.singletonList(host1)));
            ProtectedResource host2 = new ProtectedEnvironment();
            host2.setUuid("host2");
            node2.setDependencies(new HashMap<>());
            node2.getDependencies().put(GeneralDbConstant.DEPENDENCY_HOST_KEY, new ArrayList<>(Collections.singletonList(host2)));
        }

        return protectedEnvironment;
    }

    /**
     * env
     * ----host
     *
     * @param isSingle 是否是单机
     * @return env
     */
    public static ProtectedEnvironment mockDatabase(boolean isSingle) {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        // env properties
        protectedEnvironment.setEndpoint("1.1.1.1");
        Authentication auth = new Authentication();
        auth.setAuthPwd("password");
        auth.setExtendInfo(new HashMap<String, String>() {{
            put("a", "b");
        }});
        protectedEnvironment.setAuth(auth);
        protectedEnvironment.setDependencies(new HashMap<>());
        protectedEnvironment.setName("test");
        protectedEnvironment.setExtendInfoByKey(GeneralDbConstant.EXTEND_SCRIPT_KEY, SAP_HANA);
        protectedEnvironment.setExtendInfoByKey(GeneralDbConstant.EXTEND_FIRST_CLASSIFICATION_KEY,
            GeneralDbConstant.GENERAL_DB_DATABASE);

        if (isSingle) {
            protectedEnvironment.setExtendInfoByKey(GeneralDbConstant.EXTEND_DEPLOY_TYPE,
                DatabaseDeployTypeEnum.SINGLE.getType());
            ProtectedEnvironment host1 = new ProtectedEnvironment();
            host1.setUuid("host1");
            protectedEnvironment.getDependencies()
                .put(GeneralDbConstant.DEPENDENCY_HOST_KEY, new ArrayList<>(Collections.singletonList(host1)));
        } else {
            protectedEnvironment.setExtendInfoByKey(GeneralDbConstant.EXTEND_DEPLOY_TYPE,
                DatabaseDeployTypeEnum.AP.getType());
            ProtectedEnvironment host1 = new ProtectedEnvironment();
            host1.setUuid("host1");
            ProtectedEnvironment host2 = new ProtectedEnvironment();
            host2.setUuid("host2");
            protectedEnvironment.getDependencies()
                .put(GeneralDbConstant.DEPENDENCY_HOST_KEY, new ArrayList<>(Arrays.asList(host1, host2)));
        }

        return protectedEnvironment;
    }

    public static List<ProtectedEnvironment> mockHost() {
        ProtectedEnvironment host1 = new ProtectedEnvironment();
        host1.setUuid("host1");
        host1.setName("host1Name");
        host1.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        host1.setEndpoint("9.0.0.1");
        host1.setPort(91);

        ProtectedEnvironment host2 = new ProtectedEnvironment();
        host2.setUuid("host2");
        host2.setName("host2Name");
        host2.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        host2.setEndpoint("9.0.0.2");
        host2.setPort(92);

        return Arrays.asList(host1,host2);
    }
}
