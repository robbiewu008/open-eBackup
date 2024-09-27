import { Injectable } from '@angular/core';
import {
  ActivatedRouteSnapshot,
  Resolve,
  Router,
  RouterStateSnapshot
} from '@angular/router';
import { CommonConsts, CookieService, DataMap, I18NService } from 'app/shared';
import { includes } from 'lodash';
import { Observable, Observer } from 'rxjs';
import { map } from 'rxjs/operators';
import { SystemApiService } from '..';

const CLOUD_BACKUP_DEPLOY_TYPES = [
  DataMap.Deploy_Type.cloudbackup2.value,
  DataMap.Deploy_Type.cloudbackup.value,
  DataMap.Deploy_Type.hyperdetect.value
] as const;

@Injectable({
  providedIn: 'root'
})
export class InitResolver implements Resolve<any> {
  constructor(
    private router: Router,
    private i18n: I18NService,
    private cookieService: CookieService,
    private systemApiService: SystemApiService
  ) {}

  resolve(
    _: ActivatedRouteSnapshot,
    state: RouterStateSnapshot
  ): Observable<any> | Promise<any> | any {
    const deployType = this.i18n.get('deploy_type');
    const isCloudBackup = includes(CLOUD_BACKUP_DEPLOY_TYPES, deployType);
    const isCyberEngine = deployType === DataMap.Deploy_Type.cyberengine.value;
    const isX3000 = deployType === DataMap.Deploy_Type.x3000.value;
    const isHcsUser =
      this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

    // 安全一体机 init 接口 404
    if (isCyberEngine || isHcsUser) {
      return new Observable<void>((observer: Observer<void>) => {
        this.skipInit().setLocalCluster();
        observer.next();
        observer.complete();
      });
    }
    return this.systemApiService.getInitConfigUsingGET({}).pipe(
      map(
        res => {
          this.cookieService.setIsCloudBackup(isCloudBackup);
          if (isCloudBackup) {
            this.skipInit().setLocalCluster();
            return res;
          }
          this.setInitedStatus(res.status);
          if (this.isNoInit(res.status)) {
            state.url === '/init' && this.router.navigate([this.router.url]);
            return res;
          }

          if (state.url === '/init') {
            return res;
          }

          this.router.navigate(['/init']);
          return res;
        },
        () => {
          this.skipInit();
        }
      )
    );
  }

  private isNoInit(status: number) {
    return status === DataMap.System_Init_Status.no.value;
  }

  private setInitedStatus(status: number) {
    this.cookieService.setIsInited(this.isNoInit(status));
    this.cookieService.setInitedStatus(status);
  }

  private skipInit() {
    this.setInitedStatus(DataMap.System_Init_Status.no.value);
    return this;
  }

  private setLocalCluster() {
    this.cookieService.set(
      'currentCluster',
      encodeURIComponent(
        JSON.stringify({
          clusterId: DataMap.Cluster_Type.local.value,
          clusterType: DataMap.Cluster_Type.local.value
        })
      )
    );
  }
}
