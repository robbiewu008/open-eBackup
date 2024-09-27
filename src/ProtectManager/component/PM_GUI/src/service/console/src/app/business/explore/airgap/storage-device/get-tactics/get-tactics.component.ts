import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageboxService } from '@iux/live';
import {
  AntiRansomwareAirgapApiService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  MODAL_COMMON
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, isNumber } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-get-tactics',
  templateUrl: './get-tactics.component.html',
  styleUrls: ['./get-tactics.component.less']
})
export class GetTacticsComponent implements OnInit {
  isModify;
  data;
  formGroup: FormGroup;
  tacticsOptions = [];
  portOptions = [];
  bindPort = [];
  isForceStop = true;
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isDataBackup = this.appUtilsService.isDataBackup;

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private messageBox: MessageboxService,
    private appUtilsService: AppUtilsService,
    private antiRansomwareAirgapApiService: AntiRansomwareAirgapApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.updateData();
    this.getPort(this.data);
    this.getAirGapPolicy();
  }

  updateData() {
    if (!this.isModify) {
      return;
    }
    const params = { port: [] };
    assign(params, { isForceStop: this.data.forceStop });
    assign(params, { isLinkedDetection: this.data.linkedDetection });
    this.antiRansomwareAirgapApiService
      .ShowDevicePorts({
        deviceType: this.data.deviceType,
        deviceId: this.data.id,
        memberEsn: this.data?.esn
      })
      .subscribe(res => {
        let ports = [];
        each(res.airGapDevicePortInfos, item => {
          if (item.deviceId === this.data.id) {
            ports.push(item.portId);
          }
        });
        params.port = ports;
        assign(params, {
          tactics: this.data.airGapPolicyInfo.id
        });

        this.formGroup.patchValue(params, { emitEvent: false });
      });
  }

  getAirGapPolicy(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE
    };

    this.antiRansomwareAirgapApiService
      .ShowPagePolicies(params as any)
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          const policys = [];
          each(recordsTemp, item => {
            policys.push({
              ...item,
              key: item.id,
              value: item.id,
              label: item.name,
              isLeaf: true
            });
          });
          this.tacticsOptions = policys;
          return;
        }
        this.getAirGapPolicy(recordsTemp, startPage);
      });
  }

  getPort(data) {
    this.antiRansomwareAirgapApiService
      .ShowDevicePorts({
        deviceType: data.deviceType,
        deviceId: data.id,
        memberEsn: data?.esn
      })
      .subscribe(res => {
        const ports = [];
        each(res.airGapDevicePortInfos, item => {
          ports.push({
            ...item,
            key: item.portId,
            value: item.portId,
            label: item.name,
            isLeaf: true
          });
        });
        this.portOptions = ports;
        if (!this.isModify) {
          let params = {
            port: []
          };

          each(this.portOptions, item => {
            params.port.push(item.value);
          });
          this.formGroup.patchValue(params);
        }
      });
  }

  getParams() {
    let forceStop = true;
    if (!this.isCyberengine) {
      forceStop = this.formGroup.value.isForceStop;
    }
    const params = {
      deviceId: this.data.id,
      policyId: this.formGroup.value.tactics,
      isForceStop: forceStop,
      isLinkedDetection: false,
      deviceType: this.data.deviceType,
      airGapDevicePortInfos: []
    };
    if (this.isCyberengine || this.isDataBackup) {
      assign(params, {
        isLinkedDetection: this.formGroup.value.isLinkedDetection
      });
    }
    each(this.formGroup.value.port, item => {
      each(this.portOptions, port => {
        if (port.portId === item) {
          params.airGapDevicePortInfos.push({
            portId: port.portId,
            name: port.name,
            status: port.status,
            ip: port.ip,
            deviceId: port.deviceId
          });
        }
      });
    });
    return params;
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      let params = this.getParams();
      if (!this.isModify) {
        this.antiRansomwareAirgapApiService
          .CreateDevicePolicyRelation({
            CreateDevicePolicyRelationRequestBody: params,
            memberEsn: this.data?.esn
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        this.antiRansomwareAirgapApiService
          .UpdateDevicePolicyRelation({
            UpdateDevicePolicyRelationRequestBody: params,
            memberEsn: this.data?.esn
          })
          .subscribe(
            res => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      }
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      tactics: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      port: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      isForceStop: new FormControl(this.isForceStop),
      isLinkedDetection: new FormControl(true)
    });

    this.formGroup.get('isLinkedDetection').valueChanges.subscribe(res => {
      if (res) {
        this.messageBox.confirm({
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvDialogIcon: 'lv-icon-popup-info-48',
          lvContent: this.i18n.get('explore_linked_detection_turn_on_label'),
          lvWidth: MODAL_COMMON.normalWidth,
          lvOk: modal => {
            modal.close();
          },
          lvCancel: modal => {
            this.formGroup.get('isLinkedDetection').setValue(false);
          }
        });
      }
    });
  }
}
