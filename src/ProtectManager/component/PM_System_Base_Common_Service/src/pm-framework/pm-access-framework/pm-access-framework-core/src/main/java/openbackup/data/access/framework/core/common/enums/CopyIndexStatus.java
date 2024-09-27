package openbackup.data.access.framework.core.common.enums;

import com.fasterxml.jackson.annotation.JsonValue;

import lombok.Getter;

/**
 * Copy index status
 *
 * @author l00347293
 * @since 2021-01-07
 */
@Getter
public enum CopyIndexStatus {
    /**
     * 未索引
     */
    UNINDEXED("Unindexed"),

    /**
     * 索引中
     */
    INDEXING("Indexing"),

    /**
     * 已索引
     */
    INDEXED("Indexed"),

    /**
     * 索引失败
     */
    INDEX_FAIL("Index_fail"),

    /**
     * 索引删除失败
     */
    INDEX_DELETING("Index_deleting"),

    /**
     * 索引删除失败
     */
    INDEX_DELETE_FAIL("Index_delete_fail"),

    /**
     * 不支持索引
     */
    UNSUPPORT("Unsupport"),

    /**
     * index_scan_response_error_label
     */
    INDEX_SCAN_RESPONSE_ERROR_LABEL("index_scan_response_error_label"),

    /**
     * index_response_error_label
     */
    INDEX_RESPONSE_ERROR_LABEL("index_response_error_label"),

    /**
     * index_copy_status_error_label
     */
    INDEX_COPY_STATUS_ERROR_LABEL("index_copy_status_error_label");


    private String status;

    CopyIndexStatus(String status) {
        this.status = status;
    }

    /**
     * 获取副本索引状态
     *
     * @return string
     */
    @JsonValue
    public String getIndexStaus() {
        return status;
    }
}
