/*
 * This file is a part of the open-eBackup project.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) [2024] Huawei Technologies Co.,Ltd.
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */
import {
  ChangeDetectorRef,
  Component,
  OnInit,
  Pipe,
  PipeTransform,
  ViewChild
} from '@angular/core';
import {
  FormGroup,
  FormBuilder,
  FormArray,
  FormControl,
  ValidatorFn,
  AbstractControl,
  Validators
} from '@angular/forms';
import { DatatableComponent, ModalRef } from '@iux/live';
import {
  AgentLanFreeControllerService,
  AgentLanFreeAixControllerService,
  CommonConsts,
  DataMapService,
  BaseUtilService,
  DataMap,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  filter,
  find,
  includes,
  isEmpty,
  map,
  omit,
  reject,
  size,
  each,
  toLower,
  uniqueId,
  forEach,
  defer,
  isUndefined
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Pipe({ name: 'selecLanfreeTable' })
export class SelectionPipe implements PipeTransform {
  transform(value: any[], exponent: string = 'isEidt') {
    return filter(value, item => !item[exponent]);
  }
}

@Component({
  selector: 'aui-lan-free',
  templateUrl: './lan-free.component.html',
  styleUrls: ['./lan-free.component.less']
})
export class LanFreeComponent implements OnInit {
  data: any;
  enableLanFree = true;
  fcEnable = false;
  wwpnData: any = [];
  sanclientWwpnData = [];
  selectionWwpn = [];
  _find = find;
  tableConfig: TableConfig;
  tableData: TableData;
  selectionPort = [];
  isWwpnValid = true;
  iswwpnValid = true;
  wwpnValidLable: string;
  WwpnValidLable: string;
  formGroup: FormGroup;
  sanclientChosen = {} as any;
  addSanclientW = true;
  addSanclientw = true;

  wwpnaddble = true;
  sanclientWwpnaddble = true;
  isIqnManual = false;

  value3 = 'FC';
  value4 = 'ISCSI';
  dataMap = DataMap;
  proxyOptions = [];
  sanclientWwpnOptions = [];
  clientWwpnOptions = [];
  wwpnOptions = [];
  sanclientIqnOptions = [];
  fcportOptions = [];
  agentLanFreeDTO: any;
  agentLanFreeAixDTO: any;
  nodeOptions = [];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  fcportTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidLength: this.i18n.get('common_valid_maxlength_label', [4])
  };

  sanclientTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidLength: this.i18n.get('common_valid_maxlength_label', [4])
  };

  inputTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidLength: this.i18n.get('protection_wwpn_num_info_label')
  };

  iqnTip = {
    ...this.baseUtilService.requiredErrorTip
  };

  @ViewChild('lvTable', { static: false }) lvTable: DatatableComponent;

  constructor(
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private cdr: ChangeDetectorRef,
    private modal: ModalRef,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private agentLanFreeService: AgentLanFreeControllerService,
    private fb: FormBuilder,
    private protectedResourceApiService: ProtectedResourceApiService,
    private agentLanFreeAixService: AgentLanFreeAixControllerService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initForm();
    if (this.data.osType !== this.dataMap.Os_Type.aix.value) {
      this.disableBtn();
      this.getNode();
    } else {
      this.getLanFree();
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      esn: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      sanclient: new FormControl([], {
        validators: [
          this.validLength(4),
          this.baseUtilService.VALID.required()
        ],
        updateOn: 'change'
      }),
      protocol: new FormControl(this.value3),
      fcport: new FormControl([]),
      clientWwpn: new FormControl([], {
        validators: [this.validLength(0)],
        updateOn: 'change'
      }),
      clientIqn: new FormControl(''),
      sanclientWwpn: new FormControl([], {
        validators: this.validLength(2),
        updateOn: 'change'
      }),
      sanclientWwpnInput: new FormControl([]),
      sanclientwwpn: new FormControl([]),
      sanclientwwpnInput: new FormControl([]),
      sanclientIqn: new FormControl([])
    });
    if (this.data.osType === this.dataMap.Os_Type.aix.value) {
      this.formGroup.get('sanclientWwpn').clearValidators();
      this.formGroup.get('esn').clearValidators();
    } else if (
      this.data.subType === this.dataMap.Resource_Type.SBackupAgent.value
    ) {
      this.formGroup.get('sanclient').clearValidators();
      this.formGroup.get('clientWwpn').clearValidators();
    }

    this.watchForm();
  }

  validLength(type): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      const value = control.value;
      if (type === 0) {
        if (this.onlineNum(this.clientWwpnOptions, value) > 4) {
          return { invalidLength: { value: control.value } };
        } else if (this.onlineNum(this.clientWwpnOptions, value) === 0) {
          return { required: { value: control.value } };
        }
      } else if (type === 1) {
        if (this.onlineNum(this.fcportOptions, value) > 4) {
          return { invalidLength: { value: control.value } };
        } else if (this.onlineNum(this.fcportOptions, value) === 0) {
          return { required: { value: control.value } };
        }
      } else if (type === 2) {
        if (
          this.onlineNum(this.sanclientWwpnOptions, value) > 4 ||
          (this.onlineNum(this.sanclientWwpnOptions, value) > 3 &&
            !this.addSanclientW)
        ) {
          this.sanclientWwpnaddble = false;
          return { invalidLength: { value: control.value } };
        } else if (this.onlineNum(this.sanclientWwpnOptions, value) > 3) {
          this.sanclientWwpnaddble = false;
        } else if (
          this.onlineNum(this.sanclientWwpnOptions, value) === 0 &&
          (!this.isWwpnValid ||
            this.addSanclientW ||
            this.formGroup.value.sanclientWwpnInput?.length === 0)
        ) {
          this.sanclientWwpnaddble = true;
          return { required: { value: control.value } };
        } else {
          this.sanclientWwpnaddble = true;
        }
      } else if (type === 4) {
        if (this.onlineNum(this.proxyOptions, value) > 4) {
          return { invalidLength: { value: control.value } };
        } else if (this.onlineNum(this.proxyOptions, value) === 0) {
          return { required: { value: control.value } };
        }
      } else {
        if (
          this.onlineNum(this.wwpnOptions, value) > 4 ||
          (this.onlineNum(this.wwpnOptions, value) > 3 && !this.addSanclientw)
        ) {
          this.wwpnaddble = false;
          return { invalidLength: { value: control.value } };
        } else if (this.onlineNum(this.wwpnOptions, value) > 3) {
          this.wwpnaddble = false;
        } else if (
          this.onlineNum(this.wwpnOptions, value) === 0 &&
          (!this.iswwpnValid ||
            this.addSanclientw ||
            this.formGroup.value.sanclientwwpnInput?.length === 0)
        ) {
          this.wwpnaddble = true;
          return { required: { value: control.value } };
        } else {
          this.wwpnaddble = true;
        }
      }
    };
  }

  watchForm() {
    this.formGroup.statusChanges.subscribe(res => {
      defer(() => {
        this.disableBtn();
      });
    });
    this.formGroup.get('protocol').valueChanges.subscribe(res => {
      if (res === this.value3) {
        this.formGroup.get('clientIqn').disable();
        this.formGroup.get('clientWwpn').enable();
      } else {
        this.formGroup.get('clientWwpn').disable();
        this.formGroup.get('clientIqn').enable();
        this.formGroup
          .get('clientIqn')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      defer(() => {
        this.disableBtn();
      });
    });
  }

  addSanclientWwpn() {
    this.formGroup.get('sanclientWwpnInput').enable();
    this.addSanclientW = false;
    this.formGroup.get('sanclientWwpn').updateValueAndValidity();
    this.disableBtn();
    this.formGroup.get('sanclientWwpnInput').valueChanges.subscribe(res => {
      if (this.addSanclientW === true) {
        return;
      }
      if (res.length === 0) {
        this.isWwpnValid = true;
      } else {
        this.isWwpnValid = this.validWwpn(res, 0);
      }
      this.formGroup.get('sanclientWwpn').updateValueAndValidity();
      this.disableBtn();
    });
  }

  deleteSanclientWwpn() {
    this.formGroup.get('sanclientWwpnInput').setValue([]);
    this.addSanclientW = true;
    this.isWwpnValid = true;
    this.formGroup.get('sanclientWwpnInput').disable();
    this.disableBtn();
  }

  addSanclientwwpn() {
    this.formGroup.get('sanclientwwpnInput').enable();
    this.addSanclientw = false;
    this.formGroup.get('sanclientwwpn').updateValueAndValidity();
    this.disableBtn();
    this.formGroup.get('sanclientwwpnInput').valueChanges.subscribe(res => {
      if (this.addSanclientw === true) {
        return;
      }
      if (res.length === 0) {
        this.iswwpnValid = false;
      } else {
        this.iswwpnValid = this.validWwpn(res, 1);
      }
      this.formGroup.get('sanclientwwpn').updateValueAndValidity();
      this.disableBtn();
    });
  }

  deleteSanclientwwpn() {
    this.formGroup.get('sanclientwwpnInput').setValue([]);
    this.addSanclientw = true;
    this.iswwpnValid = true;
    this.formGroup.get('sanclientwwpnInput').disable();
    this.disableBtn();
  }

  initConfig() {
    this.tableConfig = {
      table: {
        async: false,
        compareWith: 'id',
        columns: [
          {
            key: 'location',
            name: this.i18n.get('common_location_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'runningStatus',
            name: this.i18n.get('protection_running_status_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('lanFreeRunningStatus')
            }
          },
          {
            key: 'wwpn',
            name: 'WWPN',
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          }
        ],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        selectionChange: data => {
          this.selectionPort = data;
          this.disableBtn();
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  onFocus() {
    if (this.data.subType === this.dataMap.Resource_Type.SBackupAgent.value) {
      if (!this.isIqnManual) {
        this.formGroup.get('sanclientIqn').setValue('');
      }
    } else {
      if (!this.isIqnManual) {
        this.formGroup.get('clientIqn').setValue('');
      }
    }
    this.isIqnManual = true;
  }

  getIqn() {
    this.agentLanFreeService
      .querySanClientIqnUsingGET({ agentId: this.data?.uuid })
      .subscribe(res => {
        if (!!res.length) {
          this.formGroup.get('sanclientIqn').setValue(res[0]);
          this.isIqnManual = false;
        }
      });
  }

  getSanclient(clientWwpns) {
    const clientWwpnArray = [];
    each(clientWwpns, item => {
      clientWwpnArray.push({
        ...item,
        key: item.wwpn,
        value: item.wwpn,
        label: item.wwpn,
        isLeaf: true,
        disabled:
          item.runningStatus !== DataMap.clickHouse_node_status.normal.value
      });
    });
    this.clientWwpnOptions = clientWwpnArray;
    if (!isEmpty(this.formGroup.value.clientWwpn)) {
      this.formGroup
        .get('clientWwpn')
        .setValue(
          filter(this.formGroup.value.clientWwpn, item =>
            includes(map(clientWwpnArray, 'value'), item)
          )
        );
    }

    const extparmas = {
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        type: 'Host',
        subType: [DataMap.Resource_Type.SBackupAgent.value],
        isCluster: [['=='], false]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extparmas,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          if (
            item.subType === DataMap.Host_Proxy_Type.SBackupAgent.value &&
            item.extendInfo.isAddLanFree === '1'
          ) {
            hostArray.push({
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: `${item.name}(${item.endpoint})`,
              isLeaf: true,
              disabled:
                item.linkStatus !==
                DataMap.resource_LinkStatus_Special.normal.value
            });
          }
        });
        this.proxyOptions = hostArray;
        if (!isEmpty(this.formGroup.value.sanclient)) {
          this.formGroup
            .get('sanclient')
            .setValue(
              filter(this.formGroup.value.sanclient, item =>
                includes(map(hostArray, 'value'), item)
              )
            );
        }
      }
    );
  }
  getNode() {
    this.agentLanFreeService
      .queryClusterNodeInfoGET({ agentId: this.data?.uuid })
      .subscribe(res => {
        this.nodeOptions = map(res, item => {
          return {
            isLeaf: true,
            label: item.nodeName,
            value: item.esn,
            ...item
          };
        });
        const node = find(res, { chosen: true });
        if (node) {
          this.formGroup.get('esn').setValue(node.esn);
        }
      });
  }
  nodeChange(val) {
    if (this.data.subType === this.dataMap.Resource_Type.SBackupAgent.value) {
      this.formGroup.get('sanclientWwpn').setValue([]);
      this.formGroup.get('sanclientIqn').setValue([]);
      this.sanclientWwpnOptions = [];
      this.fcportOptions = [];
      this.wwpnOptions = [];
      if (this.fcEnable) {
        this.formGroup.get('sanclientwwpn').setValue([]);
        this.formGroup.get('fcport').setValue([]);
        this.fcEnable = false;
        this.fcChange();
      } else {
        this.formGroup.get('sanclientwwpn').enable();
        this.formGroup.get('fcport').enable();
        this.formGroup.get('sanclientwwpn').setValue([]);
        this.formGroup.get('fcport').setValue([]);
        this.fcChange();
      }
      if (!this.addSanclientw) {
        this.deleteSanclientwwpn();
      }
      if (!this.addSanclientW) {
        this.deleteSanclientWwpn();
      }
    } else {
      this.selectionPort = [];
      this.selectionWwpn = [];
      this.lvTable.clearSelection();
      this.dataTable.setSelections([]);
      this.cdr.detectChanges();
    }
    this.getLanFree(val);
  }

  fcChange(e?) {
    if (!this.fcEnable) {
      this.formGroup.get('sanclientwwpn').disable();
      this.formGroup.get('fcport').disable();
    } else {
      this.formGroup.get('sanclientwwpn').enable();
      this.formGroup.get('fcport').enable();
      this.formGroup.get('sanclientwwpn').setValidators([this.validLength(3)]);
      this.formGroup
        .get('fcport')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1),
          this.validLength(1)
        ]);
      this.formGroup.get('sanclientwwpn').updateValueAndValidity();
      this.formGroup.get('fcport').updateValueAndValidity();
    }
    this.disableBtn();
  }

  getParams(list) {
    const params = [];
    const resultList = [];
    const chosen = [];
    each(list, item => {
      resultList.push({
        ...item,
        key: item.wwpn,
        value: item.wwpn,
        label: item.wwpn,
        isLeaf: true,
        disabled:
          item.runningStatus !== DataMap.clickHouse_node_status.normal.value
      });
      if (item.chosen === true) {
        chosen.push(item.wwpn);
      }
    });
    params.push(resultList);
    params.push(chosen);
    return params;
  }

  getLanFree(esn?) {
    if (this.data.osType === this.dataMap.Os_Type.aix.value) {
      this.agentLanFreeAixService
        .configurationLanFreeAixUsingGET({
          agentId: this.data?.uuid
        })
        .subscribe(res => {
          const resInfo = res;
          this.enableLanFree = !resInfo.delete;
          this.formGroup.get('protocol').setValue(resInfo.dataProtocol);
          if (resInfo.clientIqns) {
            this.formGroup.get('clientIqn').setValue(resInfo.clientIqns[0]);
          }
          if (resInfo.sanclientResourceIds) {
            this.formGroup
              .get('sanclient')
              .setValue(resInfo?.sanclientResourceIds);
          }
          let tmpwwpn = [];
          let wwpn = filter(resInfo.clientWwpns, item => {
            return item.chosen === true;
          });
          each(wwpn, item => {
            tmpwwpn.push(item.wwpn);
          });
          this.formGroup.get('clientWwpn').setValue(tmpwwpn);
          this.getSanclient(resInfo.clientWwpns);
          this.disableBtn();
        });
    } else {
      const params = { agentId: this.data?.uuid, memberEsn: esn };
      if (this.appUtilsService.isDecouple) {
        assign(params, { targetEsn: this.formGroup.value.esn });
      }
      this.agentLanFreeService
        .configurationLanFreeGET(params)
        .subscribe((res: any) => {
          this.enableLanFree = !res.delete;
          this.wwpnData = res.wwpns;
          if (res.sanClientWwpns) {
            const fcArray = [];
            let chosenFc = [];
            if (res.sanClientIqns.length !== 0) {
              this.formGroup.get('sanclientIqn').setValue(res.sanClientIqns[0]);
            }
            if (res.fcPorts.length !== 0) {
              each(res.fcPorts, item => {
                fcArray.push({
                  ...item,
                  key: item.id,
                  value: item.location,
                  label: `${item.location}(${item.wwpn})`,
                  isLeaf: true,
                  disabled:
                    item.runningStatus !==
                    DataMap.lanFreeRunningStatus.connected.value
                });
                if (item.chosen === true) {
                  chosenFc.push(item.location);
                  this.fcEnable = true;
                  this.fcChange();
                }
              });
              this.fcportOptions = fcArray;
              this.formGroup.get('fcport').setValue(chosenFc);
            }
            const sanclientwwpnData = this.getParams(res.wwpns);
            this.wwpnOptions = sanclientwwpnData[0];
            this.formGroup.get('sanclientwwpn').setValue(sanclientwwpnData[1]);
            const sanclientData = this.getParams(res.sanClientWwpns);
            this.sanclientWwpnOptions = sanclientData[0];
            this.formGroup.get('sanclientWwpn').setValue(sanclientData[1]);
            this.disableBtn();
          } else {
            this.selectionWwpn = filter(res.wwpns, item => item.chosen);
            this.tableData = {
              data: res.fcPorts,
              total: size(res.fcPorts)
            };
            setTimeout(() => {
              this.selectionPort = filter(res.fcPorts, item => item.chosen);
              this.dataTable.setSelections(this.selectionPort);
              this.disableBtn();
            });
          }
        });
    }
  }

  ngModelChange() {
    this.disableBtn();
  }

  selectionChange() {
    this.disableBtn();
  }

  onlineNum(list, formValue) {
    let num = 0;
    forEach(list, item => {
      forEach(formValue, tmp => {
        if (
          tmp === item.wwpn &&
          item.runningStatus === DataMap.clickHouse_node_status.normal.value
        ) {
          num++;
        }
        if (
          tmp === item.location &&
          item.runningStatus === DataMap.lanFreeRunningStatus.connected.value
        ) {
          num++;
        }
        if (
          tmp === item.uuid &&
          item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
        ) {
          num++;
        }
      });
    });
    return num;
  }

  disableBtn() {
    if (this.data.subType === this.dataMap.Resource_Type.SBackupAgent.value) {
      if (
        (this.formGroup.valid &&
          (this.isWwpnValid || this.addSanclientW) &&
          (this.iswwpnValid || this.addSanclientw)) ||
        !this.enableLanFree
      ) {
        this.modal.getInstance().lvOkDisabled = false;
      } else {
        this.modal.getInstance().lvOkDisabled = true;
      }
    } else if (this.data.osType === this.dataMap.Os_Type.aix.value) {
      if (this.formGroup.valid || !this.enableLanFree) {
        this.modal.getInstance().lvOkDisabled = false;
      } else {
        this.modal.getInstance().lvOkDisabled = true;
      }
    } else {
      this.modal.getInstance().lvOkDisabled =
        this.enableLanFree &&
        !(
          !isEmpty(this.selectionWwpn) &&
          size(this.selectionWwpn) <= 2 &&
          isEmpty(find(this.selectionWwpn, { isEidt: true })) &&
          !isEmpty(this.selectionPort) &&
          size(this.selectionPort) <= 4
        );
    }
  }

  isWwpnChecked(item): boolean {
    return !isEmpty(find(this.selectionWwpn, { wwpn: item.wwpn }));
  }

  validWwpn(wwpn, type?): boolean {
    if (!wwpn) {
      return;
    }
    const reg = /^[0-9A-Fa-f]{16}$/;
    if (
      find(
        this.wwpnData,
        item => toLower(item.wwpn) === toLower(wwpn) && !item.isEidt
      ) &&
      this.data.subType !== this.dataMap.Resource_Type.SBackupAgent.value
    ) {
      this.wwpnValidLable = this.i18n.get('protection_wwpn_exist_label');
      return false;
    }
    if (
      type === 0 &&
      find(
        this.sanclientWwpnOptions,
        item => toLower(item.wwpn) === toLower(wwpn)
      ) &&
      this.data.subType === this.dataMap.Resource_Type.SBackupAgent.value
    ) {
      this.wwpnValidLable = this.i18n.get('protection_wwpn_exist_label');
      return false;
    }
    if (
      type === 1 &&
      find(this.wwpnOptions, item => toLower(item.wwpn) === toLower(wwpn)) &&
      this.data.subType === this.dataMap.Resource_Type.SBackupAgent.value
    ) {
      this.WwpnValidLable = this.i18n.get('protection_wwpn_exist_label');
      return false;
    }
    if (
      !reg.test(wwpn) ||
      includes(
        ['0000000000000000', 'ffffffffffffffff', 'FFFFFFFFFFFFFFFF'],
        wwpn
      )
    ) {
      if (type === 1) {
        this.WwpnValidLable = this.i18n.get('common_incorrect_format_label');
      } else {
        this.wwpnValidLable = this.i18n.get('common_incorrect_format_label');
      }
      return false;
    }
    return true;
  }

  wwpnChange(wwpn) {
    this.isWwpnValid = this.validWwpn(wwpn);
  }

  addWwpn() {
    const id = uniqueId();
    this.wwpnData.push({
      id,
      manualAdd: true,
      isEidt: true,
      wwpn: ''
    });
    this.wwpnData = [...this.wwpnData];
  }

  deleteWwpn(item) {
    this.wwpnData = reject(this.wwpnData, data => {
      return data.wwpn === item.wwpn && data.manualAdd;
    });
    this.selectionWwpn = reject(this.selectionWwpn, data => {
      return data.wwpn === item.wwpn && data.manualAdd;
    });
    this.isWwpnValid = true;
    this.disableBtn();
    this.wwpnData = [...this.wwpnData];
  }

  modifyWwpn(item) {
    assign(item, {
      isEidt: true
    });
    this.disableBtn();
  }

  saveWwpn(item) {
    if (!this.isWwpnValid) {
      return;
    }
    assign(item, {
      isEidt: false
    });
    this.isWwpnValid = true;
    this.disableBtn();
    this.wwpnData = [...this.wwpnData];
  }

  getWwpnParams(wwpnChosen?, wwpnList?) {
    const params = [];
    each(wwpnChosen, item => {
      let tmp: any = filter(wwpnList, temp => {
        return temp.value === item;
      });
      if (
        !tmp[0].location &&
        tmp[0].runningStatus === DataMap.clickHouse_node_status.normal.value
      ) {
        params.push({
          chosen: true,
          manualAdd: tmp[0].manualAdd,
          runningStatus: tmp[0].runningStatus,
          wwpn: tmp[0].wwpn
        });
      } else if (
        tmp[0].runningStatus === DataMap.lanFreeRunningStatus.connected.value
      ) {
        params.push({
          chosen: true,
          id: tmp[0].id,
          location: tmp[0].location,
          runningStatus: tmp[0].runningStatus,
          wwpn: tmp[0].wwpn
        });
      }
    });
    return params;
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.data.osType === this.dataMap.Os_Type.aix.value) {
        const sanclientArray = [];
        each(this.proxyOptions, item => {
          map(this.formGroup.value.sanclient, tmp => {
            if (
              tmp === item.uuid &&
              item.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
            ) {
              sanclientArray.push(tmp);
            }
          });
        });
        this.agentLanFreeAixDTO = {
          delete: !this.enableLanFree,
          dataProtocol: this.formGroup.value.protocol,
          sanclientResourceIds: sanclientArray,
          clientIqns:
            this.formGroup.value.protocol === this.value3
              ? []
              : this.isIqnManual
              ? [`Manual.${this.formGroup.value.clientIqn}`]
              : [this.formGroup.value.clientIqn],
          clientWwpns: this.getWwpnParams(
            this.formGroup.value.clientWwpn,
            this.clientWwpnOptions
          )
        };
      } else if (
        this.data.subType === this.dataMap.Resource_Type.SBackupAgent.value
      ) {
        const wwpn = this.getWwpnParams(
          this.formGroup.value.sanclientwwpn,
          this.wwpnOptions
        );
        const Wwpn = this.getWwpnParams(
          this.formGroup.value.sanclientWwpn,
          this.sanclientWwpnOptions
        );
        if (
          this.fcEnable &&
          this.iswwpnValid &&
          !!this.formGroup.value.sanclientwwpnInput?.length
        ) {
          wwpn.push({
            wwpn: this.formGroup.value.sanclientwwpnInput,
            manualAdd: true,
            chosen: true,
            runningStatus: DataMap.clickHouse_node_status.normal.value
          });
        }
        if (
          !!this.formGroup.value.sanclientWwpnInput?.length &&
          this.isWwpnValid
        ) {
          Wwpn.push({
            wwpn: this.formGroup.value.sanclientWwpnInput,
            manualAdd: true,
            chosen: true,
            runningStatus: DataMap.clickHouse_node_status.normal.value
          });
        }

        this.agentLanFreeDTO = {
          delete: !this.enableLanFree,
          fcPorts: this.fcEnable
            ? this.getWwpnParams(
                this.formGroup.value.fcport,
                this.fcportOptions
              )
            : [],
          wwpns: this.iswwpnValid && this.fcEnable ? wwpn : [],
          sanClientWwpns: Wwpn,
          sanClientIqns:
            this.formGroup.value.sanclientIqn.length === 0
              ? []
              : this.isIqnManual
              ? [`Manual.${this.formGroup.value.sanclientIqn}`]
              : [this.formGroup.value.sanclientIqn]
        };
      } else {
        this.agentLanFreeDTO = {
          delete: !this.enableLanFree,
          fcPorts: map(this.tableData?.data, item => {
            return assign(omit(item, ['parent']), {
              chosen: includes(map(this.selectionPort, 'id'), item.id)
            });
          }),
          wwpns: map(
            reject(this.wwpnData, item => item.isEidt),
            item => {
              return assign(omit(item, ['parent', 'id', 'isEidt']), {
                chosen: includes(map(this.selectionWwpn, 'wwpn'), item.wwpn)
              });
            }
          )
        };
      }

      if (this.data.osType === this.dataMap.Os_Type.aix.value) {
        this.agentLanFreeAixService
          .configurationLanFreeAixUsingPUT({
            memberEsn: this.formGroup.value.esn,
            agentId: this.data?.uuid,
            agentLanFreeAixDTO: this.agentLanFreeAixDTO
          })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      } else {
        const params = {
          memberEsn: this.formGroup.value.esn,
          agentId: this.data?.uuid,
          agentLanFreeDTO: this.agentLanFreeDTO
        };
        if (this.appUtilsService.isDecouple) {
          assign(params, { targetEsn: this.formGroup.value.esn });
        }
        this.agentLanFreeService.configurationLanFreePUT(params).subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
      }
    });
  }
}
