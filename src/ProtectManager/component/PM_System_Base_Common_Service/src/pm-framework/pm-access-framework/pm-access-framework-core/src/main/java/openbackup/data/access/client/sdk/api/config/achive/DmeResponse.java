/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.config.achive;

import openbackup.system.base.common.exception.LegoCheckedException;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.Optional;

/**
 * DME Response
 *
 * @param <T> template type
 * @author l00272247
 * @since 2020-04-03
 */
@Getter
@Setter
public class DmeResponse<T> {
    @JsonProperty("Data")
    private T data;

    @JsonProperty("Error")
    private DmeResponseError error;

    /**
     * get checked data
     *
     * @return checked data
     */
    @JsonIgnore
    public T getCheckedData() {
        checkData();
        return data;
    }

    /**
     * check data
     */
    public void checkData() {
        Optional<LegoCheckedException> exception = getExceptionIfError();
        if (exception.isPresent()) {
            throw exception.get();
        }
    }

    /**
     * get exception if error
     *
     * @return exception
     */
    public Optional<LegoCheckedException> getExceptionIfError() {
        if (error != null && error.getCode() > 0) {
            return Optional.of(new LegoCheckedException(error.getCode(), error.getDetailParams()));
        }
        return Optional.empty();
    }
}
