package openbackup.openstack.adapter.enums;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 云核OpenStack任务执行结果
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
public enum JobResult {
    /**
     * 成功
     */
    SUCCESS("success"),

    /**
     * 失败
     */
    FAIL("fail"),

    /**
     * 未执行或正在执行
     */
    OTHERS("");

    private final String result;

    JobResult(String result) {
        this.result = result;
    }

    @JsonValue
    public String getResult() {
        return result;
    }
}
