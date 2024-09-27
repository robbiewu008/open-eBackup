import { Component, OnDestroy, ViewChild } from '@angular/core';
import {
  CommonConsts,
  DataMap,
  I18NService,
  JobAPIService,
  ProtectedResourceApiService,
  SupportLicense
} from 'app/shared';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { first, includes } from 'lodash';
import { map } from 'rxjs';
import { LocalFileSystemComponent } from '../local-file-system/local-file-system.component';
import { LocalLunComponent } from '../local-lun/local-lun.component';

@Component({
  selector: 'aui-local-resource',
  templateUrl: './local-resource.component.html',
  styleUrls: ['./local-resource.component.less']
})
export class LocalResourceComponent implements OnDestroy {
  activeIndex = 'file';
  oceanStorDoradoV6Uuid;
  scanJobTimeout;
  supportLicense = SupportLicense;

  @ViewChild(LocalFileSystemComponent, { static: false })
  LocalFileSystemComponent: LocalFileSystemComponent;
  @ViewChild(LocalLunComponent, { static: false })
  LocalLunComponent: LocalLunComponent;

  constructor(
    private i18n: I18NService,
    private jobApiService: JobAPIService,
    private infoMessageService: InfoMessageService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnDestroy(): void {
    clearTimeout(this.scanJobTimeout);
  }

  ngOnInit() {
    this.getOceanStorDoradoV6();
    if (SupportLicense.isFile) {
      this.activeIndex = 'file';
    } else {
      this.activeIndex = 'lun';
    }
  }

  queryRescanJob(akLoading = false) {
    clearTimeout(this.scanJobTimeout);
    this.jobApiService
      .queryJobsUsingGET({
        sourceId: this.oceanStorDoradoV6Uuid,
        akDoException: false,
        akLoading
      })
      .subscribe(res => {
        const job = first(res.records);
        if (
          job &&
          includes(
            [
              DataMap.Job_status.running.value,
              DataMap.Job_status.pending.value,
              DataMap.Job_status.initialization.value
            ],
            job.status
          )
        ) {
          this.scanJobTimeout = setTimeout(() => {
            this.queryRescanJob();
          }, 1e3);
        } else {
          if (this.activeIndex === 'file') {
            this.LocalFileSystemComponent?.DoradoFileSystemComponent?.dataTable?.fetchData(
              { isAutoPolling: true }
            );
          } else {
            this.LocalLunComponent?.dataTable?.fetchData({
              isAutoPolling: true
            });
          }
        }
      });
  }

  rescan() {
    this.infoMessageService.create({
      content: this.i18n.get('protection_rescan_info_label'),
      onOK: () => {
        this.protectedResourceApiService
          .ScanProtectedResources({
            resId: this.oceanStorDoradoV6Uuid
          })
          .subscribe(() => this.queryRescanJob());
      }
    });
  }

  getOceanStorDoradoV6() {
    return this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType: 'OceanStorDoradoV6'
        })
      })
      .pipe(
        map(res => {
          return first(res.records)?.uuid;
        })
      )
      .subscribe(uuid => {
        this.oceanStorDoradoV6Uuid = uuid;
      });
  }
}
