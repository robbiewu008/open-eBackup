import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  AntiRansomwareAirgapApiService,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import { forEach, isNull, map } from 'lodash';

@Component({
  selector: 'aui-device-detail',
  templateUrl: './device-detail.component.html',
  styleUrls: ['./device-detail.component.less']
})
export class DeviceDetailComponent implements OnInit {
  data;
  freque;
  optItems: any[];
  timePeriod = [];
  isOnline = false;
  portsArray = [];
  minTime = 0;
  maxTime;
  lines: any[] = [];
  linesArray = [];
  points: any[] = [];
  timeValue = '';
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private antiRansomwareAirgapApiService: AntiRansomwareAirgapApiService
  ) {}
  formGroup: FormGroup;

  formItems = [
    [
      {
        key: 'name',
        label: this.i18n.get('common_name_label'),
        content: ''
      },
      {
        key: 'esn',
        label: 'ESN',
        content: ''
      },
      {
        key: 'status',
        label: this.i18n.get('common_device_status_label'),
        content: ''
      }
    ],
    [
      {
        key: 'poicy',
        label: this.i18n.get('common_associated_airgap_policy_label'),
        content: '--'
      },
      {
        key: 'policyStatus',
        label: this.i18n.get('common_airgap_policy_status_label'),
        content: '--'
      },
      {
        key: 'abort',
        label: this.isCyberengine
          ? this.i18n.get('explore_linked_detection_label')
          : this.i18n.get('common_force_abort_replication_label'),
        content: '--'
      }
    ]
  ];

  ngOnInit(): void {
    this.getModalHeader();
    this.initLines();
    this.initConfig();
  }

  getModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  optCallback = () => {
    return this.optItems || [];
  };

  getSeconds(dateLong) {
    const hours = parseInt(dateLong.split(':')[0]);
    const minutes = parseInt(dateLong.split(':')[1]);
    return hours * 3600 + minutes * 60;
  }
  _addZero(val) {
    if (val < 10) {
      return `0${val}`;
    } else {
      return val;
    }
  }
  initLines() {
    this.minTime = 0;
    this.maxTime = 24 * 3600;

    for (let i = 0; i <= 12; i++) {
      this.points.push({
        value: i * 2 * 3600,
        label:
          this._addZero(i * 2) !== 24 ? `${this._addZero(i * 2)}:00` : '00:00',
        pointStyle: {
          width: 0
        }
      });
    }
  }
  initLinesData(data) {
    forEach(
      data.airGapDeviceInfo.airGapPolicyInfo?.airGapPolicyWindows,
      item => {
        if (item.endTime === '00:00') {
          item.endTime = '24:00';
        }
        this.lines.push({
          value: [
            this.getSeconds(item.startTime),
            this.getSeconds(item.endTime)
          ],
          tips: `${item.startTime} - ${item.endTime}`,
          style: {
            'background-color': '#6c92fa'
          }
        });
      }
    );
    this.lines = [...this.lines];
  }

  initConfig() {
    this.formItems[0][0].content = this.data.name;
    this.formItems[0][1].content = this.data.esn;
    this.formItems[0][2].content = this.data.linkStatus;
    if (!isNull(this.data?.airGapPolicyInfo)) {
      this.formItems[1][0].content = this.data.airGapPolicyInfo?.name;
      this.formItems[1][1].content = this.data.policyStatus;
      if (!this.isCyberengine) {
        this.formItems[1][2].content = this.dataMapService.getLabel(
          'airGapSwitchStatus',
          this.data?.forceStop
        );
        this.formItems[1].push({
          key: 'link_detect',
          label: this.i18n.get('explore_linked_detection_label'),
          content: this.dataMapService.getLabel(
            'airGapSwitchStatus',
            this.data?.linkedDetection
          )
        });
      } else {
        this.formItems[1][2].content = this.dataMapService.getLabel(
          'airGapSwitchStatus',
          this.data?.linkedDetection
        );
      }
    }

    this.antiRansomwareAirgapApiService
      .ShowDeviceDetail({ deviceId: this.data.id, memberEsn: this.data?.esn })
      .subscribe(res => {
        forEach(
          res.airGapDeviceInfo?.airGapPolicyInfo?.airGapPolicyWindows,
          item => {
            this.timePeriod.push(`${item.startTime} - ${item.endTime}`);
          }
        );
        this.portsArray = res.airGapDevicePortInfos;
        this.freque = res.airGapDeviceInfo.airGapPolicyInfo?.triggerWeekFreq;
        if (!isNull(res.airGapDeviceInfo.airGapPolicyInfo?.triggerWeekFreq)) {
          if (
            res.airGapDeviceInfo.airGapPolicyInfo?.triggerWeekFreq.split(',')
              .length === 7
          ) {
            this.freque = this.i18n.get('common_everyday_label');
          } else {
            this.freque = map(
              res.airGapDeviceInfo.airGapPolicyInfo?.triggerWeekFreq.split(','),
              v => {
                return this.dataMapService.getLabel('Days_Of_Week', v);
              }
            ).join(', ');
          }
        }

        this.initLinesData(res);
      });
  }
}
