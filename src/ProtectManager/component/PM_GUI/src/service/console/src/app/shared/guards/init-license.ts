import { Injectable } from '@angular/core';
import {
  ActivatedRouteSnapshot,
  Resolve,
  RouterStateSnapshot
} from '@angular/router';
import { BackupClustersApiService } from '../api/services/backup-clusters-api.service';
import { CommonConsts, DataMap, MultiCluster, SupportLicense } from '../consts';
import { finalize, tap } from 'rxjs/operators';
import {
  filter,
  find,
  includes,
  isEmpty,
  isUndefined,
  map,
  size
} from 'lodash';
import { I18NService } from '../services/i18n.service';
import { CookieService } from '../services/cookie.service';
import { Observable, Observer } from 'rxjs';
import { LicenseApiService } from '../api/services';

@Injectable({
  providedIn: 'root'
})
export class InitLicense implements Resolve<any> {
  constructor(
    private i18n: I18NService,
    private licenseApiService: LicenseApiService,
    public backupClustersApiService: BackupClustersApiService
  ) {}

  resolve(route: ActivatedRouteSnapshot, state: RouterStateSnapshot) {
    if (
      !includes(
        [DataMap.Deploy_Type.hyperdetect.value],
        this.i18n.get('deploy_type')
      ) ||
      SupportLicense.isInit
    ) {
      return new Observable<void>((observer: Observer<void>) => {
        observer.next();
        observer.complete();
      });
    }
    return new Observable<void>((observer: Observer<void>) => {
      this.licenseApiService
        .queryLicenseUsingGET({ akLoading: false, akDoException: false })
        .pipe(
          finalize(() => {
            observer.next();
            observer.complete();
          })
        )
        .subscribe(res => {
          SupportLicense.isInit = true;
          SupportLicense.isFile = !!find(
            res,
            item => item.name === 'HyperDetect (anti-Ransomware)'
          );
          SupportLicense.isSan = !!find(
            res,
            item => item.name === 'HyperDetect for SAN'
          );
          SupportLicense.isBoth = SupportLicense.isFile && SupportLicense.isSan;
        });
    });
  }
}
