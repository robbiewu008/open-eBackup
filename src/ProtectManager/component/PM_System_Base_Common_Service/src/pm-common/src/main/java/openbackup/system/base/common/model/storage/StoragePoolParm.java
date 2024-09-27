package openbackup.system.base.common.model.storage;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 存储池参数
 *
 * @author w00493811
 * @since 2021-01-12
 */
@Data
@NoArgsConstructor
public class StoragePoolParm {
    private static final int ENDING_UP_THRESHOLD = 95;

    @JsonProperty("USERCONSUMEDCAPACITYTHRESHOLD")
    private int userConsumedCapacityThreshold;

    @JsonProperty("ENDINGUPTHRESHOLD")
    private int endingUpThreshold = ENDING_UP_THRESHOLD;

    public StoragePoolParm(int userConsumedCapacityThreshold) {
        this.userConsumedCapacityThreshold = userConsumedCapacityThreshold;
    }
}


