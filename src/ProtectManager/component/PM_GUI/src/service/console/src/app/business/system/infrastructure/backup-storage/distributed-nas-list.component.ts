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
  EventEmitter,
  Input,
  OnDestroy,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CAPACITY_UNIT,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  DefaultRoles,
  getAccessibleViewList,
  I18NService,
  MODAL_COMMON,
  NasDistributionStoragesApiService,
  OperateItems,
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
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  isEmpty,
  isUndefined,
  map,
  remove,
  size
} from 'lodash';
import { Subject, Subscription } from 'rxjs';
import { CreateDistributedNasComponent } from './create-distributed-nas/create-distributed-nas.component';
import { ClusterDetailComponent } from './distributed-nas-detail/cluster-detail.component';

@Component({
  selector: 'aui-distributed-nas-list',
  templateUrl: './distributed-nas-list.component.html',
  styleUrls: ['./distributed-nas-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class DistributedNasListComponent
  implements OnInit, AfterViewInit, OnDestroy {
  @Input() isRbac = false;
  @Input() data;
  @Input() resourceSetMap;
  @Input() isGeneral = false;
  @Input() isDetail = false;
  @Output() rbacSelectChange = new EventEmitter<any>();
  optsConfig;
  selectionData = [];
  tableData: TableData;
  timeSub$: Subscription;
  destroy$ = new Subject();
  tableConfig: TableConfig;
  unitconst = CAPACITY_UNIT;
  activeIndex: string = 'unitGroup';
  dataMap = DataMap;
  specialDefaultRoleIdList = [
    DefaultRoles.rdAdmin.roleId,
    DefaultRoles.drAdmin.roleId,
    DefaultRoles.audit.roleId,
    DefaultRoles.sysAdmin.roleId
  ];

  isDistributed =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.e6000.value;
  isDecouple = this.appUtilsService.isDecouple;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('deviceTypeTPl', { static: true }) deviceTypeTPl: TemplateRef<any>;
  SetStoragePolicyComponent: any;
  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private nasDistributionStoragesApiService: NasDistributionStoragesApiService,
    private cookieService: CookieService,
    public dataMapService: DataMapService,
    public appUtilsService?: AppUtilsService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    if (this.appUtilsService.isJumpToStorageUnits) {
      this.jumpToStorageUnits();
    }
    this.initConfig();
  }

  ngAfterViewInit() {
    if (
      !this.isGeneral &&
      !(
        this.isDetail &&
        isEmpty(
          this.resourceSetMap.get(
            DataMap.storagePoolBackupStorageType.group.value
          )
        )
      )
    ) {
      this.dataTable.fetchData();
    }
    if (
      this.isRbac &&
      this.resourceSetMap.has(DataMap.storagePoolBackupStorageType.group.value)
    ) {
      const tmpData = this.resourceSetMap.get(
        DataMap.storagePoolBackupStorageType.group.value
      );
      this.dataTable.setSelections(tmpData);
      if (this.isGeneral) {
        this.tableData = {
          data: tmpData,
          total: size(tmpData)
        };
      }
    }
  }

  jumpToStorageUnits() {
    this.appUtilsService.isJumpToStorageUnits = false;
    this.activeIndex = 'unit';
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_create_label'),
        displayCheck: () => {
          return getAccessibleViewList(this.cookieService.role)[
            OperateItems.AddBackupStorage
          ];
        },
        onClick: () => {
          this.createNas();
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        displayCheck: () => {
          return getAccessibleViewList(this.cookieService.role)[
            OperateItems.DeleteBackupStorage
          ];
        },
        disableCheck: data => {
          return !size(this.selectionData);
        },
        onClick: () => {
          this.deleteNass();
        }
      }
    ];
    this.optsConfig = opts;

    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: 'uuid',
        hidden: true
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.isRbac
          ? null
          : {
              type: 'text',
              config: {
                id: 'outerClosable',
                iconPos: 'flow-text',
                click: data => {
                  this.getDetail(data);
                }
              }
            }
      },
      {
        key: 'description',
        name: this.i18n.get('common_desc_label')
      },
      {
        key: 'deviceType',
        name: this.i18n.get('common_equipment_type_label'),
        width: 150,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('poolStorageDeviceType')
            .filter(
              item =>
                item.value !== DataMap.poolStorageDeviceType.OceanPacific.value
            )
        },
        cellRender: this.deviceTypeTPl
      },
      {
        key: 'unitCount',
        name: this.i18n.get('system_backup_storage_unit_count_label'),
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => {
              this.getDetail(data);
            }
          }
        }
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        width: 144,
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'modify',
                label: this.i18n.get('common_modify_label'),
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ModifyBackupStorage
                  ];
                },
                onClick: data => {
                  this.modifyNas(data);
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_delete_label'),
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.DeleteBackupStorage
                  ];
                },
                onClick: data => {
                  this.deleteNas(data);
                }
              }
            ]
          }
        }
      }
    ];

    if (!this.isDecouple && !this.isDistributed) {
      cols.splice(3, 1);
    }

    if (this.isRbac) {
      remove(cols, { key: 'operation' });
    }

    this.tableConfig = {
      table: {
        async: !this.isRbac,
        compareWith: 'uuid',
        columns: cols,
        scrollFixed: true,
        autoPolling: this.isRbac ? null : CommonConsts.TIME_INTERVAL,
        rows:
          this.isGeneral || this.isDetail
            ? null
            : {
                selectionMode: 'multiple',
                selectionTrigger: 'selector',
                showSelector: true
              },
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: (renderSelection, selection) => {
          this.selectionData = selection;
          if (this.isRbac) {
            this.rbacSelectionChange(
              selection,
              DataMap.storagePoolBackupStorageType.group.value
            );
          }
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };
  }

  rbacSelectionChange(e, type) {
    this.resourceSetMap.set(type, e);
  }

  onChange() {
    this.ngOnInit();
  }

  getData(filters: Filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.name) {
        assign(params, { name: conditions.name });
      }

      if (conditions.deviceType) {
        assign(params, { deviceType: conditions.deviceType });
      }
    }

    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }

    this.nasDistributionStoragesApiService
      .ListNasDistributionStorages(params)
      .subscribe(res => {
        // 接口暂时没法直接过滤，所以一次性获取后前端过滤
        if (this.isDetail) {
          res.records = filter(res.records, item => {
            return !!find(
              this.resourceSetMap.get(
                DataMap.storagePoolBackupStorageType.group.value
              ),
              { uuid: item.uuid }
            );
          });
        }
        this.tableData = {
          data: res.records || [],
          total: this.isDetail ? size(res.records) : res.totalCount
        };
        // rbac可能不会重新选取，那么所选项里面的数据需要更新，因为那个接口返回值不够多
        if (
          this.isRbac &&
          this.resourceSetMap.has(
            DataMap.storagePoolBackupStorageType.group.value
          )
        ) {
          each(
            this.resourceSetMap.get(
              DataMap.storagePoolBackupStorageType.group.value
            ),
            item => {
              const tmpData = find(res.records, { uuid: item.uuid });
              assign(item, tmpData);
            }
          );
        }
        this.cdr.detectChanges();
      });
  }

  getDetail(item) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'distributed-nas-list',
        lvHeader: item.name,
        lvWidth: MODAL_COMMON.xLargeWidth,
        lvContent: ClusterDetailComponent,
        lvComponentParams: {
          data: item
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

  createNas(callback?: () => void) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvModalKey: 'createStep1',
        lvWidth: MODAL_COMMON.xLargeWidth + 200,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('common_create_label'),
        lvContent: CreateDistributedNasComponent,
        lvFooter: [
          {
            label: this.i18n.get('common_next_label'),
            type: 'primary',
            disabled: true,
            onClick: modal => {
              const content = modal.getContentComponent() as CreateDistributedNasComponent;
              content.next(...[, () => !isUndefined(callback) && callback()]);
            }
          },
          {
            label: this.i18n.get('common_cancel_label'),
            onClick: modal => {
              modal.close();
            }
          }
        ],
        lvComponentParams: {
          dataTable: this.dataTable
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateDistributedNasComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(latestValues => {
            modalIns.lvOkDisabled = !(latestValues === 'VALID');
            if (content.activeStep === 1) {
              if (content.formGroup.value.hasEnableParallelStorage) {
                modal.setProperty({
                  lvFooter: [
                    {
                      type: 'primary',
                      label: this.i18n.get('common_ok_label'),
                      disabled: !(latestValues === 'VALID'),
                      onClick: modal => {
                        content.create().subscribe(() => {
                          this.dataTable?.fetchData();
                          !isUndefined(callback) && callback();
                        });
                        modal.close();
                      }
                    },
                    {
                      label: this.i18n.get('common_cancel_label'),
                      onClick: modal => {
                        modal.close();
                      }
                    }
                  ]
                });
              } else {
                modal.setProperty({
                  lvFooter: [
                    {
                      type: 'primary',
                      label: this.i18n.get('common_next_label'),
                      disabled: !(latestValues === 'VALID'),
                      onClick: modal => {
                        const content = modal.getContentComponent() as CreateDistributedNasComponent;
                        content.next(
                          ...[, () => !isUndefined(callback) && callback()]
                        );
                      }
                    },
                    {
                      label: this.i18n.get('common_cancel_label'),
                      onClick: modal => {
                        modal.close();
                      }
                    }
                  ]
                });
              }
            }
          });
        }
      }
    });
  }

  modifyNas(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.xLargeWidth + 200,
        lvOkDisabled: false,
        lvHeader: this.i18n.get('common_modify_label'),
        lvContent: CreateDistributedNasComponent,
        lvComponentParams: {
          isEdit: true,
          rowData: first(data),
          dataTable: this.dataTable
        },
        lvFooter: [
          {
            label: this.i18n.get('common_next_label'),
            type: 'primary',
            disabled: true,
            onClick: modal => {
              const content = modal.getContentComponent() as CreateDistributedNasComponent;
              content.next();
            }
          },
          {
            label: this.i18n.get('common_cancel_label'),
            onClick: modal => {
              modal.close();
            }
          }
        ],
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateDistributedNasComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(latestValues => {
            modalIns.lvOkDisabled = !(latestValues === 'VALID');
            if (content.activeStep === 1) {
              if (content.formGroup.value.hasEnableParallelStorage) {
                modal.setProperty({
                  lvFooter: [
                    {
                      type: 'primary',
                      label: this.i18n.get('common_ok_label'),
                      disabled: !(latestValues === 'VALID'),
                      onClick: modal => {
                        content
                          .modify()
                          .subscribe(() => this.dataTable.fetchData());
                        modal.close();
                      }
                    },
                    {
                      label: this.i18n.get('common_cancel_label'),
                      onClick: modal => {
                        modal.close();
                      }
                    }
                  ]
                });
              } else {
                modal.setProperty({
                  lvFooter: [
                    {
                      type: 'primary',
                      label: this.i18n.get('common_next_label'),
                      disabled: !(latestValues === 'VALID'),
                      onClick: modal => {
                        const content = modal.getContentComponent() as CreateDistributedNasComponent;
                        content.next(
                          content.formGroup.value.hasEnableParallelStorage
                        );
                      }
                    },
                    {
                      label: this.i18n.get('common_cancel_label'),
                      onClick: modal => {
                        modal.close();
                      }
                    }
                  ]
                });
              }
            }
          });
        }
      }
    });
  }
  deleteNas(data) {
    this.warningMessageService.create({
      content: this.i18n.get('system_distributed_nas_delete_label', [
        data[0].name
      ]),
      onOK: () => {
        this.nasDistributionStoragesApiService
          .DeleteNasDistributionStorage({ id: data[0].uuid })
          .subscribe(res => {
            this.dataTable.fetchData();
            this.selectionData = [];
          });
      }
    });
  }

  deleteNass() {
    this.warningMessageService.create({
      content: this.i18n.get('system_distributed_nas_delete_label', [
        map(this.selectionData, item => item.name).join(',')
      ]),
      onOK: () => {
        if (size(this.selectionData) === 1) {
          this.nasDistributionStoragesApiService
            .DeleteNasDistributionStorage({ id: this.selectionData[0].uuid })
            .subscribe(res => {
              this.dataTable.fetchData();
              this.selectionData = [];
            });
          return;
        }
        this.batchOperateService.selfGetResults(
          item => {
            return this.nasDistributionStoragesApiService.DeleteNasDistributionStorage(
              {
                id: item.uuid,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              }
            );
          },
          map(cloneDeep(this.selectionData), item => {
            return assign(item, {
              isAsyn: false
            });
          }),
          () => {
            this.dataTable.fetchData();
            this.selectionData = [];
          }
        );
      }
    });
  }
}
