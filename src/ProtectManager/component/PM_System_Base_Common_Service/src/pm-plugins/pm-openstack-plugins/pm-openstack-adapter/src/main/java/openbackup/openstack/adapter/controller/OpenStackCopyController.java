/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.openstack.adapter.controller;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;
import openbackup.openstack.adapter.controller.resp.OpenStackCopiesResp;
import openbackup.openstack.adapter.dto.OpenStackCopyDto;
import openbackup.openstack.adapter.service.OpenStackCopyAdapter;

import lombok.extern.slf4j.Slf4j;

import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

/**
 * 云核OpenStack副本北向接口
 * <p>
 * 提供第三方调用，不使用系统内部token校验机制
 * </p>
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-17
 */
@Slf4j
@RestController
@RequestMapping("/v2/backup_copies")
public class OpenStackCopyController {
    private final OpenStackCopyAdapter adapter;

    public OpenStackCopyController(OpenStackCopyAdapter adapter) {
        this.adapter = adapter;
    }

    /**
     * 查询指定副本信息
     *
     * @param copyId 副本id
     * @return {@link OpenStackCopyDto} OpenStack副本信息
     */
    @ExterAttack
    @GetMapping("/{id}")
    public OpenStackCopyDto queryCopy(@PathVariable("id") String copyId) {
        log.info("Openstack query copy: {}.", copyId);
        return adapter.queryCopy(copyId);
    }

    /**
     * 查询所有副本
     *
     * @param resourceId 资源id
     * @return 备份副本列表
     */
    @ExterAttack
    @GetMapping
    public OpenStackCopiesResp queryCopies(@RequestParam("backup_job_id") String resourceId) {
        log.info("Openstack query all copies of resource: {}.", resourceId);
        return new OpenStackCopiesResp(adapter.queryCopies(resourceId));
    }

    /**
     * 删除副本
     *
     * @param copyId 副本id
     */
    @ExterAttack
    @DeleteMapping("/{id}")
    public void deleteCopy(@PathVariable("id") String copyId) {
        log.info("Openstack delete copy: {}.", copyId);
        adapter.deleteCopy(copyId);
    }
}
