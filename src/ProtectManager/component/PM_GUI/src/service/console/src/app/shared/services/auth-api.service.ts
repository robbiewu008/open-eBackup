/* tslint:disable */
import { Injectable } from '@angular/core';
import {
  HttpClient,
  HttpRequest,
  HttpResponse,
  HttpHeaders
} from '@angular/common/http';
import { BaseService as __BaseService } from '../api/base-service';
import { ApiConfiguration as __Configuration } from '../api/api-configuration';
import { StrictHttpResponse as __StrictHttpResponse } from '../api/strict-http-response';
import { Observable as __Observable } from 'rxjs';
import { map as __map, filter as __filter } from 'rxjs/operators';
import { LogoutType } from '../consts/common.const';

/* tslint:disable */
export interface AuthRequest {
  password: string;
  userName: string;
}

/* tslint:disable */
export interface LoginResponseBody {
  /**
   * 密码过期剩余天数
   */
  expireDay?: number;

  /**
   * 是否需要修改密码
   */
  modifyPassword?: boolean;

  /**
   * token
   */
  sessionId?: string;

  /**
   * 用户ID
   */
  userId?: string;

  /**
   * 上次登录IP
   */
  lastLoginIp?: string;

  /**
   * 上次登录时间
   */
  lastLoginTime?: string;

  /**
   * 上次登录时区
   */
  lastLoginZone?: string;

  /**
   * 服务类型，用于标识，为空则为普通备份一体机 值：HCS(op服务化)
   */
  serviceProduct?: string;
}

/**
 * Auth Controller
 */
@Injectable({
  providedIn: 'root'
})
class AuthApiService extends __BaseService {
  static readonly loginUsingPOSTPath = '/v1/auth/action/login';
  static readonly logoutUsingPOSTPath = '/v1/auth/action/logout';

  constructor(config: __Configuration, http: HttpClient) {
    super(config, http);
  }

  /**
   * @param params The `AuthApiService.LoginUsingPOSTParams` containing the following parameters:
   *
   * - `authRequest`: authRequest
   *
   * - `akTimeout`: HttpInterceptor requires parameter: akTimeout
   *
   * - `akPrefix`: HttpInterceptor requires parameter: akPrefix
   *
   * - `akOperationTipsContent`: HttpInterceptor requires parameter: akOperationTipsContent
   *
   * - `akOperationTips`: HttpInterceptor requires parameter: akOperationTips
   *
   * - `akLoading`: HttpInterceptor requires parameter: akLoading
   *
   * - `akEscapeSession`: HttpInterceptor requires parameter: akEscapeSession
   *
   * - `akDoException`: HttpInterceptor requires parameter: akDoException
   *
   * @return OK
   */
  loginUsingPOSTResponse(
    params: AuthApiService.LoginUsingPOSTParams
  ): __Observable<__StrictHttpResponse<LoginResponseBody>> {
    let __params = this.newParams();
    let __headers = new HttpHeaders();
    let __body: any = null;
    __body = params.authRequest;
    if (params.akTimeout != null)
      __params = __params.set('akTimeout', params.akTimeout.toString());
    if (params.akPrefix != null)
      __params = __params.set('akPrefix', params.akPrefix.toString());
    if (params.akOperationTipsContent != null)
      __params = __params.set(
        'akOperationTipsContent',
        params.akOperationTipsContent.toString()
      );
    if (params.akOperationTips != null)
      __params = __params.set(
        'akOperationTips',
        params.akOperationTips.toString()
      );
    if (params.akLoading != null)
      __params = __params.set('akLoading', params.akLoading.toString());
    if (params.akEscapeSession != null)
      __params = __params.set(
        'akEscapeSession',
        params.akEscapeSession.toString()
      );
    if (params.akDoException != null)
      __params = __params.set('akDoException', params.akDoException.toString());
    if (params.clustersType != null)
      __headers = __headers.set(
        'clusters-type',
        params.clustersType.toString()
      );
    if (params.clustersId != null)
      __headers = __headers.set('clusters-id', params.clustersId.toString());
    let req = new HttpRequest<any>(
      'POST',
      this.rootUrl + `/v1/auth/action/login`,
      __body,
      {
        headers: __headers,
        params: __params,
        responseType: 'json'
      }
    );

    return this.http.request<any>(req).pipe(
      __filter(_r => _r instanceof HttpResponse),
      __map(_r => {
        return _r as __StrictHttpResponse<LoginResponseBody>;
      })
    );
  }
  /**
   * @param params The `AuthApiService.LoginUsingPOSTParams` containing the following parameters:
   *
   * - `authRequest`: authRequest
   *
   * - `akTimeout`: HttpInterceptor requires parameter: akTimeout
   *
   * - `akPrefix`: HttpInterceptor requires parameter: akPrefix
   *
   * - `akOperationTipsContent`: HttpInterceptor requires parameter: akOperationTipsContent
   *
   * - `akOperationTips`: HttpInterceptor requires parameter: akOperationTips
   *
   * - `akLoading`: HttpInterceptor requires parameter: akLoading
   *
   * - `akEscapeSession`: HttpInterceptor requires parameter: akEscapeSession
   *
   * - `akDoException`: HttpInterceptor requires parameter: akDoException
   *
   * @return OK
   */
  loginUsingPOST(
    params: AuthApiService.LoginUsingPOSTParams
  ): __Observable<LoginResponseBody> {
    return this.loginUsingPOSTResponse(params).pipe(
      __map(_r => _r.body as LoginResponseBody)
    );
  }

