package openbackup.access.framework.resource.client.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * PM防勒索-更新租户列表请求
 *
 * @since 2022-12-13
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class AntiRansomwareVstoreListReq {
    @NotNull
    @Size(max = 200)
    List<AntiRansomwareVstoreReq> vstores;
}
