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
package openbackup.system.base.sdk.storage;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;

import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.storage.model.ProductStorageInfoRes;
import openbackup.system.base.sdk.storage.model.ProductStorageReq;
import openbackup.system.base.sdk.storage.model.ProductStorageRes;

import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

import javax.validation.Valid;

/**
 * 存储rest定义
 *
 */
public interface ProductStorageRestApi {
    /**
     * 更新存储信息
     *
     * @param storageId 存储id
     * @param storageDeviceInfo 更新的存储信息
     */
    @ExterAttack
    @PutMapping("/product-storages/{storageId}")
    void updateProductStorage(@PathVariable("storageId") String storageId,
            @RequestBody @Valid ProductStorageReq storageDeviceInfo);

    /**
     * 根据id查询存储信息
     *
     * @param storageId 存储id
     * @return 存储信息
     */
    @ExterAttack
    @GetMapping("/product-storages/{storageId}")
    ProductStorageRes getProductStorage(@PathVariable("storageId") String storageId);

    /**
     * 删除存储资源
     *
     * @param storageId 存储id
     */
    @ExterAttack
    @DeleteMapping("/product-storages/{storageId}")
    void deleteStorage(@PathVariable("storageId") String storageId);

    /**
     * 创建存储资源
     *
     * @param storageDeviceInfo 存储资源信息
     * @param token token
     */
    @ExterAttack
    @PostMapping("/product-storages")
    void createStorage(@RequestBody @Valid ProductStorageReq storageDeviceInfo,
            @RequestHeader("X-Auth-Token") String token);

    /**
     * 根据wwn查设备信息
     *
     * @param wwns wwns 传wwn数组
     * @return 设备信息
     */
    @ExterAttack
    @PostMapping("/internal/product-storages")
    @ResponseBody
    List<ProductStorageInfoRes> getDoradoResourceInfo(@RequestBody @Valid List<String> wwns);

    /**
     * 分页查询
     *
     * @param page 第几页
     * @param size 页的大小
     * @param conditions 查询条件
     * @param orders 排序方式
     * @param token token
     * @return 存储信息
     */
    @ExterAttack
    @GetMapping("/product-storages")
    BasePage<ProductStorageRes> queryProductStorages(@RequestParam("page") int page, @RequestParam("size") int size,
            @RequestParam(value = "conditions", required = false) String conditions,
            @RequestParam(value = "orders", required = false) List<String> orders,
            @RequestHeader("X-Auth-Token") String token);

    /**
     * verify storage ownership
     *
     * @param userId user id
     * @param uuidList uuid list
     */
    @ExterAttack
    @GetMapping("/internal/product-storages/action/verify")
    @ResponseBody
    void verifyStorageOwnership(@RequestParam("userId") String userId, @RequestParam("idList") List<String> uuidList);
}
