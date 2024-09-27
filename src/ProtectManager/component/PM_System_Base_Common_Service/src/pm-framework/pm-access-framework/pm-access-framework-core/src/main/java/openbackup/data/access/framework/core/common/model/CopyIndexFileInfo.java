package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Global Search index file info
 *
 * @author l00347293
 * @since 2021-01-07
 **/
@Data
public class CopyIndexFileInfo {
    @JsonProperty("time")
    private String time;

    @JsonProperty("type")
    private String type;

    @JsonProperty("size")
    private String size;
}
