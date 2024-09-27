package openbackup.system.base.sdk.anti.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * PM防勒索-更新租户列表请求
 *
 * @author j00619968
 * @since 2023-01-06
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class AntiRansomwareVstoreListReq {
    // 租户列表
    @NotNull
    @Size(max = 200)
    List<AntiRansomwareVstoreReq> vstores;
}
