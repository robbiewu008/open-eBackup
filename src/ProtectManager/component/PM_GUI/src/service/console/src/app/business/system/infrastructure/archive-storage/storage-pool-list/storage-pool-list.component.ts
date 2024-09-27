import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CAPACITY_UNIT,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  MediaSetApiService,
  OperateItems,
  RoleType,
  WarningMessageService,
  getAccessibleViewList,
  isRBACDPAdmin
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  includes,
  isEmpty,
  isFunction,
  map,
  size
} from 'lodash';
import { Subject, Subscription, combineLatest, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { CreateStoragePoolComponent } from './create-storage-pool/create-storage-pool.component';
import { StoragePoolDetailComponent } from './storage-pool-detail/storage-pool-detail.component';

@Component({
  selector: 'aui-storage-pool-list',
  templateUrl: './storage-pool-list.component.html',
  styleUrls: ['./storage-pool-list.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class StoragePoolListComponent
  implements OnInit, AfterViewInit, OnDestroy {
  optsConfig;
  tableData: TableData;
  tableConfig: TableConfig;
  unitconst = CAPACITY_UNIT;
  timeSub$: Subscription;
  destroy$ = new Subject();
  selectionData = [];
  clusterNode;
  node;
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );

  @ViewChild('poolTable', { static: false }) poolTable: ProTableComponent;
  @ViewChild('capacityTpl', { static: true }) capacityTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private mediaSetApiService: MediaSetApiService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private cookieService: CookieService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    if (!this.isDataBackup) {
      this.poolTable.fetchData();
    }
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_create_label'),
        displayCheck: () => {
          return getAccessibleViewList(this.cookieService.role)[
            OperateItems.CreateArchiveStorage
          ];
        },
        onClick: () => {
          this.createPool();
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        displayCheck: () => {
          return getAccessibleViewList(this.cookieService.role)[
            OperateItems.DeletingArchiveStorage
          ];
        },
        disableCheck: data => {
          return !size(this.selectionData);
        },
        onClick: () => {
          this.deletePools();
        }
      }
    ];
    this.optsConfig = opts;

    const cols: TableCols[] = [
      {
        key: 'mediaSetName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: isRBACDPAdmin(this.cookieService.role)
            ? null
            : {
                id: 'outerClosable',
                iconPos: 'flow-text',
                click: data => {
                  this.getDetail(data);
                }
              }
        }
      },
      {
        key: 'type',
        name: this.i18n.get('system_tape_type_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Media_Pool_Type')
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: false,
          options: this.dataMapService.toArray('Media_Pool_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Media_Pool_Status')
        }
      },
      {
        key: 'usedSize',
        name: this.i18n.get('common_used_capcity_label'),
        thAlign: 'right',
        cellRender: this.capacityTpl
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
                onClick: data => {
                  this.modifyPool(data);
                },
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ModifyingArchiveStorage
                  ];
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_delete_label'),
                onClick: data => {
                  this.deletePool(data);
                },
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.DeletingArchiveStorage
                  ];
                }
              }
            ]
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'mediaSetId',
        columns: cols,
        scrollFixed: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      }
    };
  }

  getData(filters?: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      akOperationTips: false,
      memberEsn: this.node?.remoteEsn
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.mediaSetName) {
        assign(params, { mediaSetName: conditions.mediaSetName });
      }
      if (conditions.type) {
        assign(params, { types: conditions.type });
      }
      if (conditions.status) {
        assign(params, { statuses: conditions.status });
      }
    }

    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }

    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.mediaSetApiService.getMediaSetAllUsingGET({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  getDetail(data) {
    this.mediaSetApiService
      .getMediaSetUsingGET({
        mediaSetId: data.mediaSetId,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        this.drawModalService.create({
          ...MODAL_COMMON.generateDrawerOptions(),
          ...{
            lvWidth: MODAL_COMMON.largeWidth,
            lvOkDisabled: true,
            lvHeader: data.mediaSetName,
            lvContent: StoragePoolDetailComponent,
            lvComponentParams: {
              node: this.node,
              mediaSet: res
            },
            lvFooter: [
              {
                label: this.i18n.get('common_close_label'),
                onClick: modal => {
                  modal.close();
                }
              }
            ]
          }
        });
      });
  }

  createPool(callback?: () => void) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.largeModal,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('common_create_label'),
        lvContent: CreateStoragePoolComponent,
        lvComponentParams: {
          node: this.node
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateStoragePoolComponent;
          const modalIns = modal.getInstance();
          const combined: any = combineLatest([
            content.formGroup.statusChanges,
            content.validSelection$
          ]);
          combined.subscribe(latestValues => {
            const [formGroupStatus, validSelection] = latestValues;
            modalIns.lvOkDisabled = !(
              formGroupStatus === 'VALID' && validSelection
            );
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateStoragePoolComponent;
            content.create().subscribe({
              next: res => {
                resolve(true);
                if (isFunction(callback)) {
                  callback();
                } else {
                  this.poolTable.fetchData();
                }
              },
              error: error => resolve(false)
            });
          });
        }
      }
    });
  }

  modifyPool(data) {
    this.mediaSetApiService
      .getMediaSetUsingGET({
        mediaSetId: data[0].mediaSetId,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        this.drawModalService.create({
          ...MODAL_COMMON.generateDrawerOptions(),
          ...{
            lvWidth: MODAL_COMMON.largeModal,
            lvOkDisabled: true,
            lvHeader: this.i18n.get('common_modify_label'),
            lvContent: CreateStoragePoolComponent,
            lvComponentParams: {
              node: this.node,
              data: res
            },
            lvAfterOpen: modal => {
              const content = modal.getContentComponent() as CreateStoragePoolComponent;
              const modalIns = modal.getInstance();
              const combined: any = combineLatest([
                content.formGroup.statusChanges,
                content.validSelection$
              ]);
              combined.subscribe(latestValues => {
                const [formGroupStatus, validSelection] = latestValues;
                modalIns.lvOkDisabled = !(
                  formGroupStatus === 'VALID' && validSelection
                );
              });
            },
            lvOk: modal => {
              return new Promise(resolve => {
                const content = modal.getContentComponent() as CreateStoragePoolComponent;
                content.modify().subscribe({
                  next: res => {
                    resolve(true);
                    this.poolTable.fetchData();
                  },
                  error: error => resolve(false)
                });
              });
            }
          }
        });
      });
  }

  clearSelection() {
    this.selectionData = [];
    this.poolTable.setSelections([]);
  }

  deletePool(data) {
    this.warningMessageService.create({
      content: this.i18n.get('system_storage_pool_delete_label', [
        data[0].mediaSetName
      ]),
      onOK: () => {
        this.mediaSetApiService
          .deleteMediaSetUsingDELETE({
            mediaSetId: data[0].mediaSetId,
            memberEsn: this.node?.remoteEsn
          })
          .subscribe(res => {
            this.poolTable.fetchData();
            this.clearSelection();
          });
      }
    });
  }

  deletePools() {
    const poolNames = [];
    this.selectionData.forEach(slection => {
      poolNames.push(slection.mediaSetName);
    });
    this.warningMessageService.create({
      content: this.i18n.get('system_storage_pool_delete_label', [
        poolNames.toString()
      ]),
      onOK: () => {
        if (size(this.selectionData) === 1) {
          this.mediaSetApiService
            .deleteMediaSetUsingDELETE({
              mediaSetId: this.selectionData[0].mediaSetId,
              memberEsn: this.node?.remoteEsn
            })
            .subscribe(res => {
              this.poolTable.fetchData();
              this.clearSelection();
            });
          return;
        }
        this.batchOperateService.selfGetResults(
          item => {
            return this.mediaSetApiService.deleteMediaSetUsingDELETE({
              mediaSetId: item.mediaSetId,
              akDoException: false,
              akOperationTips: false,
              akLoading: false,
              memberEsn: this.node?.remoteEsn
            });
          },
          map(cloneDeep(this.selectionData), item => {
            return assign(item, {
              name: item.mediaSetName,
              isAsyn: false
            });
          }),
          () => {
            this.poolTable.fetchData();
            this.clearSelection();
          }
        );
      }
    });
  }
}
