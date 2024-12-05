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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  FilterType,
  MessageboxService,
  MessageService,
  ModalRef,
  OptionItem,
  TransferColumnItem
} from '@iux/live';
import {
  BaseUtilService,
  CapacityCalculateLabel,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  isJson,
  quaDrantTableOther,
  WarningMessageService
} from 'app/shared';
import {
  LogManagerApiService,
  SftpManagerApiService,
  StorageUnitService,
  SystemApiService
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isNil,
  isUndefined,
  map,
  nth,
  set,
  some
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { RouteConfigComponent } from '../../config-network/route-config/route-config.component';

@Component({
  selector: 'aui-start-sftp',
  templateUrl: './start-sftp.component.html',
  styleUrls: ['./start-sftp.component.less'],
  providers: [CapacityCalculateLabel]
})
export class StartSftpComponent implements OnInit {
  data;
  activeNode;
  isModify;
  dataMap = DataMap;
  formGroup: FormGroup;
  wormGroup: FormGroup;
  source: { cols: TransferColumnItem[]; data: any[]; selection: any[] } = {
    cols: [],
    data: [],
    selection: []
  };
  target: { cols: TransferColumnItem[] } = { cols: [] };
  oldSource: { cols: TransferColumnItem[]; data: any[]; selection: any[] } = {
    cols: [],
    data: [],
    selection: []
  };
  oldTarget: { cols: TransferColumnItem[] } = { cols: [] };
  targetData = [];
  oldTargetData = [];
  vlanList = [];
  nonVlanCols = []; // 用于普通绑定复用表格展示
  vlanCols = []; // 用于vlan复用表格展示
  controllers = [];
  bondPort = [];
  logicPort = [];
  ethPort = [];
  oldPort = []; // 用于临时存储上次的端口
  lastSelectPort = [];
  enableRoute = false;
  routeGroupData = [];
  routeData = [];
  isGatewayDisabled = false;
  isX9000 = includes(
    [DataMap.Deploy_Type.x9000.value],
    this.i18n.get('deploy_type')
  );
  quaDrantTableOther = quaDrantTableOther;
  portEnable = false;
  enableVlan = false;
  tips = this.i18n.get('system_pod_rule_tip_label');
  modeOptions = this.dataMapService.toArray('wormComplianceMode');
  storageUnitOptions = [];
  IntervalUnitOptions = this.dataMapService
    .toArray('sftpProtectionTimeUnit')
    .reverse()
    .filter((v: OptionItem) => {
      return (v.isLeaf = true);
    });
  protectionDurations = this.IntervalUnitOptions.filter(v =>
    includes(['46', '47', '48'], v.value)
  );
  lockWaitDurations = this.IntervalUnitOptions.filter(v =>
    includes(['44', '45'], v.value)
  );
  homePortTypeOptions = this.dataMapService
    .toArray('initHomePortType')
    .filter(item => {
      return (item.isLeaf = true && item.value !== '8');
    });

  maskErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    ...this.baseUtilService.ipErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 128])
  };

  vlanErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 4094])
  };

  protectErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidInput: this.i18n.get('common_sftp_time_tip_label')
  };

  stopErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidInput: this.i18n.get('common_sftp_stop_time_tip_label')
  };

  @ViewChild('linkStatusTpl', { static: true }) linkStatusTpl: TemplateRef<any>;
  @ViewChild(RouteConfigComponent, { static: false })
  routConfigComponent: RouteConfigComponent;

  constructor(
    private modal: ModalRef,
    private logManagerApiService: LogManagerApiService,
    public fb: FormBuilder,
    public i18n: I18NService,
    public messageBox: MessageboxService,
    public cookieService: CookieService,
    public dataMapService: DataMapService,
    public messageService: MessageService,
    public baseUtilService: BaseUtilService,
    public appUtilsService: AppUtilsService,
    public drawModalService: DrawModalService,
    public virtualScroll: VirtualScrollService,
    public systemApiService: SystemApiService,
    public batchOperateService: BatchOperateService,
    public storageUnitService: StorageUnitService,
    public sftpManagerApiService: SftpManagerApiService,
    public warningMessageService: WarningMessageService,
    public capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getControl();
    this.getStorageUnit();
    this.initColumns();
  }

  updateData() {
    if (this.isModify && isEmpty(this.data?.poolId)) {
      this.data.poolId = '0';
    }
    this.formGroup.patchValue(this.data);
    this.wormGroup.patchValue(this.data);
    if (!this.data.isWormExist) {
      this.wormGroup.get('wormType').setValue(1);
      this.wormGroup.get('wormDefProtectPeriod').setValue('0');
      this.wormGroup.get('defProtectTimeUnit').setValue('48');
      this.wormGroup.get('autoLockTimeUnit').setValue('45');
      this.wormGroup.get('wormAutoLockTime').setValue('1');
    }

    if (this.data.isWormExist) {
      this.wormGroup.get('wormType').disable();
      this.wormGroup.get('wormDefProtectPeriod').disable();
      this.wormGroup.get('wormAutoLockTime').disable();
    }
    if (this.data.ipType === 'IPV6') {
      this.formGroup.get('ipType').setValue('IPV6');
    } else {
      this.formGroup.get('ipType').setValue('IPV4');
    }
    if (!!this.data.vlan) {
      this.enableVlan = true;
      this.vlanChange(this.enableVlan);
      this.formGroup.get('homePortType').setValue(this.data.vlan.portType);
      if (!this.formGroup.value.isReuse) {
        this.formGroup.get('vlanID').setValue(this.data.vlan.tags.join(','));
      }
    }
    this.routeData = this.data.portRoutes;
    if (!!this.data?.portRoutes?.length) {
      this.formGroup.get('enableRoute').setValue(true);
    }
    defer(() => this.updatePort());

    this.formGroup.statusChanges.subscribe(result => {
      this.modal.getInstance().lvOkDisabled =
        result === 'INVALID' ||
        this.wormGroup.invalid ||
        !this.validPort() ||
        this.validRoute();
    });
    this.wormGroup.statusChanges.subscribe(result => {
      this.modal.getInstance().lvOkDisabled =
        result === 'INVALID' || !this.validPort() || this.validRoute();
    });
  }

  updatePort() {
    if (this.portEnable) {
      if (!this.formGroup.value.isReuse) {
        this.source.selection = [...this.lastSelectPort];
        this.targetData = this.source.selection;
      } else {
        this.oldSource.selection = filter(this.oldSource.data, item =>
          this.oldPort.includes(item.id)
        );
        this.oldTargetData = this.oldSource.selection;
      }
      this.formGroup.updateValueAndValidity();
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      ipType: new FormControl('IPV4'),
      ip: new FormControl(''),
      mask: new FormControl(''),
      gateway: new FormControl('', {
        validators: [this.baseUtilService.VALID.ipv4()]
      }),
      poolId: new FormControl('', {
        validators: this.baseUtilService.VALID.required()
      }),
      homePortType: new FormControl('1'),
      vlanID: new FormControl(''),
      enableRoute: new FormControl(false),
      isWormEnable: new FormControl(false),
      isReuse: new FormControl(false)
    });
    this.wormGroup = this.fb.group({
      isWormExist: new FormControl(false),
      wormType: new FormControl(1),
      wormDefProtectPeriod: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.validProtectTime(),
          this.baseUtilService.VALID.required()
        ]
      }),
      defProtectTimeUnit: new FormControl('48'),
      isWormAutoLock: new FormControl(true),
      wormAutoLockTime: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.integer(),
          this.validLockWaitTime(),
          this.baseUtilService.VALID.required()
        ]
      }),
      autoLockTimeUnit: new FormControl('45'),
      isWormAutoDel: new FormControl(false)
    });
    this.formGroup.get('ipType').valueChanges.subscribe(res => {
      if (res === 'IPV4') {
        this.formGroup
          .get('ip')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID._ipv4()
          ]);
        this.formGroup
          .get('mask')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.mask)
          ]);
        this.formGroup
          .get('gateway')
          .setValidators(this.baseUtilService.VALID.ipv4());
      } else {
        this.formGroup
          .get('ip')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID._ipv6()
          ]);
        this.formGroup
          .get('mask')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 128)
          ]);
        this.formGroup
          .get('gateway')
          .setValidators(this.baseUtilService.VALID._ipv6());
      }
      this.formGroup.get('ip').updateValueAndValidity();
      this.formGroup.get('mask').updateValueAndValidity();
      this.formGroup.get('gateway').updateValueAndValidity();
    });

    this.formGroup.get('homePortType').valueChanges.subscribe(res => {
      if (this.enableVlan) {
        defer(() => this.switchOldCols());
      }
      if (res === DataMap.initHomePortType.ethernet.value) {
        if (!this.enableVlan) {
          // 从绑定切到以太如果没有开vlan则不展示复用
          this.formGroup.get('isReuse').setValue(false);
        }
        this.tips = this.isX9000
          ? this.i18n.get('system_pod_rule_x9000_tip_label')
          : this.i18n.get('system_pod_rule_tip_label');
        this.source.data = this.ethPort.filter(item => {
          return (
            item.runningStatus === DataMap.initRuningStatus.connect.value &&
            ((this.isX9000 &&
              item.logicType === DataMap.initLogicType.frontEndPort.value) ||
              !this.isX9000)
          );
        });
        if (this.source.cols.length === 4) {
          this.source.cols.pop();
          this.target.cols = this.source.cols;
        }
      } else {
        this.tips = this.isX9000
          ? this.i18n.get('system_pod_bonding_rule_x9000_tip_label')
          : this.i18n.get('system_pod_bonding_rule_tip_label');
        this.getBondTablePort(this.oldPort);
        if (this.source.cols.length === 3) {
          this.source.cols.push({
            key: 'runningStatus',
            label: this.i18n.get('protection_running_status_label'),
            filterType: FilterType.DEFAULT,
            filterMultiple: true,
            filters: this.dataMapService.toArray('initRuningStatus'),
            render: this.linkStatusTpl
          });
          this.target.cols = this.source.cols;
        }
      }
      this.source.selection = [];
      this.targetData = [];
      this.formGroup.updateValueAndValidity();
    });

    this.formGroup.get('isReuse').valueChanges.subscribe(res => {
      if (this.enableVlan && res) {
        this.formGroup.get('vlanID').clearValidators();
        this.formGroup.get('vlanID').updateValueAndValidity();
        this.switchOldCols();
      } else if (
        res &&
        this.formGroup.value.homePortType ===
          DataMap.initHomePortType.bonding.value
      ) {
        this.oldSource.data = this.bondPort;
        this.oldSource.cols = this.nonVlanCols;
        this.oldTarget.cols = this.nonVlanCols;
      }
      if (this.enableVlan && !res) {
        this.formGroup
          .get('vlanID')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.rangeValue(1, 4094),
            this.baseUtilService.VALID.integer()
          ]);
        this.formGroup.get('vlanID').updateValueAndValidity();
        // vlan情况下切换需要右边表格项变化
        this.target.cols = this.source.cols;
      }
      defer(() => this.tipChange());
      this.oldSource.selection = [];
      this.oldTargetData = [];
    });

    this.formGroup.get('enableRoute').valueChanges.subscribe(res => {
      this.formGroup.updateValueAndValidity();
    });

    this.wormGroup.get('defProtectTimeUnit').valueChanges.subscribe(res => {
      defer(() =>
        this.wormGroup.get('wormDefProtectPeriod').updateValueAndValidity()
      );
    });

    this.wormGroup.get('autoLockTimeUnit').valueChanges.subscribe(res => {
      defer(() =>
        this.wormGroup.get('wormAutoLockTime').updateValueAndValidity()
      );
    });

    this.wormGroup.get('isWormAutoLock').valueChanges.subscribe(res => {
      if (!res) {
        this.wormGroup.get('wormAutoLockTime').clearValidators();
      } else {
        this.wormGroup
          .get('wormAutoLockTime')
          .setValidators([
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.required(),
            this.validLockWaitTime()
          ]);
      }
      this.wormGroup.get('wormAutoLockTime').updateValueAndValidity();
    });
  }

  getBondTablePort(port: any) {
    // 用于获取绑定类型的表格数据，提取出来降低深度
    this.source.data = this.ethPort.filter(item => {
      if (item.logicType !== DataMap.initLogicType.frontEndPort.value) {
        return false;
      }

      if (!!item?.failOverGroup) {
        return false;
      }

      // 以太类型vlan里面用过的端口绑定也不能用
      if (
        find(
          this.vlanList,
          vlanPort =>
            vlanPort.portNameList.includes(item.location) &&
            vlanPort.portType === DataMap.initHomePortType.ethernet.value
        )
      ) {
        return false;
      }

      const usedPort = filter(
        this.logicPort,
        val =>
          val.HOMEPORTTYPE !== DataMap.initHomePortType.bonding.value &&
          item.location === val.HOMEPORTNAME
      );

      if (!this.portEnable) {
        return !usedPort.length;
      } else {
        // 否则如果端口被SFTP本身创出的逻辑端口使用是可以的
        return (
          !usedPort.length ||
          (usedPort.length === 1 && find(port, val => val === item.location))
        );
      }
    });
  }

  getStorageUnit() {
    this.appUtilsService.getResourceByRecursion(
      null,
      params => this.storageUnitService.queryBackUnitGET(params),
      resource => {
        const unitArray = [];
        each(resource, item => {
          if (
            item.generatedType ===
              DataMap.backupStorageGeneratedType.local.value &&
            item.deviceType ===
              DataMap.poolStorageDeviceType.OceanProtectX.value &&
            item.deviceId === this.activeNode
          ) {
            unitArray.push({
              ...item,
              value: item.poolId,
              label: item.name,
              isLeaf: true
            });
          }
        });
        this.storageUnitOptions = unitArray;
      }
    );
  }

  tipChange() {
    // 用来动态变化内嵌通知的提示
    const { homePortType, isReuse } = this.formGroup.value;
    if (this.isX9000 && isReuse) {
      this.tips = this.i18n.get('system_pod_rule_x9000_tip_label');
    } else if (this.isX9000 && !isReuse) {
      this.tips =
        homePortType === DataMap.initHomePortType.ethernet.value
          ? this.i18n.get('system_pod_rule_x9000_tip_label')
          : this.i18n.get('system_pod_bonding_rule_x9000_tip_label');
    } else if (!this.isX9000 && !isReuse) {
      this.tips =
        homePortType === DataMap.initHomePortType.ethernet.value
          ? this.i18n.get('system_pod_rule_tip_label')
          : this.i18n.get('system_pod_bonding_rule_tip_label');
    } else {
      this.tips = this.i18n.get('system_pod_rule_x9000_tip_label');
    }
  }

  switchOldCols() {
    // 开了vlan处理表格的表头和数据
    const portType =
      this.formGroup.value.homePortType ===
      DataMap.initHomePortType.bonding.value
        ? DataMap.initHomePortType.bonding.value
        : DataMap.initHomePortType.ethernet.value;
    this.oldSource.data = this.vlanList.filter(
      item => item.portType === portType
    );
    this.oldSource.cols = this.vlanCols;
    this.oldTarget.cols = this.vlanCols;
    this.oldSource.selection = [];
    this.oldTargetData = [];
    this.formGroup.updateValueAndValidity();
  }

  validProtectTime(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }

      const value = control.value;
      const unit = this.wormGroup.value.defProtectTimeUnit;
      let num;
      if (unit === DataMap.sftpProtectionTimeUnit.year.value) {
        num = value * 365;
      } else if (unit === DataMap.sftpProtectionTimeUnit.month.value) {
        num = value * 30;
      } else {
        num = value;
      }
      if (num < 0 || num > 70 * 365) {
        return { invalidInput: { value: control.value } };
      }
      return null;
    };
  }

  validLockWaitTime(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value)) {
        return null;
      }

      const value = control.value;
      const unit = this.wormGroup.value.autoLockTimeUnit;
      let num;
      if (unit === DataMap.sftpProtectionTimeUnit.hour.value) {
        num = value * 60;
      } else {
        num = value;
      }
      if (num < 0 || num > 10 * 365 * 24 * 60) {
        return { invalidInput: { value: control.value } };
      }
      return null;
    };
  }

  getWormTip() {
    return this.formGroup.value.isWormEnable
      ? this.i18n.get('common_sftp_worm_enable_tip_label')
      : this.i18n.get('common_sftp_worm_disable_tip_label');
  }

  getControl() {
    this.logManagerApiService
      .collectNodeInfo({ memberEsn: this.activeNode || '' })
      .subscribe(
        res => {
          each(res.data, item => {
            this.controllers.push(item.nodeName);
          });
          this.getData();
          this.source.cols[1].filters = this.getFilter();
          this.target.cols[1].filters = this.getFilter();
        },
        err => {}
      );
  }

  getData() {
    this.systemApiService
      .getAllPorts({
        queryLogicPortRequest: {},
        ethLogicTypeValue: this.isX9000 ? '0' : '0;13',
        akOperationTips: false,
        memberEsn: this.activeNode || ''
      })
      .subscribe(
        (res: any) => {
          this.logicPort = res.dmLogicPortList;
          if (!!res?.ethPortDtoList?.length) {
            // 除physicaltype为1的端口不能使用
            res.ethPortDtoList = filter(
              res.ethPortDtoList,
              item => item.physicalType === '1'
            );
          }
          this.ethPort = map(res.ethPortDtoList, item => {
            return assign(item, {
              logicTypeLabel: this.dataMapService.getLabel(
                'initLogicType',
                item.logicType
              ),
              control: this.isX9000
                ? this.getControlType(item)
                : item.ownIngController
            });
          });
          this.bondPort = res.bondPortList.map((item: any) => {
            return assign(item, {
              control: find(this.ethPort, port =>
                item.portIdList.includes(port.id)
              )['control']
            });
          });

          this.vlanList = res.vlanList.map(vlan => {
            return assign(vlan, {
              control: find(
                this.ethPort,
                val => val.location === vlan.portNameList[0]
              ).control
            });
          });

          this.portEnable = !isNil(this.data.port);
          if (this.portEnable) {
            // 用于处理以前创建造成的数据
            this.oldDataConfig();
          } else {
            // 之前没口则直接过滤就行
            this.ethPort = filter(
              this.ethPort,
              item =>
                !find(this.bondPort, port => {
                  return port.portIdList.includes(item.id);
                })
            );
          }

          if (this.isX9000) {
            // X9000只能使用前端端口
            this.vlanList = filter(this.vlanList, item => {
              return !find(
                this.ethPort,
                port =>
                  port.location === item.portNameList[0] &&
                  port.logicType === DataMap.initLogicType.podFrontEndPort.value
              );
            });
          }

          // 普通场景下，默认为以太网口，把连接状态过滤一下
          this.source.data = filter(
            this.ethPort,
            item =>
              item.runningStatus === DataMap.initRuningStatus.connect.value
          );

          this.updateData();
          this.formGroup.updateValueAndValidity();
        },
        err => {}
      );
  }

  oldDataConfig() {
    let port = JSON.parse(isJson(this.data?.port) ? this.data?.port : '{}');
    if (isEmpty(port)) {
      // 升级场景是用的名字，去所有口里找出来
      port = map(this.ethPort, item => {
        if (item.location.includes(this.data?.port)) {
          return item.id;
        }
      });
    }
    this.oldPort = port;
    if (!this.formGroup.value.isReuse) {
      // 如果不是复用场景，则使用第一套表
      // 先获取选中项
      this.lastSelectPort = filter(this.ethPort, item => {
        return find(this.oldPort, val => val === item.id);
      });
      // 过滤所有绑定端口包含的以太口，，以及把选中的，即上次创的时候使用的塞回去
      this.ethPort = filter(
        this.ethPort,
        item =>
          !find(this.bondPort, port => {
            return port.portIdList.includes(item.id);
          }) || find(this.lastSelectPort, { id: item.id })
      );
    }
  }

  getControlType(item) {
    // X9000获取端口所属控制器
    const Quadrant = nth(item.location.split('.'), -2);
    const controlType = find(this.controllers, control => {
      return some(this.quaDrantTableOther, table => {
        return (
          includes(table.table, Quadrant) && includes(control, table[item.name])
        );
      });
    });
    return controlType;
  }

  initColumns() {
    const columns = [
      {
        key: 'location',
        label: this.i18n.get('common_location_label'),
        filterType: FilterType.SEARCH,
        width: 150
      },
      {
        key: 'control',
        label: this.i18n.get('common_controller_label'),
        filterType: FilterType.DEFAULT,
        filterMultiple: true,
        filters: this.getFilter()
      },
      {
        key: 'logicTypeLabel',
        label: this.i18n.get('system_logic_type_label')
      }
    ];
    this.source.cols = columns;
    this.target.cols = cloneDeep(columns);
    this.nonVlanCols = [
      {
        key: 'name',
        label: this.i18n.get('common_name_label')
      },
      {
        key: 'mtu',
        label: this.i18n.get('common_maximum_transmission_unit_label')
      },
      {
        key: 'control',
        label: this.i18n.get('common_controller_label'),
        filterType: FilterType.DEFAULT,
        filterMultiple: true,
        filters: this.getFilter()
      },
      {
        key: 'runningStatus',
        label: this.i18n.get('protection_running_status_label'),
        filterType: FilterType.DEFAULT,
        filterMultiple: true,
        filters: this.dataMapService.toArray('initRuningStatus'),
        render: this.linkStatusTpl
      }
    ];
    this.vlanCols = [
      {
        key: 'name',
        label: this.i18n.get('common_name_label'),
        width: 150
      },
      {
        key: 'tags',
        label: 'VLAN ID'
      },
      {
        key: 'control',
        label: this.i18n.get('common_controller_label'),
        filterType: FilterType.DEFAULT,
        filterMultiple: true,
        filters: this.getFilter()
      },
      {
        key: 'runningStatus',
        label: this.i18n.get('protection_running_status_label'),
        filterType: FilterType.DEFAULT,
        filterMultiple: true,
        filters: this.dataMapService.toArray('initRuningStatus'),
        render: this.linkStatusTpl
      }
    ];
    this.oldSource.cols = this.nonVlanCols;
    this.oldTarget.cols = this.nonVlanCols;
  }

  getFilter() {
    return map(this.controllers, control => {
      return assign(
        {},
        {
          value: control,
          label: control,
          key: control
        }
      );
    });
  }

  selectionChange(e) {
    this.source.selection = e?.selection;
    this.targetData = e?.selection;
    this.formGroup.updateValueAndValidity();
  }

  oldSelectionChange(e) {
    this.oldTargetData = e.selection;
    this.formGroup.updateValueAndValidity();
  }

  vlanChange(e) {
    if (e) {
      if (!this.formGroup.value.isReuse) {
        this.formGroup
          .get('vlanID')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.rangeValue(1, 4094),
            this.baseUtilService.VALID.integer()
          ]);
        this.target.cols = this.source.cols;
      } else {
        this.switchOldCols();
      }
    } else {
      this.formGroup.get('vlanID').clearValidators();
      if (this.formGroup.value.homePortType === '1') {
        this.formGroup.get('isReuse').setValue(false);
      } else {
        if (this.formGroup.value.isReuse) {
          this.oldSource.data = this.bondPort;
          this.oldSource.cols = this.nonVlanCols;
          this.oldTarget.cols = this.nonVlanCols;
        }
      }
    }
    defer(() => this.tipChange());
    this.formGroup.get('vlanID').updateValueAndValidity();
  }

  routeStatusChange(e) {
    this.routeGroupData = e;
    this.modal.getInstance().lvOkDisabled = some(e, item => item.invalid);
    if (
      find(
        e,
        item => item.get('type').value === DataMap.initRouteType.default.value
      )
    ) {
      this.formGroup.get('gateway').disable();
      this.formGroup.get('gateway').setValue('');
    } else {
      this.formGroup.get('gateway').enable();
    }
  }

  validRoute() {
    if (!this.formGroup.get('enableRoute').value) {
      return false;
    }
    return some(this.routeGroupData, item => item.invalid);
  }

  validPort() {
    let redFlag = true;
    each(this.controllers, control => {
      if (
        (!this.isX9000 && this.findPort(control)) ||
        (this.isX9000 && !this.targetData.length)
      ) {
        redFlag = false;
      }
    });

    if (!this.controllers.length) {
      redFlag = false;
    }

    return redFlag;
  }

  findPort(control) {
    // 校验端口在不同情况下的数量
    let tmpData = filter(
      !this.formGroup.value.isReuse ? this.targetData : this.oldTargetData,
      item => item.control === control
    );
    let onlinePort = filter(
      tmpData,
      item => item.runningStatus === DataMap.initRuningStatus.connect.value
    );
    if (
      this.formGroup.value.homePortType ===
      DataMap.initHomePortType.bonding.value
    ) {
      if (this.formGroup.value.isReuse) {
        return tmpData.length < 1 || onlinePort.length < 1;
      } else {
        return tmpData.length < 2 || onlinePort.length < 1;
      }
    } else {
      return tmpData.length < 1;
    }
  }

  getParams() {
    const portData = !isUndefined(this.routConfigComponent)
      ? this.routConfigComponent.getTargetParams()
      : [];
    const params: any = {
      SftpSwitchRequest: {
        status: 1,
        ...this.formGroup.getRawValue(),
        ...this.wormGroup.getRawValue(),
        port: JSON.stringify(
          map(
            !this.formGroup.value.isReuse
              ? this.targetData
              : this.oldTargetData,
            item => item.id
          )
        )
      },
      memberEsn: this.activeNode || ''
    };
    delete params.SftpSwitchRequest.vlanID;
    delete params.SftpSwitchRequest.enableRoute;
    if (this.enableVlan) {
      params.SftpSwitchRequest.homePortType =
        DataMap.initHomePortType.vlan.value;
      params.SftpSwitchRequest.vlan = {
        portType: this.formGroup.value.homePortType,
        tags: this.formGroup.get('isReuse').value
          ? []
          : [this.formGroup.value.vlanID]
      };
    }
    if (!!portData.length) {
      set(params, 'SftpSwitchRequest.portRoutes', portData);
    }
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (!this.isModify) {
        this.sftpManagerApiService
          .switchSftpStatusUsingPUT(this.getParams())
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      } else {
        this.sftpManagerApiService
          .modifySftpConfigUsingPUT(this.getParams())
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }
}
