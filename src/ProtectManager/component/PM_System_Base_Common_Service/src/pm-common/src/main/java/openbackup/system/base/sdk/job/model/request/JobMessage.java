package openbackup.system.base.sdk.job.model.request;

import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.annotation.JsonAlias;

import lombok.Data;

/**
 * Job Message
 *
 * @author l00272247
 * @since 2021-03-10
 */
@Data
public class JobMessage {
    private String topic;

    private JSONObject payload;

    private JSONObject traffic;

    private JSONArray abolish;

    @JsonAlias({"context", "in_context"})
    private boolean isInContext;
}
