import { Component, OnInit } from '@angular/core';
import { IODETECTFILESYSTEMService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-real-time-detection',
  templateUrl: './real-time-detection.component.html',
  styleUrls: ['./real-time-detection.component.less']
})
export class RealTimeDetectionComponent implements OnInit {
  activeIndex;
  totalFileSystem = 0;
  protectedFileSystem = 0;
  totalPolicy = 0;
  totalWhitelist = 0;

  constructor(
    private appUtilsService: AppUtilsService,
    private ioDetectFilesystemService: IODETECTFILESYSTEMService
  ) {}

  ngOnInit(): void {
    if (this.appUtilsService.getCacheValue('ioDetectionTab')) {
      this.activeIndex = 'policy';
    }
    this.getSummary();
  }

  getSummary(loading = true) {
    this.ioDetectFilesystemService
      .getIoDetectConfigSummary({ akLoading: loading })
      .subscribe(res => {
        this.totalFileSystem = res.fsNum;
        this.protectedFileSystem = res.ioDetectEnabledNum;
        this.totalPolicy = res.policyNum;
        this.totalWhitelist = res.whitelistNum;
      });
  }

  refreshSummary() {
    this.getSummary(false);
  }
}
