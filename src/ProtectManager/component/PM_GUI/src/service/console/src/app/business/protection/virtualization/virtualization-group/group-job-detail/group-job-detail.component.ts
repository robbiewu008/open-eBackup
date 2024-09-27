import { DatePipe } from '@angular/common';
import {
  Component,
  Input,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService, ModalRef } from '@iux/live';
import {
  DataMap,
  DataMapService,
  I18NService,
  JobAPIService
} from 'app/shared';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import {
  chunk,
  cloneDeep,
  each,
  first,
  includes,
  mapValues,
  values
} from 'lodash';
import { Subject, Subscription, combineLatest, timer } from 'rxjs';
import { map, switchMap, takeUntil } from 'rxjs/operators';
import { BaseTableComponent } from '../../virtualization-base/base-table/base-table.component';

@Component({
  selector: 'aui-group-job-detail',
  templateUrl: './group-job-detail.component.html',
  styleUrls: ['./group-job-detail.component.less'],
  providers: [DatePipe]
})
export class GroupJobDetailComponent implements OnInit, OnDestroy {
  job;
  isSelectedResource;
  detailData;
  sysTime;
  dataMap = DataMap;
  timeZone = 'UTC+08:00';
  jobForms = {};
  _values = values;
  jobDestroy$ = new Subject();
  jobSubscription$ = new Subscription();

  baseTableComponent: BaseTableComponent;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private dataMapService: DataMapService,
    private jobApiService: JobAPIService,
    private systemTimeService: SystemTimeService,
    public modal: ModalRef,
    public messageService: MessageService
  ) {}
  ngOnDestroy(): void {
    this.jobDestroy$.next(true);
    this.jobDestroy$.complete();
  }

  ngOnInit(): void {
    if (this.isSelectedResource) {
      this.modal.setProperty({ lvHeader: this.headerTpl });
    }
    this.getJob();
  }

  optCallback = data => {
    return this.detailData.optItems || [];
  };

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }
  getJob() {
    if (this.jobSubscription$) {
      this.jobSubscription$.unsubscribe();
    }
    this.jobSubscription$ = timer(0, 5 * 1e3)
      .pipe(
        switchMap(index => {
          return combineLatest([
            this.systemTimeService.getSystemTime(!index),
            this.jobApiService
              .queryJobsUsingGET({
                jobId: this.job?.jobId,
                akLoading: !index
              })
              .pipe(
                map(res => {
                  return first(res.records);
                })
              )
          ]);
        }),
        takeUntil(this.jobDestroy$)
      )
      .subscribe(result => {
        this.timeZone = result[0].displayName;
        this.sysTime = new Date(
          `${result[0].time.replace(/-/g, '/')} ${result[0].displayName}`
        ).getTime();
        this.job = {
          ...cloneDeep(this.job),
          ...result[1]
        };
        this.initJobForms();
      });
  }

  initJobForms() {
    this.jobForms = {
      basicInfo: {
        title: this.i18n.get('common_basic_info_label'),
        keys: ['jobId', 'status', 'startTime', 'endTime', 'durationTime'],
        values: []
      },
      targetInfo: {
        title: this.i18n.get('insight_job_target_object_label'),
        keys: ['sourceName', 'sourceSubType'],
        values: []
      }
    };

    for (const key in this.jobForms) {
      const array = this.jobForms[key];
      each(array.keys, prop => {
        this.jobForms[key]['values'].push({
          key: prop,
          value: this.getValue(prop, this.job[prop]),
          label: this.i18n.get(`insight_job_${prop.toLowerCase()}_label`)
        });
      });
      array.values = chunk(array.values, key === 'targetInfo' ? 2 : 5);
    }
  }

  getValue(key, value) {
    switch (key) {
      case 'startTime':
      case 'endTime':
        value = this.datePipe.transform(
          value,
          'yyyy/MM/dd HH:mm:ss',
          this.timeZone
        );
        break;
      case 'sourceSubType':
        value = this.dataMapService.getLabel('Job_Target_Type', value);
        break;
      case 'durationTime':
        value = this.job.getDuration(
          includes(
            [
              DataMap.Job_status.running.value,
              DataMap.Job_status.initialization.value,
              DataMap.Job_status.pending.value,
              DataMap.Job_status.aborting.value
            ],
            this.job.status
          )
            ? this.sysTime - this.job.startTime < 0
              ? 0
              : this.sysTime - this.job.startTime
            : this.job.endTime
            ? this.job.endTime - this.job.startTime
            : 0
        );
        break;
      default:
        break;
    }
    return value;
  }
}
