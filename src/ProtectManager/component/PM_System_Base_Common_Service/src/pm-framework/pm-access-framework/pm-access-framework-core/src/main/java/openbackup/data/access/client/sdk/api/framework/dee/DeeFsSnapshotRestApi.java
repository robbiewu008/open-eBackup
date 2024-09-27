package openbackup.data.access.client.sdk.api.framework.dee;

import openbackup.data.access.client.sdk.api.framework.dee.model.CreateFsSnapshotRequest;
import openbackup.data.access.client.sdk.api.framework.dee.model.DeleteFsSnapshotRequest;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * 文件系统快照DEE REST接口
 *
 * @author q00564609
 * @since 2024-6-17
 * @version OceanCyber 300 1.2.0
 */
@FeignClient(name = "deeFsSnapshotRestApi",
    url = "${data-enable-engine-server.url}/v1/internal/",
    configuration = CommonFeignConfiguration.class)
public interface DeeFsSnapshotRestApi {
    /**
     * 创建文件系统快照
     *
     * @param createFsSnapshotRequest 创建文件系统快照请求参数
     */
    @ExterAttack
    @PostMapping("anti/ransomware/fssnapshots")
    void createFsSnapshot(@RequestBody CreateFsSnapshotRequest createFsSnapshotRequest);

    /**
     * 删除文件系统快照
     *
     * @param deleteFsSnapshotRequest  删除文件系统快照请求参数
     */
    @ExterAttack
    @DeleteMapping("anti/ransomware/fssnapshots")
    void deleteFsSnapshot(@RequestBody DeleteFsSnapshotRequest deleteFsSnapshotRequest);
}