  /**
   * @param params The `AuthApiService.LogoutUsingPOSTParams` containing the following parameters:
   *
   * - `userName`: 用户名
   *
   * - `akTimeout`: HttpInterceptor requires parameter: akTimeout
   *
   * - `akPrefix`: HttpInterceptor requires parameter: akPrefix
   *
   * - `akOperationTipsContent`: HttpInterceptor requires parameter: akOperationTipsContent
   *
   * - `akOperationTips`: HttpInterceptor requires parameter: akOperationTips
   *
   * - `akLoading`: HttpInterceptor requires parameter: akLoading
   *
   * - `akEscapeSession`: HttpInterceptor requires parameter: akEscapeSession
   *
   * - `akDoException`: HttpInterceptor requires parameter: akDoException
   *
   * @return OK
   */
  logoutUsingPOSTResponse(
    params: AuthApiService.LogoutUsingPOSTParams
  ): __Observable<__StrictHttpResponse<boolean>> {
    let __params = this.newParams();
    let __headers = new HttpHeaders();
    let __body: any = null;
    if (params.logoutType != null)
      __params = __params.set('logoutType', params.logoutType.toString());
    if (params.akTimeout != null)
      __params = __params.set('akTimeout', params.akTimeout.toString());
    if (params.akPrefix != null)
      __params = __params.set('akPrefix', params.akPrefix.toString());
    if (params.akOperationTipsContent != null)
      __params = __params.set(
        'akOperationTipsContent',
        params.akOperationTipsContent.toString()
      );
    if (params.akOperationTips != null)
      __params = __params.set(
        'akOperationTips',
        params.akOperationTips.toString()
      );
    if (params.akLoading != null)
      __params = __params.set('akLoading', params.akLoading.toString());
    if (params.akEscapeSession != null)
      __params = __params.set(
        'akEscapeSession',
        params.akEscapeSession.toString()
      );
    if (params.akDoException != null)
      __params = __params.set('akDoException', params.akDoException.toString());
    if (params.clustersType != null)
      __headers = __headers.set(
        'clusters-type',
        params.clustersType.toString()
      );
    if (params.clustersId != null)
      __headers = __headers.set('clusters-id', params.clustersId.toString());
    let req = new HttpRequest<any>(
      'POST',
      this.rootUrl + `/v1/auth/action/logout`,
      __body,
      {
        headers: __headers,
        params: __params,
        responseType: 'text'
      }
    );

    return this.http.request<any>(req).pipe(
      __filter(_r => _r instanceof HttpResponse),
      __map(_r => {
        return (_r as HttpResponse<any>).clone({
          body: (_r as HttpResponse<any>).body === 'true'
        }) as __StrictHttpResponse<boolean>;
      })
    );
  }
  /**
   * @param params The `AuthApiService.LogoutUsingPOSTParams` containing the following parameters:
   *
   * - `userName`: 用户名
   *
   * - `akTimeout`: HttpInterceptor requires parameter: akTimeout
   *
   * - `akPrefix`: HttpInterceptor requires parameter: akPrefix
   *
   * - `akOperationTipsContent`: HttpInterceptor requires parameter: akOperationTipsContent
   *
   * - `akOperationTips`: HttpInterceptor requires parameter: akOperationTips
   *
   * - `akLoading`: HttpInterceptor requires parameter: akLoading
   *
   * - `akEscapeSession`: HttpInterceptor requires parameter: akEscapeSession
   *
   * - `akDoException`: HttpInterceptor requires parameter: akDoException
   *
   * @return OK
   */
  logoutUsingPOST(
    params: AuthApiService.LogoutUsingPOSTParams
  ): __Observable<boolean> {
    return this.logoutUsingPOSTResponse(params).pipe(
      __map(_r => _r.body as boolean)
    );
  }
}

