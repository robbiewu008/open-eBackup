import {
  HttpEvent,
  HttpHandler,
  HttpInterceptor,
  HttpRequest,
  HttpResponse
} from '@angular/common/http';
import { Injectable } from '@angular/core';
import { assign, each, find, includes, isEmpty, map } from 'lodash';
import { finalize, mergeMap, Observable, Observer } from 'rxjs';
import { RoleAuthApiService } from '../api/services';
import { DataMap } from '../consts';
import { I18NService } from '../services';

@Injectable()
export class PermissionInterceptor implements HttpInterceptor {
  constructor(
    private i18n: I18NService,
    private roleAuthApiService: RoleAuthApiService
  ) {}

  // 需要查询资源权限的接口
  RESOURCE_URL = [
    '/console/rest/v1/host-agent',
    '/console/rest/v2/resources',
    '/console/rest/v1/environments',
    '/console/rest/v1/virtual-resource',
    '/console/rest/v2/resource/group',
    '/console/rest/v1/slas',
    '/console/rest/v1/report',
    '/console/rest/v1/anti-ransomware/airgap/policies',
    '/console/rest/v1/livemount-policies',
    '/console/rest/v1/copies',
    '/console/rest/v1/anti-ransomware/policies',
    '/console/rest/v1/anti-ransomware/infected-copy/config'
  ];

  useIdUrl = [
    '/console/rest/v1/anti-ransomware/airgap/policies',
    '/console/rest/v1/anti-ransomware/policies'
  ];

  usePolicyIdUrl = ['/console/rest/v1/livemount-policies'];

  useResourceIdUrl = ['/console/rest/v1/anti-ransomware/infected-copy/config'];

  private _setRoleAuth(resources, res, url) {
    each(resources, item => {
      const temp: any = find(res, {
        resourceId: `${item[this._getTrackKey(url)]}`
      });
      assign(item, {
        resourceRoleAuth: temp?.resourceRoleAuth
      });
    });
  }

  private _getTrackKey(url: string): string {
    if (includes(this.usePolicyIdUrl, url)) {
      return 'policyId';
    }
    if (includes(this.useIdUrl, url)) {
      return 'id';
    }
    if (includes(this.useResourceIdUrl, url)) {
      return 'resourceId';
    }
    return 'uuid';
  }

  // 资源接口拦截器，查询资源对应的权限
  private _getResourcePermission(
    event,
    req: HttpRequest<unknown>
  ): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const supportRoleAuth = includes(
        [
          DataMap.Deploy_Type.a8000.value,
          DataMap.Deploy_Type.x3000.value,
          DataMap.Deploy_Type.x6000.value,
          DataMap.Deploy_Type.x8000.value,
          DataMap.Deploy_Type.x9000.value,
          DataMap.Deploy_Type.e6000.value,
          DataMap.Deploy_Type.decouple.value,
          DataMap.Deploy_Type.openServer.value
        ],
        this.i18n.get('deploy_type')
      );
      const resources = event.body?.records || event.body?.items;
      if (
        supportRoleAuth &&
        event instanceof HttpResponse &&
        includes(this.RESOURCE_URL, req.url) &&
        !isEmpty(resources)
      ) {
        this.roleAuthApiService
          .queryRoleAuthSetByResourceIds({
            resourceIds: map(resources, this._getTrackKey(req.url)),
            akOperationTips: false,
            akDoException: false,
            akLoading: false
          })
          .pipe(
            finalize(() => {
              observer.next(event);
              observer.complete();
            })
          )
          .subscribe(res => this._setRoleAuth(resources, res, req.url));
      } else {
        observer.next(event);
        observer.complete();
      }
    });
  }

  intercept(
    req: HttpRequest<unknown>,
    next: HttpHandler
  ): Observable<HttpEvent<unknown>> {
    return next
      .handle(req)
      .pipe(mergeMap(event => this._getResourcePermission(event, req)));
  }
}
