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
  EventEmitter,
  Input,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AntiRansomwareAirgapApiService,
  CommonConsts,
  DataMapService,
  getPermissionMenuItem,
  GROUP_COMMON,
  hasAirgapPermission,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ResourceSetApiService,
  ResourceSetType,
  RoleOperationMap,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  isEmpty,
  isUndefined,
  map,
  set,
  size,
  some,
  trim
} from 'lodash';
import { AirgapDetailComponent } from './airgap-detail/airgap-detail.component';
import { CreateAirgapComponent } from './create-airgap/create-airgap.component';

@Component({
  selector: 'aui-airgap-tactics',
  templateUrl: './airgap-tactics.component.html',
  styleUrls: ['./airgap-tactics.component.less']
})
export class AirgapTactisComponent implements OnInit, AfterViewInit {
  @Input() isResourceSet = false; // 用于资源集判断
  @Input() allSelectionMap;
  @Input() data;
  @Input() isDetail;
  @Output() allSelectChange = new EventEmitter<any>();
  @Output() refreshPolicy = new EventEmitter();

  optsConfig: ProButton[];
  opts: ProButton[];
  selectionData = [];
  tableConfig: TableConfig;
  timeWindow = [];
  tableData: TableData;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('timeTpl', { static: true })
  timeTpl: TemplateRef<any>;
  isAllSelect = false; // 用来标记是否全选
  allSelectDisabled = true;
  buttonLabel = this.i18n.get('system_resourceset_all_select_label');

  groupCommon = GROUP_COMMON;

