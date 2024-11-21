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
  AfterViewInit,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  LicenseApiService,
  MODAL_COMMON,
  OperateItems,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  each,
  filter,
  find,
  first,
  get,
  isEmpty,
  isUndefined,
  reject,
  size,
  trim,
  values
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { map } from 'rxjs/operators';
import { AddStorageDeviceComponent } from './add-storage-device/add-storage-device.component';

@Component({
  selector: 'aui-storage-device',
  templateUrl: './storage-device.component.html',
  styleUrls: ['./storage-device.component.less']
})
export class StorageDeviceComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData = {};
  optsConfig;
  selectionData = [];
  licenseInfos = [];
  liceseMap = {
    [DataMap.cyberDeviceStorageType.OceanProtect.value]:
      DataMap.License_Type.cyberEngineBackupStorage.value,
    [DataMap.cyberDeviceStorageType.OceanStorPacific.value]:
      DataMap.License_Type.cyberEngineDistributedStorage.value,
    [DataMap.cyberDeviceStorageType.OceanStorDorado.value]:
      DataMap.License_Type.cyberEngineAllFlash.value
  };
  dataMap = DataMap;

  totalFileSystem = 0;
  totalNormal = 0;
  totalOceanCyber = 0;
  totalInDevice = 0;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('nameTpl', { static: true }) nameTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private licenseApiService: LicenseApiService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngAfterViewInit(): void {
    this.getLicense().subscribe(
      () => this.dataTable.fetchData(),
      () => this.dataTable.fetchData()
    );
  }

  ngOnInit(): void {
    this.initConfig();
  }

  getLicense(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      this.licenseApiService
        .queryLicenseUsingGET({ akDoException: false })
        .subscribe({
          next: res => {
            this.licenseInfos = res;
            observer.next(res);
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
    });
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        type: 'primary',
        label: this.i18n.get('protection_add_device_label'),
        permission: OperateItems.AddStorageDevice,
        onClick: () => {
          this.addStorage();
        }
      },
      scan: {
        id: 'scan',
        label: this.i18n.get('common_rescan_label'),
        permission: OperateItems.AddStorageDevice,
        divide: true,
        disabledTips: this.i18n.get('common_license_invalid_label'),
        disableCheck: data => {
          return isEmpty(data);
        },
        onClick: data => {
          this.protectedResourceApiService
            .ScanProtectedResources({
              resId: data[0].uuid
            })
            .subscribe(res => {
              this.dataTable.fetchData();
            });
        }
      },
      modify: {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyStorageDevice,
        onClick: data => {
          this.addStorage(first(data));
        }
      },
      delete: {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteStorageDevice,
        disableCheck: data => {
          return isEmpty(data);
        },
        onClick: data => {
          this.deleteStorage(data);
        }
      }
    };
    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.nameTpl
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      {
        key: 'detectType',
        name: this.i18n.get('protection_storage_device_detect_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('storageDeviceDetectType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('storageDeviceDetectType')
        }
      },
      {
        key: 'subType',
        name: this.i18n.get('common_equipment_type_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('cyberDeviceStorageType')
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'username',
        name: this.i18n.get('common_username_label')
      },
      {
        key: 'wwn',
        name: 'WWN',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'esn',
        name: this.i18n.get('common_serial_number_label')
      },
      {
        key: 'operation',
        width: 130,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: getPermissionMenuItem(reject(values(opts), { id: 'create' }))
          }
        }
      }
    ];
    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'uuid',
        columns: cols,
        showLoading: false,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        selectionChange: data => {
          this.selectionData = data;
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: null
    };
    this.optsConfig = getPermissionMenuItem([opts.create]);
  }

  gotoPM(data) {
    const url = `https://${encodeURI(
      get(data, 'endpoint')
    )}:25080/console/#/login`;
    window.open(url, '_blank');
  }

  getData(filters: Filters, args) {
    const params = {
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true,
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    const defaultConditions = {
      type: 'StorageEquipment',
      subType: [['!='], DataMap.Device_Storage_Type.Other.value]
    };

    if (!isEmpty(filters.conditions_v2)) {
      assign(defaultConditions, JSON.parse(filters.conditions_v2));
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            assign(item, {
              endpoint: trim(item['endpoint']) === '0' ? '' : item['endpoint'],
              port: trim(item['port']) === '0' ? '' : item['port'],
              wwn: item.extendInfo?.wwn,
              esn: item.uuid,
              detectType: item.extendInfo?.detectType,
              username: item.auth['authKey']
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.getSummary();
      });
  }

  getSummary() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      akLoading: false,
      conditions: JSON.stringify({
        type: 'StorageEquipment',
        subType: [['!='], DataMap.Device_Storage_Type.Other.value]
      })
    };
    this.protectedResourceApiService
      .ListResources(params)
      .subscribe((res: any) => {
        this.totalFileSystem = res.totalCount;
        this.totalNormal = size(
          filter(
            res.records,
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          )
        );
        this.totalOceanCyber = size(
          filter(
            res.records,
            item =>
              item.extendInfo?.detectType ===
              DataMap.storageDeviceDetectType.cyberEngine.value
          )
        );
        this.totalInDevice = size(
          filter(
            res.records,
            item =>
              item.extendInfo?.detectType ===
              DataMap.storageDeviceDetectType.inDevice.value
          )
        );
      });
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.name)
        }
      ]
    });
    this.dataTable.fetchData();
  }

  addStorage(item?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-storage-modal',
        lvWidth: this.i18n.isEn
          ? MODAL_COMMON.normalWidth + 150
          : MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('protection_add_device_label')
          : this.i18n.get('common_modify_colon_label', [item.name]),
        lvContent: AddStorageDeviceComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          item
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddStorageDeviceComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                this.dataTable.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  deleteStorage(item) {
    this.warningMessageService.create({
      content: this.i18n.get('common_delete_device_label', [item[0].name]),
      onOK: () => {
        this.protectedEnvironmentApiService
          .DeleteProtectedEnvironment({
            envId: item[0].uuid
          })
          .subscribe(() => this.dataTable.fetchData());
      }
    });
  }
}
