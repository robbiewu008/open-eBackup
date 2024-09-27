package openbackup.system.base.sdk.anti.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;

/**
 * 副本防勒索检测入参
 *
 * @author nwx1077006
 * @since 2021-10-28
 */
@Data
public class CopyDetectionReq {
    /**
     * 副本ID
     */
    @NotBlank
    @Length(min = 1, max = 64)
    private String copyId;

    /**
     * 未感染快照设置安全快照
     */
    @JsonProperty("isSecuritySnap")
    private boolean isSecuritySnap;

    /**
     * 未感染快照设置安全快照
     */
    @JsonProperty("isBackupDetectEnable")
    private boolean isBackupDetectEnable;

    /**
     * 熵值侦测感染阈值
     */
    @JsonProperty("upperBound")
    private int upperBound;
}
