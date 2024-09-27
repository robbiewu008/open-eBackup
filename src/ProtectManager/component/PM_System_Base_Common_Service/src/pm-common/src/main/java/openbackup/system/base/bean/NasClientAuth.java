package openbackup.system.base.bean;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Nas共享客户端下AUTH
 *
 * @author fwx1022842
 * @version [BCManager 8.0.0]
 * @since 2021-06-11
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class NasClientAuth {
    /**
     * ID
     */
    @JsonProperty("ID")
    private String id;

    /**
     * 权限（0：只读，1：读写）1
     */
    @JsonProperty("ACCESSVAL")
    private String accessVal;

    /**
     * 权限限制（0：all_squash，1：no_all_squash ）1
     */
    @JsonProperty("ALLSQUASH")
    private String allSquash;

    /**
     * 客户端IP或主机名或网络组名
     * 参数长度：1~25600
     */
    @JsonProperty("NAME")
    private String name;

    /**
     * root权限限制(0：root_squash，1：no_root_squash)1
     */
    @JsonProperty("ROOTSQUASH")
    private String rootSquash;

    /**
     * 写入模式（0：同步，1：异步）0
     */
    @JsonProperty("SYNC")
    private String sync;

    /**
     * 父对象ID
     */
    @JsonProperty("PARENTID")
    private String parentId;
}