module AuthApiService {
  /**
   * Parameters for loginUsingPOST
   */
  export interface LoginUsingPOSTParams {
    /**
     * authRequest
     */
    authRequest: AuthRequest;

    /**
     * HttpInterceptor requires parameter: clusters-type
     */
    clustersType?: string;

    /**
     * HttpInterceptor requires parameter: clusters-id
     */
    clustersId?: string;

    /**
     * HttpInterceptor requires parameter: akTimeout
     */
    akTimeout?: number;

    /**
     * HttpInterceptor requires parameter: akPrefix
     */
    akPrefix?: string;

    /**
     * HttpInterceptor requires parameter: akOperationTipsContent
     */
    akOperationTipsContent?: string;

    /**
     * HttpInterceptor requires parameter: akOperationTips
     */
    akOperationTips?: boolean;

    /**
     * HttpInterceptor requires parameter: akLoading
     */
    akLoading?: boolean;

    /**
     * HttpInterceptor requires parameter: akEscapeSession
     */
    akEscapeSession?: boolean;

    /**
     * HttpInterceptor requires parameter: akDoException
     */
    akDoException?: boolean;
  }

  /**
   * Parameters for logoutUsingPOST
   */
  export interface LogoutUsingPOSTParams {
    /**
     * HttpInterceptor requires parameter: clusters-type
     */
    clustersType?: string;

    /**
     * HttpInterceptor requires parameter: clusters-id
     */
    clustersId?: string;

    /**
     * logoutType, support manual or timeout
     *
     * the value is MANUAL or TIMEOUT
     */
    logoutType: LogoutType.Manual | LogoutType.Timeout;

    /**
     * HttpInterceptor requires parameter: akTimeout
     */
    akTimeout?: number;

    /**
     * HttpInterceptor requires parameter: akPrefix
     */
    akPrefix?: string;

    /**
     * HttpInterceptor requires parameter: akOperationTipsContent
     */
    akOperationTipsContent?: string;

    /**
     * HttpInterceptor requires parameter: akOperationTips
     */
    akOperationTips?: boolean;

    /**
     * HttpInterceptor requires parameter: akLoading
     */
    akLoading?: boolean;

    /**
     * HttpInterceptor requires parameter: akEscapeSession
     */
    akEscapeSession?: boolean;

    /**
     * HttpInterceptor requires parameter: akDoException
     */
    akDoException?: boolean;
  }
}

export { AuthApiService };
