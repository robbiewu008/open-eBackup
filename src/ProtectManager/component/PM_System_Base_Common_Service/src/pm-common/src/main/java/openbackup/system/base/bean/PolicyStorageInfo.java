package openbackup.system.base.bean;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * policy extParameters中保存的指定目标位置
 *
 * @author w30044259
 * @since 2024-03-24
 */
@Getter
@Setter
public class PolicyStorageInfo {
    @JsonProperty("storage_type")
    private String storageType;

    @JsonProperty("storage_id")
    private String storageId;
}
