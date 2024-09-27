import { DatePipe } from '@angular/common';
import { Component, Input, OnInit } from '@angular/core';
import {
  CommonConsts,
  CopiesDetectReportService,
  CopiesService,
  DataMap,
  ProtectedResourceApiService
} from 'app/shared';
import { ExportFilesService } from 'app/shared/components/export-files/export-files.component';
import { I18NService } from 'app/shared/services/i18n.service';
import {
  eq,
  find,
  first,
  includes,
  isEmpty,
  isNil,
  last,
  remove
} from 'lodash';
import { DataMapService } from 'app/shared/services/data-map.service';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { MessageService } from '@iux/live';
import { CopyActionService } from 'app/shared/services/copy-action.service';

@Component({
  selector: 'alarms-details',
  templateUrl: './alarms-details.component.html',
  styleUrls: ['./alarms-details.component.css'],
  providers: [DatePipe]
})
export class AlarmsDetailsComponent implements OnInit {
  severity;
  formItems = [];
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );

  @Input() data;
  @Input() isAlarm;

  constructor(
    public i18n: I18NService,
    private dataMapService: DataMapService,
    private datePipe: DatePipe,
    private exportFilesService: ExportFilesService,
    private appUtilsService: AppUtilsService,
    private messageService: MessageService,
    private copiesApiService: CopiesService,
    private copyActionService: CopyActionService,
    private copiesDetectReportService: CopiesDetectReportService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  id = this.i18n.get('insight_alarm_id_label');
  object = this.i18n.get('insight_alarm_object_label');
  occurred = this.i18n.get('insight_alarm_occurred_label');
  severityLabel = this.i18n.get('insight_alarm_severity_label');
  suggestions = this.i18n.get('insight_event_suggestions_label');
  sequenceNo = this.i18n.get('insight_event_squence_no_label');
  clearTime = this.i18n.get('insight_event_recovered_time_label');
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isV1Alarm =
    this.appUtilsService.isDecouple || this.appUtilsService.isDistributed;

  ngOnInit() {
    const severityTypes = this.dataMapService.toArray('Alarm_Severity_Type');
    const severityType = find(severityTypes, item => {
      return item.value === this.data.severity;
    });
    this.severity = severityType ? severityType.key : '';
    this.formItems = [
      {
        label: this.i18n.get('common_name_label'),
        value: this.isAlarm
          ? this.i18n.get(
              this.isCyberEngine || this.isV1Alarm
                ? this.data.alarmName
                : this.data.name,
              this.data.params || [],
              false,
              true
            )
          : this.i18n.get(
              this.data.alarmName,
              this.data.params || [],
              false,
              true
            )
      },
      {
        label: this.id,
        value: this.data.alarmId
      },
      {
        label: this.occurred,
        value: this.isAlarm
          ? this.getAlarmTimeStr(this.data.alarmTime)
          : this.data.alarmTimeStr
      },
      {
        label: this.i18n.get('protection_equipment_name_label'),
        value: this.data?.deviceName ?? '--'
      },
      {
        label: this.i18n.get('common_type_label'),
        value: this.i18n.get(`common_alarms_type_${this.data.type}_label`)
      },
      {
        label: this.i18n.get('system_servers_label'),
        value: this.data.nodeName
      },
      {
        label: this.sequenceNo,
        value: this.data.sequence =
          this.data.sequence === -1 ? '--' : this.data.sequence
      },
      {
        label: this.i18n.get('common_desc_label'),
        key: 'desc',
        value: this.i18n.encodeHtml(
          this.isAlarm
            ? this.i18n.get(
                this.isCyberEngine || this.isV1Alarm
                  ? this.data.desc
                  : this.data.detail,
                this.data.params || [],
                false,
                true
              )
            : this.i18n.get(this.data.desc, this.data.params || [], false, true)
        )
      },
      {
        label: this.suggestions,
        key: 'suggestions',
        value: this.data.type === 6 ? '--' : this.getSuggestion()
      }
    ];

    this.protectedResourceApiService
      .ListResources({
        conditions: JSON.stringify({ uuid: [['~~'], this.data.params[1]] })
      })
      .subscribe(({ records }) => {
        if (records.length) {
          this.data.detectType = records[0].extendInfo.detectType;
        }
      });

    if (!this.isDataBackup) {
      remove(this.formItems, col =>
        eq(col.label, this.i18n.get('system_servers_label'))
      );
    }

    if (!this.isCyberEngine) {
      remove(this.formItems, item =>
        eq(item.label, this.i18n.get('protection_equipment_name_label'))
      );
    }
    const param = {
      label: this.i18n.get('common_status_label'),
      value: this.i18n.get(
        this.isAlarm
          ? `common_alarms_status_${
              this.isCyberEngine || this.isV1Alarm
                ? this.data.confirmStatus
                : this.data.status
            }_label`
          : `common_alarms_status_${this.data.confirmStatus}_label`
      )
    };
    this.formItems.splice(5, 0, param);
    if (this.data.type !== 0 && this.data.type !== 6) {
      this.formItems.splice(3, 0, {
        label: this.clearTime,
        value: this.data.clearTimeStr
      });
    }
  }

  downloadReport() {
    const params = { alarmEntityId: this.data?.entityId || this.data?.entity };
    this.exportFilesService.create({
      data: { params, type: DataMap.Export_Query_Type.detectionReport.value }
    });
  }

  viewDetectReport() {
    this.copiesApiService
      .queryResourcesV1CopiesGet({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          generated_time: last(this.data?.params),
          resource_id: this.data.params[5]
        })
      })
      .subscribe(res => {
        if (isEmpty(res.items)) {
          this.messageService.error(this.i18n.get('1677936407'), {
            lvShowCloseButton: true,
            lvMessageKey: 'emptyCopyMesageKey'
          });
          return;
        }
        this.copiesDetectReportService
          .ShowDetectionDetails({
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE,
            resourceId: this.data.params[5],
            conditions: JSON.stringify({
              uuid: res.items[0].uuid
            })
          })
          .subscribe(snapshot => {
            this.copyActionService.detectionReport(first(snapshot.items));
          });
      });
  }

  private getSuggestion() {
    if (!this.isAlarm || this.isCyberEngine || this.isV1Alarm) {
      return !isEmpty(this.data.advice) && 'null' !== this.data.advice
        ? this.replaceAdvice(
            this.i18n.encodeHtml(
              this.i18n.get(
                this.data.advice,
                this.data.params || [],
                false,
                true
              )
            )
          )
        : '--';
    }
    return !isNil(this.data?.suggestion)
      ? this.i18n.get(this.data.suggestion.split('\n').join('</br>'))
      : '--';
  }

  private replaceAdvice(advice) {
    if (Object.prototype.toString.call(advice) === '[object String]') {
      return advice.replace(/\n/g, '</br>');
    }
    return advice;
  }

  private getAlarmTimeStr(timestamp, isSeconds = true) {
    if (this.isCyberEngine || this.isV1Alarm) {
      return this.data?.alarmTimeStr;
    }
    if (isSeconds) timestamp = timestamp * 1000;
    return this.datePipe.transform(timestamp, 'yyyy-MM-dd HH:mm:ss');
  }
}
