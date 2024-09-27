package openbackup.system.base.common.model.storage;

import lombok.Data;

/**
 * 删除存储库请求
 *
 * @author w00504341
 * @since 2020-12-18
 */
@Data
public class DeleteStorageRes {
    private Integer status;

    private String slaId;
}
