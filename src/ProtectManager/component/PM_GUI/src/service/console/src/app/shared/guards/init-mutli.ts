import { Injectable } from '@angular/core';
import {
  ActivatedRouteSnapshot,
  Resolve,
  RouterStateSnapshot
} from '@angular/router';
import { BackupClustersApiService } from '../api/services/backup-clusters-api.service';
import { CommonConsts, DataMap, MultiCluster } from '../consts';
import { finalize, tap } from 'rxjs/operators';
import { filter, includes, isEmpty, isUndefined, size } from 'lodash';
import { I18NService } from '../services/i18n.service';
import { CookieService } from '../services/cookie.service';
import { Observable, Observer } from 'rxjs';
import { ClustersApiService } from '../api/services';

@Injectable({
  providedIn: 'root'
})
export class InitMutli implements Resolve<any> {
  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    public clusterApiService: ClustersApiService,
    public backupClustersApiService: BackupClustersApiService
  ) {}

  resolve(route: ActivatedRouteSnapshot, state: RouterStateSnapshot) {
    if (
      includes(
        [
          DataMap.Deploy_Type.cyberengine.value,
          DataMap.Deploy_Type.hyperdetect.value,
          DataMap.Deploy_Type.cloudbackup.value,
          DataMap.Deploy_Type.cloudbackup2.value,
          DataMap.Deploy_Type.decouple.value,
          DataMap.Deploy_Type.e6000.value,
          DataMap.Deploy_Type.openServer.value
        ],
        this.i18n.get('deploy_type')
      ) ||
      MultiCluster.isInit
    ) {
      return new Observable<void>((observer: Observer<void>) => {
        observer.next();
        observer.complete();
      });
    }
    return new Observable<void>((observer: Observer<void>) => {
      this.backupClustersApiService
        .queryClusterInfo({ akLoading: false, akDoException: false })
        .pipe(
          finalize(() => {
            observer.next();
            observer.complete();
          })
        )
        .subscribe(res => {
          MultiCluster.isInit = true;
          MultiCluster.esn = res.esn || '';
          if (!isUndefined(res.clusterEstablished)) {
            MultiCluster.isMulti = res.clusterEstablished;
            MultiCluster.roleType = res.roleType;
          } else {
            MultiCluster.isMulti = !isEmpty(res.roleType);
          }
        });
    });
  }
}
