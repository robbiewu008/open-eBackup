package openbackup.system.base.sdk.lock;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 资源锁同步加锁响应
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/15
 **/
@Setter
@Getter
@NoArgsConstructor
public class LockResponse {
    /**
     * 是否加锁成功：true-成功，false-失败
     */
    @JsonProperty("isSuccess")
    private boolean isSuccess;

    /**
     * 失败的资源，当加锁失败时返回 <br>
     * isSuccess为false可能是由于其他错误导致，此字段并不一定会返回
     */
    private String failedResource;
}
