package openbackup.system.base.config.datasource;

import openbackup.system.base.common.model.storage.StorageError;
import openbackup.system.base.common.utils.JSONArray;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 基础设施返回值
 *
 * @author y30000858
 * @since 2021-01-22
 */
@Data
public class DataSourceRes {
    @JsonProperty("data")
    private JSONArray data;

    @JsonProperty("error")
    private StorageError error;
}
