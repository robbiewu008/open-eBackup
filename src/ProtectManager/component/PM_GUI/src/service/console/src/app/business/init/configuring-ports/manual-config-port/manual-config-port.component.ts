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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormGroup } from '@angular/forms';
import { MessageService } from '@iux/live';
import { RouteConfigComponent } from 'app/business/system/settings/config-network/route-config/route-config.component';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  LogManagerApiService,
  MODAL_COMMON,
  SystemApiService
} from 'app/shared';
import { TableData } from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { assign, cloneDeep, each, filter, find, includes, size } from 'lodash';
import { combineLatest } from 'rxjs';
import { CreatLogicPortComponent } from '../creat-logic-port/creat-logic-port.component';

@Component({
  selector: 'aui-manual-config-port',
  templateUrl: './manual-config-port.component.html',
  styleUrls: ['./manual-config-port.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ManualConfigPortComponent implements OnInit {
  @Input() componentData;
  @Input() activeIndex;
  @Input() isModify;
  @Input() modifying;
  @Input() memberEsn;
  @Output() onStatusChange = new EventEmitter<any>();
  dataMap = DataMap;
  tableData: TableData;
  formGroup: FormGroup;
  rawData;
  newConfigData = [];
  bondPortList: any[];
  logicalData: any[];
  dmLogicData: any[];
  controllerNames = [];
  columns = [];
  tableColumnKey = 'settings-network-config';

  networkType = [];
  orginalNetworkType = [];
  controllers = [];
  isCloudBackup = includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value
    ],
    this.i18n.get('deploy_type')
  );

  @ViewChild('operationTpl', { static: true }) operationTpl: TemplateRef<any>;
  @ViewChild('titleTpl', { static: true }) titleTpl: TemplateRef<any>;

  constructor(
    public message: MessageService,
    public baseUtilService: BaseUtilService,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private systemApiService: SystemApiService,
    private drawModalService: DrawModalService,
    private logManagerApiService: LogManagerApiService,
    private rememberColumnsService: RememberColumnsService
  ) {}

  ngOnInit(): void {
    this.initColumns();
    this.getControl();
  }

  initColumns() {
    this.columns = [
      {
        label: this.i18n.get('common_name_label'),
        key: 'name',
        isShow: true,
        width: 140
      },
      {
        label: this.i18n.get('common_role_label'),
        key: 'role',
        filter: !this.isCloudBackup,
        filterMap: this.dataMapService.toArray('initRoleTable'),
        isShow: true,
        width: 75
      },
      {
        key: 'portType',
        label: this.i18n.get('common_ports_type_label'),
        isShow: true
      },
      {
        key: 'ip',
        label: this.i18n.get('common_ip_address_label'),
        isShow: true
      },
      {
        key: 'mask',
        label: this.i18n.get('common_subnet_mask_prefix_label'),
        isShow: true
      },
      {
        key: 'gateWay',
        label: this.i18n.get('common_gateway_label'),
        isShow: true
      },
      {
        key: 'vlan',
        label: this.i18n.get('common_vlan_use_label'),
        isShow: true
      },
      {
        key: 'mtu',
        label: this.i18n.get('common_mtu_label'),
        isShow: true
      },
      {
        key: 'ethernetPort',
        label: this.i18n.get('common_bounded_ethernet_port_label'),
        isShow: true,
        width: this.isModify ? 120 : 240
      },
      {
        key: 'operation',
        label: this.i18n.get('common_operation_label'),
        isShow: true,
        width: this.i18n.isEn ? 285 : 165
      }
    ];
    if (this.isModify) {
      this.columns.unshift({
        label: 'ID',
        key: 'id',
        isShow: false,
        width: 134
      });
      this.columns.splice(3, 0, {
        label: this.i18n.get('common_dm_exist_label'),
        key: 'dmExists',
        isShow: true,
        filter: true,
        filterMap: this.dataMapService.toArray('initLogicPortExistStatus')
      });
      this.columns.pop();
    }
  }

  modifyChange(e) {
    // 网络配置修改变化
    if (e) {
      this.columns.push({
        key: 'operation',
        label: this.i18n.get('common_operation_label'),
        isShow: true,
        width: this.i18n.isEn ? 285 : 180
      });
    } else {
      this.columns.pop();
    }
    this.modifying = e;
    each(this.controllers, control => {
      control.tableCols = cloneDeep(this.columns);
    });
  }

  getData() {
    this.systemApiService
      .getAllPorts({
        queryLogicPortRequest: {},
        ethLogicTypeValue: '0;13',
        akOperationTips: false,
        memberEsn: this.memberEsn || ''
      })
      .subscribe((res: any) => {
        this.clearControl();
        if (!!res?.ethPortDtoList?.length) {
          // 除physicaltype为1的端口不能使用
          res.ethPortDtoList = filter(
            res.ethPortDtoList,
            item => item.physicalType === '1'
          );
        }
        this.rawData = cloneDeep(res);
        each(res.logicPortDtoList, item => {
          if (
            find(res.reuseLogicPortNameList, usedPort => usedPort === item.name)
          ) {
            assign(item, {
              modifyDisabled: true
            });
          }
          if (!this.networkType.includes(item.role)) {
            this.networkType.push(item.role);
            this.networkType = [...this.networkType];
            this.orginalNetworkType = this.networkType;
          }
          each(this.controllers, control => {
            if (
              control.controllerName ===
              (item?.homeControllerId ?? item.currentControllerId)
            ) {
              control.controllerData.push(item);
            }
            control.controllerData = [...control.controllerData];
          });
        });
        this.tableData = {
          data: res.logicPortDtoList,
          total: size(res.logicPortDtoList)
        };
        this.cdr.detectChanges();
        this.logicalData = res.logicPortDtoList;
        this.bondPortList = res.bondPortList;
        this.dmLogicData = res.dmLogicPortList.filter(
          port => !find(res.logicPortDtoList, { id: port.ID })
        );
        this.disableBtn();
      });
  }

  clearControl() {
    each(this.controllers, control => {
      control.controllerData = [];
    });
    this.networkType = [];
  }

  getControl() {
    this.controllers = [];
    this.logManagerApiService
      .collectNodeInfo({ memberEsn: this.memberEsn || '' })
      .subscribe(
        res => {
          each(res.data, item => {
            this.controllers.push({
              controllerData: [],
              controllerName: item.nodeName,
              tableCols: cloneDeep(this.columns),
              tableColsKey: this.tableColumnKey + item.nodeName
            });
          });
          this.getData();
        },
        err => {}
      );
  }

  createPort(controllerData, name) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'creat-logic-port',
        lvWidth: MODAL_COMMON.normalWidth + 200,
        lvHeader: this.i18n.get('common_config_ports_label'),
        lvContent: CreatLogicPortComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          controlType: name,
          rowData: controllerData,
          data: cloneDeep(this.rawData),
          portData: this.tableData,
          modifyData: false,
          componentData: this.componentData,
          memberEsn: this.memberEsn
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreatLogicPortComponent;
          combineLatest([
            content.formGroup.statusChanges,
            content.validTable
          ]).subscribe(res => {
            const [formVaild, validTable] = res;
            modal.getInstance().lvOkDisabled =
              formVaild !== 'VALID' || !validTable;
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreatLogicPortComponent;
            content.onOk().subscribe({
              next: res => {
                resolve(true);
                this.getData();
              },
              error: () => {
                resolve(false);
              }
            });
          });
        }
      })
    );
  }

  configLogicPort(rowData, data, type) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'creat-logic-port',
        lvWidth: MODAL_COMMON.normalWidth + 200,
        lvHeader: this.i18n.get('common_modify_ports_label'),
        lvContent: CreatLogicPortComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          controlType: type,
          rowData,
          data: cloneDeep(this.rawData),
          portData: this.tableData,
          modifyData: data?.ip ? data : false,
          componentData: this.componentData,
          memberEsn: this.memberEsn
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreatLogicPortComponent;
          combineLatest([
            content.formGroup.statusChanges,
            content.validTable
          ]).subscribe(res => {
            const [formVaild, validTable] = res;
            modal.getInstance().lvOkDisabled =
              formVaild !== 'VALID' || !validTable;
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreatLogicPortComponent;
            content.onOk().subscribe({
              next: res => {
                resolve(true);
                this.getData();
              },
              error: () => {
                resolve(false);
              }
            });
          });
        }
      })
    );
  }

  deletePort(data, rowData, name) {
    this.systemApiService
      .deletePort({
        name: rowData.name,
        memberEsn: this.memberEsn || ''
      })
      .subscribe(
        res => {
          this.deletePortChange(data, rowData, name);
          this.disableBtn();
          this.getData();
        },
        () => {
          this.disableBtn();
        }
      );
  }

  deletePortChange(data, rowData, name) {
    data = filter(data, val => {
      return val.name !== rowData.name;
    });
    each(this.controllers, item => {
      if (item.controllerName === name) {
        item.controllerData = [...data];
      }
    });
    if (this.detectPort(rowData.role)) {
      this.networkType = filter(this.networkType, item => {
        return item !== rowData.role;
      });
      this.orginalNetworkType = this.networkType;
    }
  }

  detectPort(type) {
    // 检测是否需要变更选中的网络类型
    let redFlag = true;
    each(this.controllers, item => {
      if (find(item.controllerData, val => val.role === type)) {
        redFlag = false;
      }
    });
    return redFlag;
  }

  routeConfig(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'route-config',
        lvWidth: MODAL_COMMON.largeWidth,
        lvHeader: this.i18n.get('common_config_route_label'),
        lvContent: RouteConfigComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          data,
          componentData: this.componentData,
          memberEsn: this.memberEsn
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  tableStatusChange(e) {
    // 根据表格传回操作类型执行操作
    if (e.action === 'create') {
      this.createPort(e.data, e.name);
    } else if (e.action === 'modify') {
      this.configLogicPort(e.data, e.item, e.name);
    } else if (e.action === 'routeConfig') {
      this.routeConfig(e.item);
    } else if (e.action === 'delete') {
      this.deletePort(e.data, e.item, e.name);
    }
  }

  disableBtn() {
    // 判断是否有备份业务端口，或者有还没配置的端口
    let redFlag = false;
    if (!this.networkType.includes(DataMap.initRole.data.value)) {
      redFlag = true;
    }
    if (this.networkType.includes(DataMap.initRole.data.value)) {
      each(this.networkType, item => {
        each(this.controllers, control => {
          if (
            !find(
              control.controllerData,
              val => val.role === item && !!val?.ip
            ) ||
            find(control.controllerData, val => !val.ip)
          ) {
            redFlag = true;
          }
        });
      });
    }
    this.onStatusChange.emit(redFlag);
  }

  getNetworkData(type, data) {
    each(this.logicalData, item => {
      if (item?.role === type) {
        let tempPort = cloneDeep(item);
        data.push({
          name: tempPort.name
        });
      }
    });
    return data;
  }

  getComponentData() {
    let backupPort = [];
    let archivePort = [];
    let replicatePort = [];
    backupPort = this.getNetworkData(DataMap.initRole.data.value, backupPort);
    archivePort = this.getNetworkData(
      DataMap.initRole.dataManage.value,
      archivePort
    );
    replicatePort = this.getNetworkData(
      DataMap.initRole.copy.value,
      replicatePort
    );
    const params = assign(this.componentData, {
      configLanguage: {
        language: this.i18n.language
      },
      backupNetworkConfig: {
        logicPorts: backupPort
      }
    });
    if (!!archivePort.length) {
      assign(params, {
        archiveNetworkConfig: {
          logicPorts: archivePort
        }
      });
    }
    if (!!replicatePort.length) {
      assign(params, {
        copyNetworkConfig: {
          logicPorts: replicatePort
        }
      });
    }
    return params;
  }
}
