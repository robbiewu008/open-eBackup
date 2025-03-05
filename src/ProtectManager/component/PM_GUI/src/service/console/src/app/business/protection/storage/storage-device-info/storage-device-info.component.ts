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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  getLabelList,
  getPermissionMenuItem,
  GlobalService,
  GROUP_COMMON,
  hasResourcePermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RoleOperationMap,
  SetTagType,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  first,
  includes,
  isEmpty,
  isUndefined,
  map as _map,
  reject,
  size,
  trim,
  values,
  some
} from 'lodash';
import { map } from 'rxjs/operators';
import { AddStorageComponent } from './add-storage/add-storage.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { GetLabelOptionsService } from '../../../../shared/services/get-labels.service';

@Component({
  selector: 'aui-storage-device-info',
  templateUrl: './storage-device-info.component.html',
  styleUrls: ['./storage-device-info.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class StorageDeviceInfoComponent implements OnInit, AfterViewInit {
  name;
  tableConfig: TableConfig;
  tableData = {};
  optsConfig;
  selectionData = [];
  maxDisplayItems = 3;

  groupCommon = GROUP_COMMON;

  isNasSupportType = [
    DataMap.Device_Storage_Type.DoradoV7.value,
    DataMap.Device_Storage_Type.OceanStorDoradoV7.value,
    DataMap.Device_Storage_Type.OceanStorDorado_6_1_3.value,
    DataMap.Device_Storage_Type.OceanStor_6_1_3.value,
    DataMap.Device_Storage_Type.OceanProtect.value
  ];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private batchOperateService: BatchOperateService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private getLabelOptionsService: GetLabelOptionsService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(340);
    this.initConfig();
  }

  onChange() {
    this.ngOnInit();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        type: 'primary',
        label: this.i18n.get('protection_add_device_label'),
        permission: RoleOperationMap.manageResource,
        onClick: () => {
          this.addStorage();
        },
        popoverContent: this.i18n.get('protection_add_storage_tip_label'),
        popoverShow: USER_GUIDE_CACHE_DATA.active
      },
      modify: {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyStorageDevice,
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.addStorage(first(data));
        }
      },
      delete: {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        divide: true,
        permission: OperateItems.DeleteStorageDevice,
        disableCheck: data => {
          return isEmpty(data) || some(data, v => !hasResourcePermission(v));
        },
        onClick: data => {
          this.deleteStorage(data);
        }
      },
      scan: {
        id: 'scan',
        label: this.i18n.get('common_rescan_label'),
        permission: OperateItems.AddStorageDevice,
        disableCheck: data => {
          return isEmpty(data) || some(data, v => !hasResourcePermission(v));
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
      addTag: {
        id: 'addTag',
        permission: OperateItems.AddTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag(data)
      },
      removeTag: {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return !size(data) || some(data, v => !hasResourcePermission(v));
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
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
        }
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
        key: 'subType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('Device_Storage_Type')
            .filter(item => {
              return item.value !== DataMap.Device_Storage_Type.Other.value;
            })
            .filter(item => {
              if (
                includes(
                  [
                    DataMap.Deploy_Type.e6000.value,
                    DataMap.Deploy_Type.decouple.value
                  ],
                  this.i18n.get('deploy_type')
                )
              ) {
                return !includes(this.isNasSupportType, item.value);
              }
              return true;
            })
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Device_Storage_Type')
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
        key: 'labelList',
        name: this.i18n.get('common_tag_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          showSearch: true,
          options: () => this.getLabelOptionsService.getLabelOptions()
        },
        cellRender: this.resourceTagTpl
      },
      {
        key: 'operation',
        width: 144,
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
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        compareWith: 'uuid',
        columns: cols,
        showLoading: false,
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
    this.optsConfig = getPermissionMenuItem([
      opts.create,
      opts.delete,
      opts.addTag,
      opts.removeTag
    ]);
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data,
      type: SetTagType.Resource,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data,
      type: SetTagType.Resource,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      }
    });
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
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.labelList) {
        conditionsTemp.labelList.shift();
        assign(conditionsTemp, {
          labelCondition: {
            labelList: conditionsTemp.labelList
          }
        });
        delete conditionsTemp.labelList;
      }
      assign(defaultConditions, conditionsTemp);
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
            // 获取标签数据
            const { showList, hoverList } = getLabelList(item);
            assign(item, {
              endpoint: trim(item['endpoint']) === '0' ? '' : item['endpoint'],
              port: trim(item['port']) === '0' ? '' : item['port'],
              wwn:
                includes(
                  [
                    DataMap.Device_Storage_Type.DoradoV7.value,
                    DataMap.Device_Storage_Type.OceanStorDoradoV7.value,
                    DataMap.Device_Storage_Type.OceanStorDorado_6_1_3.value,
                    DataMap.Device_Storage_Type.OceanStor_6_1_3.value,
                    DataMap.Device_Storage_Type.OceanStor_v5.value,
                    DataMap.Device_Storage_Type.OceanProtect.value
                  ],
                  item.subType
                ) && trim(item['endpoint']) !== '0'
                  ? item.extendInfo?.wwn
                  : '',
              esn: includes(
                [DataMap.Device_Storage_Type.NetApp.value],
                item.subType
              )
                ? ''
                : item.uuid,
              showLabelList: showList,
              hoverLabelList: hoverList
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
        this.cdr.detectChanges();
        this.getDeviceSummary(
          !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
        );
      });
  }

  getDeviceSummary(loading) {
    this.protectedResourceApiService
      .ListResources({
        akLoading: loading,
        akDoException: false,
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          type: 'StorageEquipment',
          subType: [['!='], DataMap.Device_Storage_Type.Other.value]
        })
      })
      .subscribe(res => {
        this.globalService.emitStore({
          action: 'emitStorage',
          state: {
            StorageEquipment: res.totalCount
          }
        });
        this.cdr.detectChanges();
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
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('protection_add_device_label')
          : this.i18n.get('common_modify_colon_label', [item.name]),
        lvContent: AddStorageComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          item,
          refresh: () => this.dataTable.fetchData()
        }
      })
    );
  }

  deleteStorage(item) {
    if (size(item) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('common_delete_device_label', [item[0].name]),
        onOK: () => {
          this.protectedEnvironmentApiService
            .DeleteProtectedEnvironment({
              envId: item[0].uuid
            })
            .subscribe(() => {
              this.selectionData = reject(
                this.dataTable.getAllSelections(),
                v => {
                  return v.uuid === item[0].uuid;
                }
              );
              this.dataTable.setSelections(this.selectionData);
              this.dataTable.fetchData();
            });
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('common_delete_device_label', [
          _map(item, 'name').join(',')
        ]),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedEnvironmentApiService.DeleteProtectedEnvironment(
                {
                  envId: item.uuid,
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                }
              );
            },
            cloneDeep(this.selectionData),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            }
          );
        }
      });
    }
  }
}
