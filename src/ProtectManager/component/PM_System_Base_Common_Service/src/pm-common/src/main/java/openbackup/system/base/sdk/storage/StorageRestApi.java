package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.model.repository.StorageInfo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * Storage Rest Api
 *
 * @author l00272247
 * @since 2020-11-30
 */
public interface StorageRestApi {
    /**
     * verify storage ownership
     *
     * @param userId   user id
     * @param uuidList uuid list
     */
    @GetMapping("/internal/product-storages/action/verify")
    @ResponseBody
    void verifyStorageOwnership(@RequestParam("userId") String userId,
        @RequestParam("idList") List<String> uuidList);

    /**
     * 获取存储信息
     *
     * @return 存储信息
     */
    @ExterAttack
    @GetMapping("/internal/storages/storage-info")
    @ResponseBody
    List<StorageInfo> storageInfo();
}
