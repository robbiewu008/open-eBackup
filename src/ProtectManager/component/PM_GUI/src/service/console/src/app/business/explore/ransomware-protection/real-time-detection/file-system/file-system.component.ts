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
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  IODETECTFILESYSTEMService,
  IODETECTPOLICYService,
  MODAL_COMMON,
  OperateItems,
  ProtectResourceAction,
  getPermissionMenuItem
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  clone,
  cloneDeep,
  each,
  filter,
  isEmpty,
  isUndefined,
  map,
  size,
  values
} from 'lodash';
import { HoneypotDetailComponent } from './honeypot-detail/honeypot-detail.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { ProtectedResourcePageListResponse } from 'app/shared/api/models';
import { SlaService } from 'app/shared/services/sla.service';

@Component({
  selector: 'aui-real-detection-file-system',
  templateUrl: './file-system.component.html',
  styleUrls: ['./file-system.component.less']
})
export class FileSystemComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems;

  @Output() refreshSummary = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;
  @ViewChild('honeypotTpl', { static: true })
  honeypotTpl: TemplateRef<any>;
  @ViewChild('policyTpl', { static: true })
  policyTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectService: ProtectService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private ioDetectPolicyService: IODETECTPOLICYService,
    private detectFilesystemService: IODETECTFILESYSTEMService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable?.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
    this.getDevice();
  }

  getDevice() {
    this.appUtilsService
      .getCyberEngineStorage()
      .subscribe((res: ProtectedResourcePageListResponse) => {
        const devices = filter(
          res.records,
          item =>
            item.subType !==
            DataMap.cyberDeviceStorageType.OceanStorPacific.value
        );
        const deviceFilterMap = map(devices, item => {
          return {
            key: item.uuid,
            label: item.name,
            value: item.name
          };
        });
        each(this.tableConfig.table.columns, item => {
          if (item.key === 'parentName') {
            assign(item, {
              filter: {
                type: 'select',
                isMultiple: true,
                showCheckAll: true,
                options: deviceFilterMap
              }
            });
          }
        });
        this.dataTable.init();
      });
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      protect: {
        id: 'protect',
        type: 'primary',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  isEmpty(val.policyId) &&
                  val.isIoDetectEnabled !==
                    DataMap.ioDetectEnabled.protected.value
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.Protection,
        label: this.i18n.get('common_protect_label'),
        onClick: data => this.protect(data, ProtectResourceAction.Create)
      },
      modifyProtect: {
        id: 'modifyProtect',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return !isEmpty(val.policyId);
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ModifyProtection,
        label: this.i18n.get('common_resource_protection_modify_label'),
        onClick: data => this.protect(data, ProtectResourceAction.Modify)
      },
      removeProtection: {
        id: 'removeProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  !isEmpty(val.policyId) ||
                  val.isIoDetectEnabled ===
                    DataMap.ioDetectEnabled.protected.value
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.RemoveProtection,
        label: this.i18n.get('protection_remove_protection_label'),
        onClick: data =>
          this.protectService.removeDetectionProtection(data).subscribe(() => {
            this.dataTable.fetchData();
            this.selectionData = [];
            this.dataTable.setSelections([]);
          })
      },
      activeProtection: {
        id: 'activeProtection',
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  !isEmpty(val.policyId) &&
                  val.isIoDetectEnabled ==
                    DataMap.ioDetectEnabled.notProtected.value
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.ActivateProtection,
        label: this.i18n.get('protection_active_protection_label'),
        onClick: data =>
          this.protectService.activeDetectionProtection(data).subscribe(() => {
            this.dataTable.fetchData();
            this.selectionData = [];
            this.dataTable.setSelections([]);
          })
      },
      deactiveProtection: {
        id: 'deactiveProtection',
        divide: true,
        disableCheck: data => {
          return (
            size(
              filter(data, val => {
                return (
                  !isEmpty(val.policyId) &&
                  val.isIoDetectEnabled ==
                    DataMap.ioDetectEnabled.protected.value
                );
              })
            ) !== size(data) || !size(data)
          );
        },
        permission: OperateItems.DeactivateProtection,
        label: this.i18n.get('protection_deactive_protection_label'),
        onClick: data =>
          this.protectService
            .deactiveDetectionProtection(data)
            .subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            })
      }
    };

    this.optItems = cloneDeep(getPermissionMenuItem(values(opts)));

    const cols: TableCols[] = [
      {
        key: 'id',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'fsName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'parentName',
        name: this.i18n.get('protection_storage_device_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: []
        },
        cellRender: this.storageDeviceTpl
      },
      {
        key: 'vstoreName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'honeypotFileNum',
        name: this.i18n.get('explore_honeypot_file_num_label'),
        cellRender: this.honeypotTpl
      },
      {
        key: 'policyName',
        name: this.i18n.get('explore_real_detection_policy_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.policyTpl
      },
      {
        key: 'isIoDetectEnabled',
        name: this.i18n.get('protection_protected_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('ioDetectEnabled')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('ioDetectEnabled')
        }
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
            items: this.optItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'id',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => this.getData(filter, args),
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
          return item.id;
        }
      }
    };

    const removeProtectionBtn = clone(opts.removeProtection);
    removeProtectionBtn.divide = false;
    const deactiveBtn = clone(opts.deactiveProtection);
    deactiveBtn.divide = false;
    this.optsConfig = getPermissionMenuItem([
      opts.protect,
      removeProtectionBtn,
      opts.activeProtection,
      deactiveBtn
    ]);
  }

  getData(filters: Filters, args) {
    const params = {
      pageNum: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.fsName) {
        assign(params, {
          name: conditions.fsName
        });
        delete conditions.fsName;
      }
      if (!isEmpty(conditions.parentName)) {
        assign(params, {
          deviceNames: conditions.parentName
        });
        delete conditions.parentName;
      }
      if (conditions.vstoreName) {
        assign(params, {
          vstoreName: conditions.vstoreName
        });
        delete conditions.vstoreName;
      }
      if (!isEmpty(conditions.isIoDetectEnabled)) {
        assign(params, {
          ioDetectStatus: conditions.isIoDetectEnabled
        });
        delete conditions.isIoDetectEnabled;
      }
      assign(params, conditions);
    }

    if (!isEmpty(filters.sort) && !isEmpty(filters.sort.key)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }

    this.detectFilesystemService
      .pageQueryProtectObject(params)
      .subscribe(res => {
        each(res.records, item => {
          assign(item, {
            isRealDetection: true,
            name: item.fsName,
            uuid: item.id,
            sla_id: item.policyId
          });
        });
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.refreshSummary.emit();
      });
  }

  protect(datas, action: ProtectResourceAction) {
    const data = size(datas) > 1 ? datas : datas[0];
    this.protectService.openProtectModal(
      DataMap.Resource_Type.LocalFileSystem.value,
      action,
      {
        width: 780,
        data: data,
        onOK: () => {
          this.selectionData = [];
          this.dataTable.setSelections([]);
          this.dataTable.fetchData();
        }
      }
    );
  }

  honeypotDetail(rowData) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('explore_honeypot_file_detail_label'),
        lvContent: HoneypotDetailComponent,
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvComponentParams: { rowData },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  policyDetail(rowData) {
    this.ioDetectPolicyService
      .getIoDetectPolicyById({ policyId: rowData?.policyId })
      .subscribe(res => {
        this.slaService.getRealDetectionPolicyDetail(res);
      });
  }
}
