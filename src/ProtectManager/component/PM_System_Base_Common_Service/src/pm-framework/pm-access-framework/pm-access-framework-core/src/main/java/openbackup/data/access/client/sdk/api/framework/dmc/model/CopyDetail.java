package openbackup.data.access.client.sdk.api.framework.dmc.model;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.DiskInfo;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 副本详情
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-04-15
 */
@Data
@JsonInclude(JsonInclude.Include.NON_EMPTY)
public class CopyDetail {
    @JsonProperty("DISKIDLIST")
    List<DiskInfo> diskInfoList;

    @JsonProperty("NETWORKLIST")
    List<String> networkNameList;

    @JsonProperty("BACKUPLEVEL")
    Integer backupLevel;

    @JsonProperty("CREATETIME")
    Long createTime;

    @JsonProperty("EXPARAMETERS")
    JSONObject exParameters;

    @JsonProperty("DATASIZE")
    Long dateSize;

    @JsonProperty("STORAGEESN")
    String storageEsn;

    @JsonProperty("STORAGEPOOL")
    String storagePool;
}
