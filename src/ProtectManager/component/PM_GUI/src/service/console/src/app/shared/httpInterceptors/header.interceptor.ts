import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import {
  HttpInterceptor,
  HttpRequest,
  HttpHandler,
  HttpEvent
} from '@angular/common/http';
import { I18NService } from '../services/i18n.service';
import { CookieService } from '../services';
import { get, includes, isEmpty, trim } from 'lodash';
import { DataMap } from '../consts/data-map.config';
import { CommonConsts, MultiCluster } from '../consts/common.const';

@Injectable()
export class HeaderInterceptor implements HttpInterceptor {
  constructor(
    private cookieService: CookieService,
    private i18n: I18NService
  ) {}

  intercept(
    req: HttpRequest<any>,
    next: HttpHandler
  ): Observable<HttpEvent<any>> {
    let currentCluster = this.cookieService.filterCluster
      ? this.cookieService.filterCluster
      : !isEmpty(
          JSON.parse(
            decodeURIComponent(this.cookieService.get('currentCluster'))
          )
        )
      ? JSON.parse(decodeURIComponent(this.cookieService.get('currentCluster')))
      : {
          clusterId: DataMap.Cluster_Type.local.value,
          clusterType: DataMap.Cluster_Type.local.value
        };

    const urls = [
        {
          url: '^/console/rest/v1/users/[0-9a-zA-Z]+$',
          method: 'get'
        },
        {
          url: '^/console/rest/v1/system/initConfig$',
          method: 'get'
        },
        {
          url: '^/console/rest/v1/infra/configmap/info+$',
          method: 'get'
        }
      ],
      isOk = urls.some(item => {
        return (
          new RegExp(item.url).test(req.url) &&
          item.method === req.method.toLowerCase()
        );
      });

    if (isOk) {
      currentCluster = {
        clusterId: DataMap.Cluster_Type.local.value,
        clusterType: DataMap.Cluster_Type.local.value
      };
    }

    let clusterId =
      req.headers.get('clusters-id') || currentCluster['clusterId'];
    let clusterType =
      req.headers.get('clusters-type') || currentCluster['clusterType'];
    let memberEsn = '';
    if (MultiCluster.isMulti && !isEmpty(req.headers.get('member-esn'))) {
      //为本地集群时，clustersType为3,clusterId为集群id,esn为节点esn
      if (currentCluster.clusterType == DataMap.Cluster_Type.local.value) {
        clusterType = '3';
      }
      memberEsn = req.headers.get('member-esn');
    }

    let headers = req.headers
      .set('opToken', this.cookieService.get('_OP_TOKEN_') || '')
      .set('clusters-id', trim(clusterId))
      .set('clusters-type', trim(clusterType))
      .set('op-hcs', '1.0')
      .set('cftk', get(window, 'parent.hcsData.cftk', ''))
      .set('X-Requested-With', 'XMLHttpRequest')
      .set('X-Language', this.cookieService.get('locale') || 'zh-cn')
      .set('region', this.cookieService.get('selectedRegion') || '')
      .set('ProjectName', get(window, 'parent.hcsData.ProjectName', ''))
      .set('AgencyId', get(window, 'parent.hcsData.AgencyId', ''));
    if (includes(req.url, '/v1.0/servers')) {
      headers = headers.set('X-Openstack-Nova-Api-Version', '1.33');
    }

    let dmeHeaders = req.headers
      .set('cftk', get(window, 'parent.dmeData.cftk', ''))
      .set('region', this.cookieService.get('selectedRegion') || '')
      .set('opToken', this.cookieService.get('_OP_TOKEN_') || '')
      .set('clusters-id', trim(clusterId))
      .set('clusters-type', trim(clusterType))
      .set('member-esn', trim(memberEsn));
    req = req.clone({
      headers:
        this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE
          ? headers
          : this.cookieService.get('userType') === CommonConsts.DME_USER_TYPE
          ? dmeHeaders
          : req.headers
              .set('opToken', this.cookieService.get('_OP_TOKEN_') || '')
              .set('clusters-id', trim(clusterId))
              .set('clusters-type', trim(clusterType))
              .set('member-esn', trim(memberEsn))
    });
    return next.handle(req);
  }
}
