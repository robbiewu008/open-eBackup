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
package openbackup.data.access.framework.core.security;

import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;
import openbackup.system.base.security.context.Context;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.permission.Permission;

import org.springframework.stereotype.Component;

import java.util.AbstractMap;
import java.util.List;
import java.util.Map;

/**
 * Operation
 *
 * @author l00272247
 * @since 2021-12-13
 */
@Component
@CalleeMethods
public class Operation {
    /**
     * 测试验证基本正常场景
     *
     * @param entry entry
     * @return 结果
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN},
            resources = "resource:$1.value")
    @Logging(name = "log-code", target = "Resource#{1}", details = "$1.key")
    public Object operate0(Map.Entry<String, List<String>> entry) {
        return entry.getValue();
    }

    /**
     * 测试验证异常场景
     *
     * @param entry entry
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_RD_ADMIN},
            resources = "resource:$1.value")
    @Logging(name = "log-code", target = "Resource#{1}", details = "$1.key")
    public void operate1(Map.Entry<String, List<String>> entry) {
        throw new LegoCheckedException("some error:" + entry);
    }

    /**
     * 测试验证上下文表达式
     *
     * @param entry entry
     * @return 结果
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_RD_ADMIN},
            resources = "resource:$1.value")
    @Logging(
            name = "log-code",
            target = "Resource#{1}",
            details = "$1.key + ':' + $resource",
            context = {
                @Context(name = "resources", statement = "{'resource-data'}"),
                @Context(name = "resource", statement = "$resources[0]")
            })
    public Object operate2(Map.Entry<String, List<String>> entry) {
        return entry.getValue();
    }

    /**
     * 测试验证定义CalleeMethod
     *
     * @param entry entry
     * @return 结果
     */
    @CalleeMethod
    public Object loadResource(Map.Entry<String, List<String>> entry) {
        return new AbstractMap.SimpleEntry<>(entry.getKey(), "resource-data");
    }

    /**
     * 测试验证引用Bean
     *
     * @param entry entry
     * @return 结果
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_RD_ADMIN},
            resources = "resource:$1.value")
    @Logging(
            name = "log-code",
            target = "Resource#{#format('{0}', #char(49))}",
            details = "$1.key + ':' + @operation_load_resource.call($0).value")
    public Object operate3(Map.Entry<String, List<String>> entry) {
        return entry.getValue();
    }

    /**
     * 测试验证引用返回值
     *
     * @param entry entry
     * @return 结果
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_RD_ADMIN},
            resources = "resource:$1.value")
    @Logging(name = "log-code", target = "Resource#{#format('{0}', #char(49))}", details = "$1.key + ':' + $return[0]")
    public Object operate4(Map.Entry<String, List<String>> entry) {
        return entry.getValue();
    }

    /**
     * 测试验证模板表达式
     *
     * @param entry entry
     * @return 结果
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_RD_ADMIN},
            resources = "resource:$1.value")
    @Logging(
            name = "log-code",
            batch = {"$1.key", "$1.value"},
            target = "Resource#{#format('{0}', #char(49))}",
            details = "$1 + ':' + $2 + ':' + $return[0]")
    public Object operate5(Map.Entry<String, List<String>> entry) {
        return entry.getValue();
    }

    /**
     * 测试验证requires机制
     *
     * @param entry entry
     * @return 结果
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_RD_ADMIN},
            resources = "resource:$1.value")
    @Logging(
            name = "log-code",
            batch = {"$1.key", "$1.value"},
            target = "Resource#{#format('{0}', #char(49))}",
            requires = "null",
            details = "$1 + ':' + $2")
    public Object operate6(Map.Entry<String, List<String>> entry) {
        return entry.getValue();
    }

    /**
     * 测试验证上下文required机制
     *
     * @param entry entry
     * @return 结果
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_RD_ADMIN},
            resources = "resource:$1.value")
    @Logging(
            name = "log-code",
            batch = {"$1.key", "$1.value"},
            target = "Resource#{#format('{0}', #char(49))}",
            details = "$1 + ':' + $2",
            context = {@Context(name = "x", statement = "null", required = true)})
    public Object operate7(Map.Entry<String, List<String>> entry) {
        return entry.getValue();
    }

    /**
     * 测试验证DataProtectionAccessException异常场景
     *
     * @param entry entry
     * @return 结果
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_RD_ADMIN},
            resources = "resource:$1.value")
    @Logging(
            name = "log-code",
            batch = {"$1.key", "$1.value"},
            target = "Resource#{#format('{0}', #char(49))}",
            details = "$1 + ':' + $2",
            context = {@Context(name = "x", statement = "null")})
    public Object operate8(Map.Entry<String, List<String>> entry) {
        throw new DataProtectionAccessException(1, new String[] {"1"});
    }

    /**
     * 测试验证LegoCheckedException异常场景
     *
     * @param entry entry
     * @return 结果
     */
    @Permission(
            roles = {Constants.Builtin.ROLE_RD_ADMIN},
            resources = "resource:$1.value")
    @Logging(
            name = "log-code",
            batch = {"$1.key", "$1.value"},
            target = "Resource#{#format('{0}', #char(49))}",
            details = "$1 + ':' + $2",
            context = {@Context(name = "x", statement = "null")})
    public Object operate9(Map.Entry<String, List<String>> entry) {
        throw new LegoCheckedException(2, new String[] {"2"});
    }
}
