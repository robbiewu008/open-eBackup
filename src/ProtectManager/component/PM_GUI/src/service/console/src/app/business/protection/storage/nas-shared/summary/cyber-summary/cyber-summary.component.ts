import { DatePipe } from '@angular/common';
import { Component, Input, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import {
  CommonConsts,
  CopiesDetectReportService,
  DataMap,
  I18NService,
  IODETECTFILESYSTEMService,
  IODETECTPOLICYService,
  ProjectedObjectApiService,
  RouterUrl
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { find, isEmpty } from 'lodash';
import { Observable, Observer, combineLatest } from 'rxjs';

@Component({
  selector: 'aui-cyber-summary',
  templateUrl: './cyber-summary.component.html',
  styleUrls: ['./cyber-summary.component.less'],
  providers: [DatePipe]
})
export class CyberSummaryComponent implements OnInit {
  @Input() source;
  _isEn = this.i18n.isEn;

  detectionStatus;
  ioDetectPolicy;
  detectEnabled;
  isHoneypotDetectEnable;

  earliestTime;
  latestTime;
  nextTime;

  totalSnapshot = 0;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private datePipe: DatePipe,
    private appUtilsService: AppUtilsService,
    private ioDetectPolicyService: IODETECTPOLICYService,
    private detectFilesystemService: IODETECTFILESYSTEMService,
    private copiesDetectReportService: CopiesDetectReportService,
    private projectedObjectApiService: ProjectedObjectApiService
  ) {}

  ngOnInit() {
    this.getCopyInfo();
    this.getInfectedCopy();
    this.getDetectFilesystem();
  }

  getDetectFilesystem() {
    this.detectFilesystemService
      .pageQueryProtectObject({
        pageNum: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        name: this.source?.name
      })
      .subscribe(res => {
        const fs = find(
          res.records,
          item =>
            item.fsName === this.source?.name &&
            item.deviceName === this.source?.environment?.name &&
            item.vstoreName === this.source?.extendInfo?.tenantName
        );
        if (fs) {
          this.ioDetectPolicy = fs.policyName;
          this.detectEnabled = fs.isIoDetectEnabled;
        }
        this.getIoPolicy(fs.policyId);
      });
  }

  getIoPolicy(policyId) {
    if (!policyId) {
      return;
    }
    this.ioDetectPolicyService
      .getIoDetectPolicyById({ policyId })
      .subscribe(res => {
        this.isHoneypotDetectEnable = res.isHoneypotDetectEnable;
      });
  }

  gotoIoDetectPolicy() {
    this.appUtilsService.setCacheValue('ioDetectionTab', 'policy');
    this.appUtilsService.setCacheValue(
      'ioDetectionPolicyName',
      this.ioDetectPolicy
    );
    this.router.navigateByUrl(
      RouterUrl.ExploreAntiRansomwareProtectionRealTimeDetection
    );
  }

  gotoCopyDetectPolicy() {
    this.appUtilsService.setCacheValue('copyDetectionTab', 'backupPolicy');
    this.appUtilsService.setCacheValue(
      'copyDetectionPolicyName',
      this.source?.sla_name
    );
    this.router.navigateByUrl(
      RouterUrl.ExploreAntiRansomwareProtectionDataBackup
    );
  }

  getInfectedCopy() {
    this.copiesDetectReportService
      .ShowDetectionStatistics({
        pageNo: 0,
        pageSize: 10,
        resourceSubType: DataMap.Resource_Type.LocalFileSystem.value,
        conditions: JSON.stringify({ resource_id: this.source.uuid })
      })
      .subscribe(res => {
        if (!isEmpty(res.items)) {
          this.detectionStatus = (res.items[0] as any).status;
        }
      });
  }

  getAllCopies() {
    return this.copiesDetectReportService.ShowDetectionDetails({
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START,
      resourceId: this.source.uuid
    });
  }

  getSyncTime() {
    return new Observable<void>((observer: Observer<any>) => {
      if (this.source.sla_id) {
        this.projectedObjectApiService
          .queryProtectionTimeV1ProtectedObjectsResourceIdBackupTimeGet({
            resourceId: this.source.uuid
          })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete();
            },
            err => {
              observer.next({});
              observer.complete();
            }
          );
      } else {
        observer.next({});
        observer.complete();
      }
    });
  }

  getCopyInfo() {
    const combined: any = combineLatest([
      this.getAllCopies(),
      this.getSyncTime()
    ]);
    combined.subscribe(resArr => {
      const [all, syncTime] = resArr;
      this.totalSnapshot = all.total;
      this.earliestTime = syncTime.earliest_time
        ? this.datePipe.transform(
            new Date(syncTime.earliest_time),
            'yyyy-MM-dd HH:mm:ss'
          )
        : '';
      this.latestTime = syncTime.latest_time
        ? this.datePipe.transform(
            new Date(syncTime.latest_time),
            'yyyy-MM-dd HH:mm:ss'
          )
        : '';
      this.nextTime = !this.source.sla_status
        ? ''
        : syncTime.next_time
        ? this.datePipe.transform(
            new Date(syncTime.next_time),
            'yyyy-MM-dd HH:mm:ss'
          )
        : '';
    });
  }
}
