/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model.request;

import lombok.Getter;
import lombok.Setter;

/**
 * 取值为domain时，表示获取的Token可以访问指定账号下所有资源，domain支持id和name
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class Project {
    private String id;

    private String name;
}
