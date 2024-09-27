package openbackup.obs.plugin.entity;

import lombok.Data;

import java.util.List;

/**
 * BucketInfoEntity
 *
 * @author c30035089
 * @since 2023-11-21
 */
@Data
public class BucketInfoEntity {
    /**
     * bucket 名称
     */
    private String name;

    /**
     * 前缀
     */
    private List<String> prefix;
}
