package openbackup.system.base.sdk.lock;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;

/**
 * 锁定资源信息
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/1/26
 * @see LockTypeEnum
 **/
@Data
@AllArgsConstructor
public class LockResource {
    /**
     * 资源id
     */
    private String id;

    /**
     * 资源锁类型
     */
    @JsonProperty("lockType")
    private LockTypeEnum lockType;
}
