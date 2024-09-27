/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dmc;

import openbackup.data.access.client.sdk.api.config.achive.DmeResponse;
import openbackup.data.access.client.sdk.api.framework.dmc.model.CopyDetail;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * Dmc Copy Service Api
 *
 * @author l00272247
 * @since 2022-01-20
 */
public interface DmcCopyServiceApi {
    /**
     * 查询备份产生的副本详情
     *
     * @param id id
     * @param appType apptype
     * @param scn scn
     * @param time time
     * @param databaseId resourceid
     * @return dme response
     */
    @ExterAttack
    @GetMapping("/v1/srv_jobmanager/backup")
    @ResponseBody
    DmeResponse<CopyDetail> queryCopyById(
            @RequestParam("id") String id,
            @RequestParam("apptype") String appType,
            @RequestParam("scn") Integer scn,
            @RequestParam("time") Integer time,
            @RequestParam("resourceid") String databaseId);

    /**
     * 查询备份产生的副本详情
     *
     * @param id id
     * @param appType app type
     * @return dme response
     */
    default DmeResponse<CopyDetail> queryCopyById(String id, String appType) {
        return queryCopyById(id, appType, null, null, null);
    }
}
