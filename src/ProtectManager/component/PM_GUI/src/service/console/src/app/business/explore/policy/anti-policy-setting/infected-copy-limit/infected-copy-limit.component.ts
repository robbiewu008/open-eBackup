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
import { DatePipe } from '@angular/common';
import { AfterViewInit, Component, OnInit, ViewChild } from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  extendSlaInfo,
  getPermissionMenuItem,
  hasWormPermission,
  I18NService,
  MODAL_COMMON,
  ProtectedResourceApiService,
  RoleOperationMap,
  RoleType,
  SYSTEM_TIME,
  VirtualResourceService,
  WarningMessageService
} from 'app/shared';
import { AntiRansomwareInfectConfigApiService } from 'app/shared/api/services/anti-ransomware-infect-config-api.service';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  first,
  includes,
  isEmpty,
  isUndefined,
  mapValues,
  omit,
  reject,
  set,
  size
} from 'lodash';
import { AddLimitComponent } from './add-limit/add-limit.component';

@Component({
  selector: 'aui-infected-copy-limit',
  templateUrl: './infected-copy-limit.component.html',
  styleUrls: ['./infected-copy-limit.component.less'],
  providers: [DatePipe]
})
export class InfectedCopyLimitComponent implements OnInit, AfterViewInit {
  optsConfig;
  selectionData = [];
  tableConfig: TableConfig;
  tableData: TableData;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private cookieService: CookieService,
    private messageService: MessageService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private detailService: ResourceDetailService,
    private warningMessageService: WarningMessageService,
    private virtualResourceService: VirtualResourceService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private antiRansomwareInfectedCopyService: AntiRansomwareInfectConfigApiService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      addLimit: {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('explore_copy_limit_add_limit_label'),
        onClick: () => {
          this.configLimit();
        },
        permission: RoleOperationMap.preventExtortionAndWorm,
        displayCheck: data => {
          return !(this.cookieService.role === RoleType.Auditor);
        }
      },
      modifyLimit: {
        id: 'modify',
        type: 'default',
        label: this.i18n.get('explore_copy_limit_modify_limit_label'),
        disableCheck: data => {
          return (
            !data.length ||
            size(
              filter(data, val => {
                return (
                  hasWormPermission(val) ||
                  val.copyType === DataMap.copyTypes.replicate.value
                );
              })
            ) !== size(data)
          );
        },
        onClick: data => {
          this.configLimit(data);
        }
      },
      deleteLimit: {
        id: 'delete',
        type: 'default',
        label: this.i18n.get('explore_copy_limit_delete_limit_label'),
        disableCheck: data => {
          return (
            !data.length ||
            size(
              filter(data, val => {
                return (
                  hasWormPermission(val) ||
                  val.copyType === DataMap.copyTypes.replicate.value
                );
              })
            ) !== size(data)
          );
        },
        onClick: data => {
          this.deleteLimit(data);
        }
      }
    };

    this.optsConfig = getPermissionMenuItem([
      opts.addLimit,
      opts.modifyLimit,
      opts.deleteLimit
    ]);

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
        key: 'resourceName',
        name: this.i18n.get('common_name_label'),
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => {
              this.getDetail(data);
            }
          }
        },
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'resourceSubType',
        name: this.i18n.get('common_resource_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('Detecting_Resource_Type')
            .filter(item => {
              if (this.isHcsUser) {
                return !includes(
                  [DataMap.Detecting_Resource_Type.openstackServer.value],
                  item.value
                );
              }
              return true;
            })
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Detecting_Resource_Type')
        }
      },
      {
        key: 'resourceLocation',
        name: this.i18n.get('common_location_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'copyType',
        name: this.i18n.get('explore_copy_limit_copy_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('copyTypes')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('copyTypes')
        }
      },
      {
        key: 'operations',
        name: this.i18n.get('explore_copy_limit_operation_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('copyDataLimitType')
            .filter(
              item => item.value !== DataMap.copyDataLimitType.replication.value
            )
        }
      },
      {
        key: 'createTime',
        name: this.i18n.get('common_created_time_label'),
        sort: true
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: reject(opts, { id: 'add' })
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'id',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.id;
        }
      }
    };
  }

  getData(filters?: Filters, args?: { isAutoPolling: any }) {
    const params = {
      pageNo: filters?.paginator.pageIndex,
      pageSize: filters?.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (conditionsTemp.resourceSubType) {
        assign(conditionsTemp, {
          resourceSubTypes: [conditionsTemp.resourceSubType]
        });
        delete conditionsTemp.resourceSubType;
      }
      if (conditionsTemp.copyType) {
        assign(conditionsTemp, {
          copyTypes: [conditionsTemp.copyType]
        });
        delete conditionsTemp.copyType;
      }
      assign(params, conditionsTemp);
    }

    if (!!size(filters?.sort)) {
      set(params, filters.sort.direction, [filters.sort.key]);
    }

    this.antiRansomwareInfectedCopyService
      .antiRansomwareInfectedCopyConfigGet(params)
      .subscribe(res => {
        if (!res?.records) {
          return;
        }
        each(res.records, (item: any) => {
          let tmpOps = item.infectedCopyOperations.split(',');
          tmpOps = tmpOps.map(val =>
            this.dataMapService.getLabel('copyDataLimitType', val)
          );
          assign(item, {
            operations: tmpOps.join(','),
            createTime: this.datePipe.transform(
              item.createTime,
              'yyyy/MM/dd HH:mm:ss',
              SYSTEM_TIME.timeZone
            )
          });
        });
        this.tableData = {
          data: res.records || [],
          total: res.totalCount
        };
      });
  }

  configLimit(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-copy-limit',
        lvWidth: MODAL_COMMON.xLargeWidth,
        lvHeader: !!data
          ? this.i18n.get('explore_copy_limit_modify_limit_label')
          : this.i18n.get('explore_copy_limit_add_limit_label'),
        lvContent: AddLimitComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            if (!!data) {
              this.modifyMessage(data, modal, resolve);
            } else {
              this.parseAddLimit(modal, resolve);
            }
          });
        }
      })
    );
  }

  modifyMessage(data: any, modal: any, resolve: (value: unknown) => void) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_copy_limit_modify_limit_tip_label', [
        data.map(item => item.resourceName).join(',')
      ]),
      width: 700,
      onOK: () => {
        this.parseAddLimit(modal, resolve);
      },
      onCancel: () => resolve(false),
      lvAfterClose: result => {
        if (result && result.trigger === 'close') {
          resolve(false);
        }
      }
    });
  }

  parseAddLimit(modal: any, resolve: (value: unknown) => void) {
    const content = modal.getContentComponent() as AddLimitComponent;
    content.onOK().subscribe({
      next: res => {
        resolve(true);
        this.dataTable.fetchData();
        this.selectionData = [];
        this.dataTable.setSelections([]);
        localStorage.setItem('addCopyLimitComplete', '1');
      },
      error: () => {
        resolve(false);
      }
    });
  }

  openDetailWin(res, item) {
    if (!item || isEmpty(item)) {
      this.messageService.error(
        this.i18n.get('common_resource_not_exist_label'),
        {
          lvShowCloseButton: true,
          lvMessageKey: 'resNotExistMesageKey'
        }
      );
      return;
    }
    if (
      includes(
        mapValues(this.drawModalService.modals, 'key'),
        'slaDetailModalKey'
      )
    ) {
      this.drawModalService.destroyModal('slaDetailModalKey');
    }
    if (
      !includes(
        [DataMap.Resource_Type.virtualMachine.value],
        res.resourceSubType
      )
    ) {
      extendSlaInfo(item);
    }
    this.detailService.openDetailModal(res.resourceSubType, {
      data: assign(omit(cloneDeep(res), ['sla_id', 'sla_name']), item)
    });
  }

  getDetail(res) {
    if (res.resourceSubType === DataMap.Resource_Type.virtualMachine.value) {
      this.virtualResourceService
        .queryResourcesV1VirtualResourceGet({
          pageSize: CommonConsts.PAGE_SIZE,
          pageNo: CommonConsts.PAGE_START,
          conditions: JSON.stringify({
            uuid: res.resourceId
          })
        })
        .subscribe(vmRes => this.openDetailWin(res, first(vmRes.items)));
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: res.resourceId })
      .subscribe(item => this.openDetailWin(res, item));
  }

  deleteLimit(data) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_copy_limit_delete_limit_tip_label', [
        data.map(item => item.resourceName).join(',')
      ]),
      width: 700,
      onOK: () => {
        this.antiRansomwareInfectedCopyService
          .antiRansomwareInfectedCopyConfigDelete({
            configDeleteReq: {
              ids: data.map(item => item.id)
            }
          })
          .subscribe(res => {
            this.dataTable.fetchData();
            this.selectionData = [];
            this.dataTable.setSelections([]);
          });
      }
    });
  }
}
