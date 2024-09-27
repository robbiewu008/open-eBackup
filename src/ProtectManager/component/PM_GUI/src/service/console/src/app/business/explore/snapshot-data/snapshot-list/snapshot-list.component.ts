import { DatePipe } from '@angular/common';
import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  OnDestroy,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageboxService } from '@iux/live';
import {
  ApiStorageBackupPluginService,
  autoTableScroll,
  CAPACITY_UNIT,
  CommonConsts,
  CookieService,
  CopiesDetectReportService,
  CopiesService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  getTableOptsItems,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  SnapshotRetenTion,
  WarningMessageService
} from 'app/shared';
import { ProtectedResourcePageListResponse } from 'app/shared/api/models';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { CopyActionService } from 'app/shared/services/copy-action.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  includes,
  isBoolean,
  isEmpty,
  isNumber,
  isString,
  isUndefined,
  map,
  reject,
  size,
  trim
} from 'lodash';
import { Subject, Subscription, combineLatest, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-ransomware-snapshot-list',
  templateUrl: './snapshot-list.component.html',
  styleUrls: ['./snapshot-list.component.less'],
  providers: [DatePipe],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SnapshotListComponent implements OnInit, OnDestroy {
  @Output() onSelectionChange = new EventEmitter<any>();
  resourceName;
  tenantName;
  snapshotName;
  tableData = [];
  selection = [];
  columns;
  columnSelection = [];
  filter = filter;
  _includes = includes;
  warningConfirm = false;
  _isNumber = isNumber;
  copyStatus = DataMap.snapshotCopyStatus;
  unitconst = CAPACITY_UNIT;

  filterParams = {};
  orders = ['+display_timestamp'];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  optItems: any;

  @Output() refreshSnapshot = new EventEmitter();

  timeSub$: Subscription;
  destroy$ = new Subject();
  isChecked$ = new Subject<boolean>();

  @ViewChild('warningContentTpl', { static: true })
  warningContentTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private appUtilsService: AppUtilsService,
    public virtualScroll: VirtualScrollService,
    private drawModalService: DrawModalService,
    private messageBox: MessageboxService,
    private copyActionService: CopyActionService,
    private systemTimeService: SystemTimeService,
    private warningMessageService: WarningMessageService,
    private copiesDetectReportService: CopiesDetectReportService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(400);
    this.getColumns();
    this.initColumnSelection();
    this.getSnapshot();
    this.initFilterMap();
    autoTableScroll(this.virtualScroll, 400, null, this.cdr);
  }

  initFilterMap() {
    this.appUtilsService
      .getCyberEngineStorage()
      .subscribe((res: ProtectedResourcePageListResponse) => {
        each(this.columns[1].children, col => {
          if (col.key === 'resource_environment_name') {
            col.filterMap = map(res.records, item => {
              return {
                key: item.uuid,
                label: item.name,
                value: item.name
              };
            });
          }
        });
        this.columns = [...this.columns];
        this.cdr.detectChanges();
      });
  }

  getColumns() {
    const copyStatus = this.dataMapService.toArray('detectionSnapshotStatus');
    const antiStatus = this.dataMapService
      .toArray('snapshotCopyStatus')
      .filter(item =>
        includes(
          [
            this.copyStatus.deleteFailed.value,
            this.copyStatus.deleting.value,
            this.copyStatus.restoring.value
          ],
          item.value
        )
      );
    this.columns = [
      {
        label: this.i18n.get('common_snapshot_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'uuid',
            label: this.i18n.get('ID'),
            show: false,
            isLeaf: true
          },
          {
            key: 'name',
            show: false,
            isLeaf: true,
            label: this.i18n.get('protection_hyperdetect_copy_name_label')
          },
          {
            key: 'display_timestamp',
            disabled: true,
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_snapshot_create_time_label'),
            sort: true
          },
          {
            key: 'status',
            label: this.i18n.get('common_status_label'),
            filter: true,
            show: true,
            isLeaf: true,
            filterMap: [...copyStatus, ...antiStatus]
          },
          {
            key: 'generate_type',
            show: true,
            isLeaf: true,
            label: this.i18n.get('common_generated_type_label'),
            filter: true,
            filterMap: this.dataMapService.toArray('snapshotGeneratetype')
          },
          {
            key: 'is_security_snapshot',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_security_snapshot_label')
          },
          {
            key: 'expiration_time',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_snapshot_expire_time_label'),
            filter: true,
            filterMap: this.dataMapService.toArray('snapshotExpiredType')
          },
          {
            key: 'total_file_size',
            show: false,
            isLeaf: true,
            label: this.i18n.get('explore_total_file_size_label')
          },
          {
            key: 'added_file_count',
            show: false,
            isLeaf: true,
            label: this.i18n.get('explore_new_file_num_label')
          },
          {
            key: 'changed_file_count',
            show: false,
            isLeaf: true,
            label: this.i18n.get('explore_modify_file_count_label')
          },
          {
            key: 'deleted_file_count',
            show: false,
            isLeaf: true,
            label: this.i18n.get('explore_delete_file_count_label')
          },
          {
            key: 'infected_file_count',
            show: false,
            isLeaf: true,
            label: this.i18n.get('explore_suspicious_file_num_label')
          },
          {
            key: 'detection_time',
            show: false,
            isLeaf: true,
            label: this.i18n.get('explore_detect_end_time_label')
          }
        ]
      },
      {
        label: this.i18n.get('common_resource_label'),
        expanded: true,
        disabled: true,
        show: true,
        children: [
          {
            key: 'resource_id',
            show: false,
            isLeaf: true,
            label: this.i18n.get('protection_resource_id_label')
          },
          {
            key: 'resource_name',
            disabled: true,
            show: true,
            isLeaf: true,
            label: this.i18n.get('common_name_label')
          },
          {
            key: 'resource_environment_name',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_belong_device_label'),
            filter: true,
            filterMap: []
          },
          {
            key: 'tenant_name',
            show: true,
            isLeaf: true,
            label: this.i18n.get('explore_belong_tenant_label')
          }
        ]
      }
    ];
  }

  initColumnSelection() {
    each(this.columns, column => {
      assign(column, {
        class: 'aui-th-deliver'
      });
      this.columnSelection.push(column);
      if (column.children) {
        this.columnSelection.push(...column.children);
        each(column.children, col => {
          assign(col, {
            class: !includes(['tenant_name'], col.key)
              ? 'timestamp-padding'
              : 'aui-th-deliver'
          });
        });
      }
    });
    this.columnSelection = reject(
      this.columnSelection,
      item => item.isLeaf && !item.show
    );
  }

  columnCheck(source) {
    if (!source.node.children) {
      source.node.show = !source.node.show;
      source.node.parent.show = !!size(
        filter(source.node.parent.children, item => {
          return item.show;
        })
      );
    } else {
      if (
        find(source.node.children, { show: true }) &&
        find(source.node.children, { show: false })
      ) {
        source.node.show = true;
      } else {
        source.node.show = !source.node.show;
      }
      each(source.node.children, item => {
        item.show = source.node.show;
      });
    }
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  getOptItems(data?) {
    const opts: ProButton[] = [
      {
        id: 'detectionResult',
        label: this.i18n.get('explore_detection_result_label'),
        permission: OperateItems.AddReport,
        disableCheck: row => {
          return (
            row &&
            row[0] &&
            (size(
              filter(row, val => {
                return includes(
                  [
                    DataMap.detectionSnapshotStatus.infected.value,
                    DataMap.detectionSnapshotStatus.uninfected.value
                  ],
                  val.anti_status
                );
              })
            ) !== size(row) ||
              !size(row))
          );
        },
        divide: true,
        onClick: ([row]) => this.copyActionService.detectionReport(row)
      },
      {
        id: 'detectImmediately',
        permission: OperateItems.AddReport,
        label: this.i18n.get('explore_detect_immediately_label'),
        disableCheck: row => {
          return (
            row &&
            row[0] &&
            (size(
              filter(row, val => {
                return (
                  !includes(
                    [DataMap.detectionSnapshotStatus.detecting.value],
                    val.anti_status
                  ) &&
                  !includes(
                    [DataMap.snapshotCopyStatus.deleting.value],
                    val.status
                  ) &&
                  !includes(
                    [DataMap.snapshotGeneratetype.ioDetect.value],
                    val.generate_type
                  )
                );
              })
            ) !== size(row) ||
              !size(row))
          );
        },
        onClick: ([row]) => this.copyActionService.manualDetecte(row)
      },
      {
        id: 'errorHandle',
        permission: OperateItems.AddReport,
        label: this.i18n.get('explore_error_feedbac_label'),
        disableCheck: row => {
          return (
            row &&
            row[0] &&
            (size(
              filter(row, val => {
                return includes(
                  [DataMap.detectionSnapshotStatus.infected.value],
                  val.anti_status
                );
              })
            ) !== size(row) ||
              !size(row))
          );
        },
        divide: true,
        onClick: ([row]) =>
          this.copyActionService.dealMisreport(
            assign(
              {
                copyId: row.uuid,
                snapshotTime: this.datePipe.transform(
                  row.generated_time,
                  'yyyy-MM-dd HH:mm:ss'
                )
              },
              row
            ),
            '',
            () => this.getSnapshot()
          )
      },
      {
        id: 'recovery',
        permission: OperateItems.RestoreCopy,
        label: this.i18n.get('common_restore_label'),
        disableCheck: row => {
          return row && row[0] && !size(row);
        },
        onClick: ([row]) => {
          this.restore(row);
        }
      },
      {
        id: 'modifyRetentionPolicy',
        permission: OperateItems.ModifyingCopyRetentionPolicy,
        label: this.i18n.get('common_modify_retention_policy_label'),
        disableCheck: row => {
          return (
            row &&
            row[0] &&
            (!isEmpty(
              find(
                row,
                item =>
                  item.is_security_snapshot &&
                  item.retention_type === SnapshotRetenTion.permanent
              )
            ) ||
              !size(row))
          );
        },
        onClick: ([row]) =>
          this.copyActionService.modifyRetention(row, () => this.getSnapshot())
      },
      {
        id: 'delete',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('common_delete_label'),
        disableCheck: row => {
          return (
            row &&
            row[0] &&
            (!isEmpty(
              find(row, item =>
                includes(
                  [DataMap.detectionSnapshotStatus.detecting.value],
                  item.anti_status
                )
              )
            ) ||
              !size(row))
          );
        },
        onClick: ([row]) => this.deleteSnapshot(row)
      }
    ];
    this.optItems = opts;
    return getPermissionMenuItem(
      getTableOptsItems(cloneDeep(this.optItems), data, this),
      this.cookieService.role
    );
  }

  deleteSnapshot(row) {
    let warnKey = 'common_snapshot_delete_label';
    const deleteFn = () => {
      this.warningMessageService.create({
        content: this.i18n.get(warnKey, [
          this.datePipe.transform(row.generated_time, 'yyyy-MM-dd HH:mm:ss')
        ]),
        onOK: () => {
          this.copiesApiService
            .deleteCopyV1CopiesCopyIdCyberDelete({
              copyId: row.uuid
            })
            .subscribe(() => this.getSnapshot());
        }
      });
    };
    if (row?.anti_status === DataMap.detectionSnapshotStatus.uninfected.value) {
      this.copiesDetectReportService
        .ShowDetectionDetails({
          akDoException: false,
          pageNo: 0,
          pageSize: 20,
          resourceId: row.resource_id,
          conditions: JSON.stringify({
            anti_status: [DataMap.detectionSnapshotStatus.uninfected.value]
          })
        })
        .subscribe(
          res => {
            if (res.total === 1 && res.items[0]?.uuid === row.uuid) {
              warnKey = 'explore_delete_snapshot_warn_label';
              deleteFn();
            } else {
              deleteFn();
            }
          },
          () => deleteFn()
        );
    } else {
      deleteFn();
    }
  }

  getParams() {
    each(this.filterParams, (value, key) => {
      if (isEmpty(value) && !isNumber(value) && !isBoolean(value)) {
        delete this.filterParams[key];
      }
    });
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };
    if (!isEmpty(this.orders)) {
      assign(params, {
        orders: this.orders
      });
    }
    if (!isEmpty(this.filterParams)) {
      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }
    return params as any;
  }

  getSnapshot(name?: string) {
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }
    if (!isUndefined(name)) {
      this.resourceName = name;
      assign(this.filterParams, {
        resource_name: trim(name)
      });
    }
    const params = this.getParams();
    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return combineLatest([
            this.systemTimeService.getSystemTime(!index),
            this.copiesDetectReportService.ShowDetectionDetails({
              ...params,
              akLoading: !index
            })
          ]);
        }),
        takeUntil(this.destroy$)
      )
      .subscribe((res: any) => {
        const [sys, snapshot] = res;
        const sysTime = new Date(
          `${sys.time.replace(/-/g, '/')} ${sys.displayName}`
        ).getTime();
        each(snapshot.items, item => {
          assign(item, {
            snapshotId: JSON.parse(item.properties || '{}')?.snapshotId,
            isExpired:
              item.expiration_time &&
              new Date(item.expiration_time).getTime() < sysTime
          });
        });
        this.tableData = snapshot.items;
        this.total = snapshot.total;
        this.refreshSnapshot.emit();
        this.cdr.detectChanges();
      });
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getSnapshot();
  }

  searchByResourceName(resourceName) {
    assign(this.filterParams, {
      resource_name: trim(resourceName)
    });
    this.getSnapshot();
  }

  searchByTenantName(tenantName) {
    assign(this.filterParams, {
      tenant_name: trim(tenantName)
    });
    this.getSnapshot();
  }

  searchBySnapshotName(snapshotName) {
    assign(this.filterParams, {
      name: trim(snapshotName)
    });
    this.getSnapshot();
  }

  sortChange(source) {
    this.orders = [];
    this.orders.push((source.direction === 'asc' ? '+' : '-') + source.key);
    this.getSnapshot();
  }

  filterChange(e) {
    const restoringMap = [
      DataMap.snapshotCopyStatus.mounting.value,
      DataMap.snapshotCopyStatus.mounted.value,
      DataMap.snapshotCopyStatus.unmounting.value
    ];
    if (e.key === 'status') {
      let statusArr = e.value;
      if (includes(statusArr, DataMap.snapshotCopyStatus.restoring.value)) {
        statusArr = [...statusArr, ...restoringMap];
      }
      assign(this.filterParams, {
        copy_status: filter(statusArr, item => isString(item)),
        anti_status: filter(statusArr, item => !isString(item))
      });
    } else if (e.key === 'expiration_time') {
      if (size(e.value) === 1) {
        assign(this.filterParams, {
          expired: `${first(e.value)}`
        });
      } else {
        delete this.filterParams['expired'];
      }
    } else {
      assign(this.filterParams, {
        [e.key]: e.value
      });
    }
    this.getSnapshot();
  }

  selectionChange() {
    this.onSelectionChange.emit(this.selection);
  }

  restore(row) {
    if (row.file_sub_type === 2) {
      this.apiStorageBackupPluginService
        .QueryPacificFileSystemWormPolicy({
          fileSystemId: row.resource_id
        })
        .subscribe(res => {
          if (res === 'true') {
            this.messageBox.confirm({
              lvContent: this.i18n.get(
                'common_cyberengine_worm_disrestore_label'
              ),
              lvOk: () => {
                this.getSnapshot();
                return;
              }
            });
          } else {
            if (
              row?.anti_status ===
              DataMap.detectionSnapshotStatus.infected.value
            ) {
              this.warningMessageService.create({
                content: this.i18n.get(
                  'common_snapshot_restore_danger_tips_label',
                  [row.detection_time]
                ),
                width: MODAL_COMMON.smallWidth + 100,
                onOK: () => {
                  this.copyActionService.snapshotRestore(row);
                }
              });
            } else {
              this.copyActionService.snapshotRestore(row);
            }
          }
        });
    } else if (row.file_sub_type === 1) {
      this.messageBox.confirm(
        this.i18n.get('explore_worm_recovery_tip_label', [row.resource_name])
      );
    } else {
      if (row?.anti_status === DataMap.detectionSnapshotStatus.infected.value) {
        this.warningMessageService.create({
          content: this.i18n.get('common_snapshot_restore_danger_tips_label', [
            row.detection_time || row.name
          ]),
          width: MODAL_COMMON.smallWidth + 100,
          onOK: () => {
            this.copyActionService.snapshotRestore(row);
          }
        });
      } else {
        this.copyActionService.snapshotRestore(row);
      }
    }
  }
  modalCheckBoxChange(e) {
    this.isChecked$.next(e);
  }

  getSnapshotDetail(rowData) {
    if (!rowData.display_timestamp) {
      return;
    }
    this.copyActionService.getSnapshotDetail(
      this.datePipe.transform(
        rowData?.display_timestamp,
        'yyyy-MM-dd HH:mm:ss'
      ),
      rowData
    );
  }

  trackByUuid = (_, item) => {
    return item.uuid;
  };
}
