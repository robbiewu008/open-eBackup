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
import { FilterType, ModalRef, TransferColumnItem } from '@iux/live';
import { RouteConfigComponent } from 'app/business/system/settings/config-network/route-config/route-config.component';
import {
  BackupClustersNetplaneService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  LogManagerApiService,
  quaDrantTableOther,
  SystemApiService
} from 'app/shared';
import {
  assign,
  defer,
  each,
  filter,
  find,
  includes,
  isUndefined,
  map,
  nth,
  set,
  some,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { distinctUntilChanged } from 'rxjs/operators';

@Component({
  selector: 'aui-add-network',
  templateUrl: './add-network.component.html',
  styleUrls: ['./add-network.component.less']
})
export class AddNetworkComponent implements OnInit {
  drawData;
  isModify;
  formGroup: FormGroup;
  dataMap = DataMap;
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
  isX9000 = includes(
    [DataMap.Deploy_Type.x9000.value],
    this.i18n.get('deploy_type')
  );
  quaDrantTableOther = quaDrantTableOther;
  selectedPort = [];
  ethPort = [];
  bondPort = [];
  logicPort = [];
  enableRoute = false;
  routeGroupData = [];
  routeData = [];
  enableVlan = false;
  tips = this.i18n.get('system_pod_rule_tip_label');
  homePortTypeOptions = this.dataMapService
    .toArray('initHomePortType')
    .filter(item => {
      return (item.isLeaf = true && item.value !== '8');
    });

  ipErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.ipErrorTip,
    invalidSameName: this.i18n.get('common_create_logical_port_same_ip_label')
  };

  prefixErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    ...this.baseUtilService.requiredErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 128]),
    invalidName: this.i18n.get('common_invalid_inputtext_label')
  };

  vlanErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInput: this.i18n.get('common_invalid_input_label')
  };

  @ViewChild(RouteConfigComponent, { static: false })
  routConfigComponent: RouteConfigComponent;
  @ViewChild('linkStatusTpl', { static: true }) linkStatusTpl: TemplateRef<any>;

  constructor(
    public modal: ModalRef,
    public baseUtilService: BaseUtilService,
    public backupClusterNetplaneService: BackupClustersNetplaneService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private systemApiService: SystemApiService,
    private logManagerApiService: LogManagerApiService
  ) {}

  ngOnInit(): void {
    this.getControl();
    this.initColumns();
    this.initFormGroup();
  }

  getControl() {
    this.logManagerApiService.collectNodeInfo({}).subscribe(
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

  initColumns() {
    const columns = [
      {
        key: 'location',
        label: this.i18n.get('common_location_label'),
        filterType: FilterType.SEARCH,
        width: 130
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
    this.target.cols = columns;
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

  initFormGroup() {
    this.formGroup = this.fb.group({
      ipType: new FormControl('0', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      mask: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.mask)
        ]
      }),
      prefix: new FormControl(
        { value: '', disabled: true },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 128)
          ]
        }
      ),
      gaussIp: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ipv4(),
          this.validSameIp(1)
        ]
      }),
      infraIp: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ipv4(),
          this.validSameIp(2)
        ]
      }),
      gateway: new FormControl('', {
        validators: [this.baseUtilService.VALID.ipv4()]
      }),
      homePortType: new FormControl('1'),
      vlanID: new FormControl(''),
      shareBondPort: new FormControl(this.isX9000),
      reuse: new FormControl(false)
    });
    if (this.isX9000) {
      // X9000 默认共享
      this.formGroup.get('shareBondPort').disable();
      this.tipChange();
    }
    this.formGroup.statusChanges.subscribe(res => {
      this.disableBtn();
    });
    this.listenForm();
  }

  listenForm() {
    this.formGroup.get('ipType').valueChanges.subscribe(res => {
      if (!isUndefined(this.routConfigComponent)) {
        this.routConfigComponent.isIpv4 = res === '0';
        this.routConfigComponent.ipTypeChange();
      }
      const gaussIpValidators = [
        this.baseUtilService.VALID.required(),
        res === '0'
          ? this.baseUtilService.VALID.ipv4()
          : this.baseUtilService.VALID._ipv6(),
        this.validSameIp(1)
      ];
      const infraIpValidators = [
        this.baseUtilService.VALID.required(),
        res === '0'
          ? this.baseUtilService.VALID.ipv4()
          : this.baseUtilService.VALID._ipv6(),
        this.validSameIp(2)
      ];
      this.formGroup.get('gaussIp').setValidators(gaussIpValidators);
      this.formGroup.get('infraIp').setValidators(infraIpValidators);
      if (res === '0') {
        this.formGroup.get('prefix').disable();
        this.formGroup.get('mask').enable();
        this.formGroup
          .get('gateway')
          .setValidators(this.baseUtilService.VALID.ipv4());
      } else {
        this.formGroup.get('prefix').enable();
        this.formGroup.get('mask').disable();
        this.formGroup
          .get('gateway')
          .setValidators(this.baseUtilService.VALID._ipv6());
      }
      this.formGroup.get('infraIp').updateValueAndValidity();
      this.formGroup.get('gaussIp').updateValueAndValidity();
      this.formGroup.get('gateway').updateValueAndValidity();
    });
    this.formGroup.get('homePortType').valueChanges.subscribe(res => {
      if (res === DataMap.initHomePortType.ethernet.value) {
        // 从绑定切到以太把共享置false
        this.formGroup.get('shareBondPort').setValue(false);
        if (!this.enableVlan) {
          // 从绑定切到以太如果没有开vlan则不展示复用
          this.formGroup.get('reuse').setValue(false);
        }
        this.source.data = this.ethPort.filter(item => {
          return item.runningStatus === DataMap.initRuningStatus.connect.value;
        });
        if (this.source.cols.length === 4) {
          this.source.cols.pop();
          this.target.cols = this.source.cols;
        }
      } else {
        this.getBondTablePort(this.selectedPort);
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
      defer(() => this.tipChange());
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('reuse').valueChanges.subscribe(res => {
      if (this.enableVlan && res) {
        this.formGroup.get('vlanID').clearValidators();
        this.formGroup.get('vlanID').updateValueAndValidity();
        this.switchOldCols();
      } else if (
        res &&
        this.formGroup.value.homePortType ===
          DataMap.initHomePortType.bonding.value
      ) {
        // 复用绑定口
        this.oldSource.data = this.bondPort;
        this.oldSource.cols = this.nonVlanCols;
        this.oldTarget.cols = this.nonVlanCols;
      }
      if (this.enableVlan && !res) {
        this.formGroup
          .get('vlanID')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validVlanID()
          ]);
        this.formGroup.get('vlanID').updateValueAndValidity();
      }
      defer(() => this.tipChange());
      this.oldSource.selection = [];
      this.oldTargetData = [];
      this.disableBtn();
    });
    this.formGroup.get('shareBondPort').valueChanges.subscribe(res => {
      defer(() => this.tipChange());
      this.disableBtn();
    });
    this.formGroup
      .get('gaussIp')
      .valueChanges.pipe(distinctUntilChanged())
      .subscribe(res => {
        defer(() => this.formGroup.get('infraIp').updateValueAndValidity());
      });
    this.formGroup
      .get('infraIp')
      .valueChanges.pipe(distinctUntilChanged())
      .subscribe(res => {
        defer(() => this.formGroup.get('gaussIp').updateValueAndValidity());
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
        find(this.vlanList, vlanPort =>
          vlanPort.portNameList.includes(item.location)
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

      if (!this.isModify) {
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

  tipChange() {
    // 用来动态变化内嵌通知的提示
    const { homePortType, reuse, shareBondPort } = this.formGroup.value;
    if (this.isX9000 && reuse) {
      this.tips = this.i18n.get('system_pod_rule_x9000_tip_label');
    } else if (this.isX9000 && !reuse) {
      this.tips =
        homePortType === DataMap.initHomePortType.ethernet.value
          ? this.i18n.get('system_pod_rule_x9000_tip_label')
          : this.i18n.get('system_pod_bonding_rule_x9000_tip_label');
    } else if (!this.isX9000 && !reuse) {
      this.tips =
        homePortType === DataMap.initHomePortType.ethernet.value
          ? this.i18n.get('system_pod_rule_tip_label')
          : shareBondPort
          ? this.i18n.get('system_pod_bonding_rule_tip_label')
          : this.i18n.get('system_pod_add_network_bonding_rule_tip_label');
    } else {
      this.tips = shareBondPort
        ? this.i18n.get('system_pod_rule_tip_label')
        : this.i18n.get('system_pod_bonding_rule_tip_label');
    }
  }

  validSameIp(type): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      if (type === 1) {
        if (control.value === this.formGroup.value.infraIp) {
          return { invalidSameName: { value: control.value } };
        }
      } else {
        if (control.value === this.formGroup.value.gaussIp) {
          return { invalidSameName: { value: control.value } };
        }
      }

      return null;
    };
  }

  validVlanID(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup || !trim(control.value)) {
        return null;
      }

      let idGroup = control.value.split(',');
      if (idGroup.length < 1) {
        return { invalidInput: { value: control.value } };
      }
      if (
        some(idGroup, item => {
          return (
            item.includes(' ') ||
            Number(item) > 4094 ||
            Number(item) < 1 ||
            (!/^[1-9]\d*$/.test(item) &&
              !/^-[1-9]{1}\d*$/.test(item) &&
              '0' !== control.value)
          );
        })
      ) {
        return { invalidInput: { value: control.value } };
      }

      return null;
    };
  }

  getData() {
    this.systemApiService
      .getAllPorts({
        queryLogicPortRequest: {},
        ethLogicTypeValue: '0;13',
        akOperationTips: false
      })
      .subscribe((res: any) => {
        if (!!res?.ethPortDtoList?.length) {
          // 除physicaltype为1的端口不能使用
          res.ethPortDtoList = filter(
            res.ethPortDtoList,
            item => item.physicalType === '1'
          );
        }
        this.logicPort = res.dmLogicPortList;
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
        this.vlanList = res.vlanList.map(vlan => {
          return assign(vlan, {
            control: find(
              this.ethPort,
              val => val.location === vlan.portNameList[0]
            ).control
          });
        });

        this.bondPort = res.bondPortList.map((item: any) => {
          return assign(item, {
            control: find(this.ethPort, port =>
              item.portIdList.includes(port.id)
            )['control']
          });
        });

        if (!this.isModify) {
          this.ethPort = filter(
            this.ethPort,
            item =>
              !find(this.bondPort, port => {
                return port.portIdList.includes(item.id);
              })
          );
        }

        // 默认情况下展示以太网类型表格
        this.source.data = filter(this.ethPort, item => {
          return (
            !find(this.bondPort, port => {
              return port.portIdList.includes(item.id);
            }) && item.runningStatus === DataMap.initRuningStatus.connect.value
          );
        });

        if (this.isModify) {
          this.updateData();
        }
      });
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

  updateData() {
    this.backupClusterNetplaneService.getNetPlane({}).subscribe((res: any) => {
      this.selectedPort = res.portList;
      this.formGroup.patchValue(res);
      if (res.homePortType === DataMap.initHomePortType.vlan.value) {
        this.enableVlan = true;
        this.formGroup.get('homePortType').setValue(res.vlanPort.portType);
        defer(() => this.vlanChange(true));
        if (!res.reuse) {
          // 复用时不回显vlanId
          this.formGroup.get('vlanID').setValue(res.vlanPort.tags.join(','));
        }
      }

      if (res.ipType === '0') {
        this.formGroup.get('mask').setValue(res.mask);
      } else {
        this.formGroup.get('prefix').setValue(res.mask);
      }

      this.routeData = res.portRoutes;
      if (!!res.portRoutes.length) {
        this.enableRoute = true;
      }

      defer(() => this.updatePort(res));
    });
  }

  updatePort(res) {
    // 是否复用往不同的表里塞数据
    if (!res.reuse) {
      // 这里其实我们找到原来被用的口，往数据里塞就行了
      const filteredSelection = this.getFilterData(
        this.ethPort,
        this.bondPort,
        res.portList
      );
      this.source.selection = [...filteredSelection];
      this.source.data = this.ethPort;
      this.targetData = this.source.selection;
    } else {
      // 复用的时候要么绑定口复用，要么vlan复用
      // 把复用的vlan或者绑定取出来放到selection里面，本身数据还是bondPort和vlanList不用其他处理
      this.oldSource.selection = filter(this.oldSource.data, item =>
        res.portList.includes(item.id)
      );
      this.oldTargetData = this.oldSource.selection;
    }
    this.disableBtn();
  }

  getFilterData(data, bondPort, portList) {
    // 处理获取上来的端口数据
    const filteredSelection = filter(data, item =>
      some(portList, port => port === item.id)
    );
    // 选了绑定端口实际上是创了绑定端口，所以这里要判断如果是用网络平面创的绑定端口则不滤掉
    this.ethPort = filter(
      data,
      item =>
        !find(bondPort, port => port.portIdList.includes(item.id)) ||
        find(filteredSelection, { id: item.id })
    );
    return filteredSelection;
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
  }

  vlanChange(e) {
    if (e) {
      if (!this.formGroup.get('reuse').value) {
        // 复用vlan则不手输vlanid，去除校验
        this.formGroup
          .get('vlanID')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validVlanID()
          ]);
      } else {
        this.switchOldCols();
      }
    } else {
      this.formGroup.get('vlanID').clearValidators();
      // 如果取消vlan且为以太网口类型，则自动变为新建端口
      if (this.formGroup.value.homePortType === '1') {
        this.formGroup.get('reuse').setValue(false);
      } else {
        if (this.formGroup.get('reuse').value) {
          this.oldSource.data = this.bondPort;
          this.oldSource.cols = this.nonVlanCols;
          this.oldTarget.cols = this.nonVlanCols;
        }
      }
    }
    defer(() => this.tipChange());
    this.formGroup.get('vlanID').updateValueAndValidity();
  }

  routeChange(e) {
    this.disableBtn();
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
    if (!this.enableRoute) {
      return false;
    }
    return some(this.routeGroupData, item => item.invalid);
  }

  disableBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.invalid || !this.validPort() || this.validRoute();
  }

  validPort() {
    let redFlag = true;
    each(this.controllers, control => {
      if (
        (!this.isX9000 && this.findPort(control)) ||
        (this.isX9000 && this.diffFindPort())
      ) {
        redFlag = false;
      }
    });

    return redFlag;
  }

  diffFindPort() {
    // 用于X9000的端口数量校验
    let tmpData = !this.formGroup.value.reuse
      ? this.targetData
      : this.oldTargetData;
    let onlinePort = filter(
      tmpData,
      item => item.runningStatus === DataMap.initRuningStatus.connect.value
    );
    if (this.formGroup.value.reuse) {
      return onlinePort.length < 1;
    } else {
      if (
        this.formGroup.value.homePortType ===
        DataMap.initHomePortType.ethernet.value
      ) {
        return onlinePort.length < 1;
      } else {
        return tmpData.length < 2 || onlinePort.length < 1;
      }
    }
  }

  findPort(control) {
    // 用于其他系列的端口数量校验
    let tmpData = filter(
      !this.formGroup.value.reuse ? this.targetData : this.oldTargetData,
      item => item.control === control
    );
    let onlinePort = filter(
      tmpData,
      item =>
        (this.enableVlan && this.formGroup.value.reuse) ||
        item.runningStatus === DataMap.initRuningStatus.connect.value
    );
    // 没选端口直接回
    if (tmpData.length < 1) {
      return true;
    }
    const { homePortType, reuse, shareBondPort } = this.formGroup.value;
    const isBonding = homePortType === DataMap.initHomePortType.bonding.value;
    if (!reuse) {
      if (!isBonding) {
        // 一个已连接以太网口
        return onlinePort.length < 1;
      } else if (shareBondPort) {
        // 共享可以让所需口减半
        return onlinePort.length < 1 || tmpData.length < 2;
      } else {
        return onlinePort.length < 2 || tmpData.length < 4;
      }
    } else {
      if (shareBondPort) {
        return onlinePort.length < 1;
      } else {
        return onlinePort.length < 2;
      }
    }
  }

  selectionChange(e) {
    this.source.selection = e.selection;
    this.targetData = e.selection;
    this.disableBtn();
  }

  oldSelectionChange(e) {
    this.oldSource.selection = e.selection;
    this.oldTargetData = e.selection;
    this.disableBtn();
  }

  getParams() {
    // 端口放置顺序按ABCD来
    this.targetData.sort((a, b) => (a.control[1] < b.control[1] ? -1 : 1));
    this.oldTargetData.sort((a, b) => (a.control[1] < b.control[1] ? -1 : 1));
    const portData = !isUndefined(this.routConfigComponent)
      ? this.routConfigComponent.getTargetParams()
      : [];
    const params = {
      gaussIp: this.formGroup.value.gaussIp,
      infraIp: this.formGroup.value.infraIp,
      ipType: this.formGroup.value.ipType,
      mask:
        this.formGroup.value.ipType === '0'
          ? this.formGroup.value.mask
          : this.formGroup.value.prefix,
      portList: map(
        !this.formGroup.value.reuse ? this.targetData : this.oldTargetData,
        item => item.id
      ),
      homePortType: this.formGroup.value.homePortType,
      shareBondPort: this.isX9000
        ? true
        : this.formGroup.get('homePortType').value ===
          DataMap.initHomePortType.ethernet.value
        ? false
        : this.formGroup.get('shareBondPort').value,
      reuse: this.formGroup.value.reuse
    };
    if (!!portData.length) {
      set(params, 'portRoutes', portData);
    }
    if (!!this.formGroup.value.gateway) {
      assign(params, {
        gateway: this.formGroup.value.gateway
      });
    }
    if (this.enableVlan) {
      params.homePortType = DataMap.initHomePortType.vlan.value;
      set(params, 'vlanPort', {
        portType: this.formGroup.value.homePortType,
        tags: this.formGroup.get('reuse').value
          ? map(this.oldTargetData, item => item.tags[0])
          : this.formGroup.value.vlanID.split(',')
      });
    }
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.isModify) {
        this.backupClusterNetplaneService
          .updateInternalNetPlaneRelationUsingPut({
            request: this.getParams()
          })
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
        this.backupClusterNetplaneService
          .addInternalNetPlaneRelationUsingPost({
            request: this.getParams()
          })
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
