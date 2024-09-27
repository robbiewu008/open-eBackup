package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Global Search index done message body
 *
 * @author l00347293
 * @since 2021-01-07
 **/
@Data
public class CopyIndexDoneMsg extends CopyIndexBaseMsg {
    @JsonProperty("index_type")
    private String indexType;

    @JsonProperty("previous_indexed_gn")
    private int previousIndexedGn = 0;
}
