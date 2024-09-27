package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Global Search index base message body
 *
 * @author l00347293
 * @since 2021-01-12
 **/
@Data
public class SearchBaseMsg {
    @JsonProperty("request_id")
    private String requestId;

    @JsonProperty("default_publish_topic")
    private String defaultPublishTopic;

    @JsonProperty("response_topic")
    private String responseTopic;
}
