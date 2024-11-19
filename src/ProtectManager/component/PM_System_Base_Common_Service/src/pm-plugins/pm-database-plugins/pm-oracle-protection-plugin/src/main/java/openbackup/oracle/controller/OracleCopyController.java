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
package openbackup.oracle.controller;

import lombok.extern.slf4j.Slf4j;
import openbackup.oracle.constants.ScnCopy;
import openbackup.oracle.service.OracleCopyService;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.permission.Permission;

import org.hibernate.validator.constraints.Length;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

import javax.validation.constraints.NotNull;

/**
 * Oracle副本相关接口
 *
 */
@Slf4j
@RestController
@RequestMapping("/v2")
public class OracleCopyController {
    private final OracleCopyService oracleCopyService;

    /**
     * 构造
     *
     * @param oracleCopyService oracleCopyService
     */
    public OracleCopyController(OracleCopyService oracleCopyService) {
        this.oracleCopyService = oracleCopyService;
    }

    /**
     * 根据scn查询副本信息
     *
     * @param resourceId 资源id
     * @param filterValue 筛选值
     * @return 副本信息
     */
    @ExterAttack
    @GetMapping("/databases/oracle/copies/scn")
    @ResponseBody
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN, Constants.Builtin.ROLE_DP_ADMIN}, resources = "resource:$1",
        enableCheckAuth = false)
    public List<ScnCopy> queryCopyByScn(@NotNull @Length(max = 128) @RequestParam("resourceId") String resourceId,
        @NotNull @Length(max = 15) @RequestParam("filterValue") String filterValue) {
        return oracleCopyService.listCopiesInfo(resourceId, filterValue);
    }
}
