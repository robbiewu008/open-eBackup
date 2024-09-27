package openbackup.system.base.sdk.copy.model;

import openbackup.system.base.util.EnumUtil;

import com.alibaba.fastjson.annotation.JSONCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * Copy Status
 *
 * @author l00272247
 * @since 2020-10-30
 */
public enum CopyStatus {
    /**
     * INVALID
     */
    INVALID("Invalid"),
    /**
     * NORMAL
     */
    NORMAL("Normal"),
    /**
     * RESTORING
     */
    RESTORING("Restoring"),
    /**
     * MOUNTING
     */
    MOUNTING("Mounting"),

    /**
     * MOUNTED
     */
    MOUNTED("Mounted"),

    /**
     * UNMOUNTING
     */
    UNMOUNTING("Unmounting"),
    /**
     * DELETING
     */
    DELETING("Deleting"),
    /**
     * VERIFYING
     */
    VERIFYING("Verifying"),
    /**
     * Delete failed
     * */
    DELETEFAILED("DeleteFailed"),
    /**
     * Sharing
     * */
    SHARING("Sharing"),

    /**
     * Downloading
     */
    DOWNLOADING("Downloading");

    private final String value;

    CopyStatus(String value) {
        this.value = value;
    }

    /**
     * get enum by value
     *
     * @param value value
     * @return copy status
     */
    @JSONCreator
    public static CopyStatus get(String value) {
        return EnumUtil.get(CopyStatus.class, CopyStatus::getValue, value);
    }

    @JsonValue
    public String getValue() {
        return value;
    }
}