  constructor(
    public i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private resourceSetService: ResourceSetApiService,
    private warningMessageService: WarningMessageService,
    private antiRansomwareAirgapApiService: AntiRansomwareAirgapApiService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    if (
      this.isResourceSet &&
      !!this.allSelectionMap[ResourceSetType.AirGap]?.isAllSelected
    ) {
      this.isAllSelect = true;
    }
  }
  ngAfterViewInit() {
    this.dataTable.fetchData();
  }
  refresh() {
    this.dataTable.fetchData();
  }
  search(value) {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(value)
        }
      ]
    });
    this.dataTable.fetchData();
  }

  initConfig() {
    const btns: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        permission: this.appUtilsService.isDataBackup
          ? RoleOperationMap.airGap
          : OperateItems.ModifyStorageDevice,
        type: 'primary',
        label: this.i18n.get('common_create_label'),
        onClick: () => {
          this.createAirgap();
        }
      },
      delete: {
        id: 'delete',
        permission: OperateItems.ModifyStorageDevice,
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return (
            data[0].deviceCount !== 0 ||
            (this.appUtilsService.isDataBackup &&
              some(data, item => !hasAirgapPermission(item)))
          );
        },
        onClick: data => {
          this.deleteAirGap(data);
        }
      },
      batchDelete: {
        id: 'batchDelete',
        permission: OperateItems.ModifyStorageDevice,
        disableCheck: data => {
          let params = false;
          if (!size(data)) {
            params = true;
            return true;
          }
          each(data, item => {
            if (item.deviceCount !== 0) {
              params = true;
            }
          });
          return (
            params ||
            (this.appUtilsService.isDataBackup &&
              some(data, item => !hasAirgapPermission(item)))
          );
        },
        label: this.i18n.get('common_delete_label'),
        disabledTips: this.i18n.get('common_not_supported_airgap_label'),
        onClick: data => {
          let params = {
            policyIds: []
          };
          each(data, item => {
            params.policyIds.push(item.id);
          });

          this.warningMessageService.create({
            content: this.i18n.get('protection_delete_air_gap_label'),
            onOK: () => {
              this.antiRansomwareAirgapApiService
                .DeleteAirGapPolicyBatch(params as any)
                .subscribe(() => {
                  this.selectionData = [];
                  this.dataTable.fetchData();
                  this.dataTable.setSelections([]);
                });
            }
          });
        }
      },
      modify: {
        id: 'modify',
        permission: OperateItems.ModifyStorageDevice,
        disableCheck: data => {
          return (
            data[0].deviceCount !== 0 ||
            (this.appUtilsService.isDataBackup &&
              some(data, item => !hasAirgapPermission(item)))
          );
        },
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.createAirgap(data);
        }
      }
    };
    this.optsConfig = getPermissionMenuItem([btns.create, btns.batchDelete]);
    this.opts = getPermissionMenuItem([btns.modify, btns.delete]);
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        cellRender: this.isResourceSet
          ? null
          : {
              type: 'text',
              config: {
                id: 'outerClosable',
                iconPos: 'flow-text',
                click: (data: any) => {
                  this.getAirGapDetail(data, true);
                }
              }
            }
      },
      {
        key: 'freq',
        name: this.i18n.get('protection_frequency_label')
      },
      {
        key: 'time',
        name: this.i18n.get('common_replication_time_window_label'),
        cellRender: this.timeTpl
      },
      {
        key: 'deviceCount',
        name: this.i18n.get('common_number_associated_storage_devices_label'),
        cellRender: this.isResourceSet
          ? null
          : {
              type: 'text',
              config: {
                id: 'outerClosable',
                iconPos: 'flow-text',
                click: (data: any) => {
                  this.getAirGapDetail(data, false);
                }
              }
            }
      },
      {
        key: 'description',
        name: this.i18n.get('common_desc_label')
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
            items: this.opts
          }
        }
      }
    ];

    if (this.isResourceSet) {
      cols.pop();
    }

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        columns: cols,
        compareWith: 'id',
        colDisplayControl: false,
        rows: this.isDetail
          ? null
          : {
              selectionMode: 'multiple',
              selectionTrigger: 'selector',
              showSelector: true
            },
        scrollFixed: true,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
          if (this.isResourceSet) {
            set(this.allSelectionMap, ResourceSetType.AirGap, {
              data: this.selectionData
            });
            this.allSelectChange.emit();
          }
        },
        trackByFn: (index, item) => {
          return item.id;
        }
      }
    };

    if (this.isResourceSet) {
      delete this.tableConfig.table.autoPolling;
    }
  }

  allSelect(turnPage?) {
    // 用于资源集全选资源
    const isAllSelected = !!turnPage ? !this.isAllSelect : this.isAllSelect;
    set(this.allSelectionMap, ResourceSetType.AirGap, { isAllSelected });
    this.selectionData = isAllSelected ? [...this.tableData.data] : [];
    each(this.tableData.data, item => {
      item.disabled = isAllSelected;
    });
    this.dataTable.setSelections(cloneDeep(this.selectionData));
    this.isAllSelect = isAllSelected;
    this.buttonLabel = this.i18n.get(
      isAllSelected
        ? 'system_resourceset_cancel_all_select_label'
        : 'system_resourceset_all_select_label'
    );
    this.allSelectChange.emit();
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  getAirGapDetail(data, isName) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data.name,
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: AirgapDetailComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        data,
        isName
      },
      lvFooter: [
        {
          id: 'close',
          label: this.i18n.get('common_close_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ]
    });
  }

  activateAirGap(data) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_activate_air_gap_label'),
      onOK: () => {
        this.antiRansomwareAirgapApiService
          .StartActivateAirGap({ airGapId: data[0].id })
          .subscribe(() => {
            this.dataTable.fetchData();
          });
      }
    });
  }

  deactivateAirGap(data) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_disable_air_gap_label'),
      onOK: () => {
        this.antiRansomwareAirgapApiService
          .StartDeactivateAirGap({ airGapId: data[0].id })
          .subscribe(() => {
            this.dataTable.fetchData();
          });
      }
    });
  }

  deleteAirGap(data) {
    let params = {
      policyIds: [data[0].id]
    };
    this.warningMessageService.create({
      content: this.i18n.get('protection_delete_air_gap_label'),
      onOK: () => {
        this.antiRansomwareAirgapApiService
          .DeleteAirGapPolicyBatch(params as any)
          .subscribe(() => {
            this.selectionData = [];
            this.dataTable?.setSelections([]);
            this.dataTable.fetchData();
          });
      }
    });
  }

  getData(filters: Filters, args: { isAutoPolling: any }) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.name) {
        conditionsTemp.name.shift();
        assign(params, {
          name: conditionsTemp.name
        });
      }
    }
    if (this.isDetail) {
      assign(params, {
        resourceSetId: this.data[0].uuid
      });
    }
    this.antiRansomwareAirgapApiService
      .ShowPagePolicies(params)
      .subscribe((res: any) => {
        each(res.records, item => {
          this.timeWindow = item.airGapPolicyWindows;
          let frequency = this.i18n.get('common_everyday_label');
          if (item.triggerWeekFreq) {
            if (item.triggerWeekFreq.split(',').length === 7) {
              frequency = this.i18n.get('common_everyday_label');
            } else {
              const time = map(item.triggerWeekFreq.split(','), v => {
                return this.dataMapService.getLabel('Days_Of_Week', v);
              }).join(', ');
              frequency = `${this.i18n.get(
                'common_every_weeks_label'
              )} ${time}`;
            }
          }
          assign(item, {
            freq: frequency
          });
        });

        this.tableData = {
          data: res.records,
          total: res.totalCount
        };

        if (this.isResourceSet) {
          this.allSelectDisabled = res.totalCount === 0;
          if (!!this.allSelectionMap[ResourceSetType.AirGap]?.isAllSelected) {
            this.allSelect(false);
          }
          this.dataCallBack();
        }
        this.refreshPolicy.emit();
      });
  }

  dataCallBack() {
    // 用于资源集各类情况下的资源回显
    if (!isEmpty(this.allSelectionMap[ResourceSetType.AirGap]?.data)) {
      // 重新进入时回显选中的数据
      this.selectionData = cloneDeep(
        this.allSelectionMap[ResourceSetType.AirGap].data
      );
      this.dataTable.setSelections(cloneDeep(this.selectionData));
    }

    if (
      !!this.data &&
      isEmpty(this.allSelectionMap[ResourceSetType.AirGap]?.data) &&
      !this.isDetail
    ) {
      this.getSelectedData();
    }
  }

  getSelectedData() {
    // 用于修改时回显
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: ResourceSetType.AirGap,
      type: ResourceSetType.AirGap
    };
    this.resourceSetService.queryResourceObjectIdList(params).subscribe(res => {
      set(this.allSelectionMap, ResourceSetType.AirGap, {
        data: map(res, item => {
          return { id: item };
        })
      });
      this.selectionData = cloneDeep(
        this.allSelectionMap[ResourceSetType.AirGap].data
      );
      this.dataTable.setSelections(cloneDeep(this.selectionData));
      this.allSelectChange.emit();
    });
  }

  createAirgap(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: !data
        ? this.i18n.get('common_create_label')
        : this.i18n.get('common_modify_label'),
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: CreateAirgapComponent,
      lvOkDisabled: !data,
      lvComponentParams: {
        data
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateAirgapComponent;
        content.formGroup.statusChanges.subscribe(status => {
          modal.getInstance().lvOkDisabled = status !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateAirgapComponent;
          content.onOk().subscribe(
            res => {
              resolve(true);
              this.dataTable.fetchData();
            },
            err => {
              resolve(false);
            }
          );
        });
      }
    });
  }
}
