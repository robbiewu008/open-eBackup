import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SystemTimeService } from 'app/shared/services/system-time.service';

@Component({
  selector: 'aui-current-system-time',
  templateUrl: './current-system-time.component.html',
  styleUrls: ['./current-system-time.component.less']
})
export class CurrentSystemTimeComponent implements OnInit, OnDestroy {
  @Input() cluster;
  @Input() needLoading: boolean;
  date;
  timeoutSysTime;
  sysTimeLong;

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private systemTimeService: SystemTimeService
  ) {}

  ngOnDestroy() {
    clearTimeout(this.timeoutSysTime);
  }

  ngOnInit() {
    this.getDate();
  }

  getAutoTime(displayName) {
    clearTimeout(this.timeoutSysTime);
    this.timeoutSysTime = setTimeout(() => {
      this.sysTimeLong += 1e3;
      this.date = this.i18n.get('common_current_device_time_label', [
        `${this.appUtilsService.convertDateLongToString(
          this.sysTimeLong
        )} ${displayName}`
      ]);
      this.getAutoTime(displayName);
    }, 1e3);
  }

  getDate() {
    this.systemTimeService
      .getSystemTime(this.needLoading ?? false, this.cluster)
      .subscribe(res => {
        this.date = this.i18n.get('common_current_device_time_label', [
          `${res.time} ${res.displayName}`
        ]);
        this.getAutoTime(res.displayName);
        this.sysTimeLong = new Date(res.time).getTime();
      });
  }
}
