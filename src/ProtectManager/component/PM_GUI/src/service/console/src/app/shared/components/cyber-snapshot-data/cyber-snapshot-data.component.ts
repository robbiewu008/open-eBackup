import { DatePipe } from '@angular/common';
import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  Input,
  OnChanges,
  OnInit,
  SimpleChanges,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageboxService } from '@iux/live';
import { SnapshotListComponent } from 'app/business/explore/snapshot-data/snapshot-list/snapshot-list.component';
import {
  ApiStorageBackupPluginService,
  CopiesDetectReportService,
  CopiesService
} from 'app/shared/api/services';
import { CAPACITY_UNIT, CommonConsts, DataMap } from 'app/shared/consts';
import {
  CookieService,
  DataMapService,
  I18NService,
  WarningMessageService
} from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { CopyActionService } from 'app/shared/services/copy-action.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  includes,
  isEmpty,
  isNumber,
  isString,
  isUndefined,
  reject,
  size
} from 'lodash';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from '../pro-table';
import { SystemTimeService } from 'app/shared/services/system-time.service';

@Component({
  selector: 'aui-cyber-snapshot-data',
  templateUrl: './cyber-snapshot-data.component.html',
  styleUrls: ['./cyber-snapshot-data.component.less'],
  providers: [DatePipe]
})
export class CyberSnapshotDataComponent
  implements OnInit, AfterViewInit, OnChanges {
  @Input() id;
  @Input() rowData;
  @Input() currentDate;
  isHistory: any;
  status: any;

  tableConfig: TableConfig;
  tableData: TableData;
  snapshotListComponent: SnapshotListComponent;
  copyStatus = DataMap.snapshotCopyStatus;
  _includes = includes;
  _isNumber = isNumber;
  unitconst = CAPACITY_UNIT;
  optItems: any[];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('snapshotHelpTpl', { static: true }) snapshotHelpTpl: TemplateRef<
    any
  >;
  @ViewChild('addFileCountTpl', { static: true }) addFileCountTpl: TemplateRef<
    any
  >;
  @ViewChild('changeFileCountTpl', { static: true })
  changeFileCountTpl: TemplateRef<any>;
  @ViewChild('deleteFileCountTpl', { static: true })
  deleteFileCountTpl: TemplateRef<any>;
  @ViewChild('infectedFileCountTpl', { static: true })
  infectedFileCountTpl: TemplateRef<any>;
  @ViewChild('snapshotrestoreHelpTpl', { static: true })
  snapshotrestoreHelpTpl: TemplateRef<any>;
  @ViewChild('statusTpl', { static: true }) statusTpl: TemplateRef<any>;
  @ViewChild('timeTpl', { static: true }) timeTpl: TemplateRef<any>;
  @ViewChild('expirationTimeTpl', { static: true })
  expirationTimeTpl: TemplateRef<any>;
  @ViewChild('securityTpl', { static: true }) securityTpl: TemplateRef<any>;
  @ViewChild('totalFileSizeTpl', { static: true })
  totalFileSizeTpl: TemplateRef<any>;

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

  ngOnChanges(changes: SimpleChanges): void {
    if (changes.id?.currentValue && this.dataTable) {
      this.dataTable.fetchData();
    }
    if (changes.currentDate?.currentValue && this.dataTable) {
      this.dataTable.fetchData();
    }
  }

  ngAfterViewInit() {
    if (this.dataTable) {
      this.dataTable.fetchData();
    }
  }

  ngOnInit() {
    this.getComponent();
    this.initTableConfig();
  }

  getComponent() {
    this.snapshotListComponent = new SnapshotListComponent(
      this.i18n,
      this.datePipe,
      this.cdr,
      this.cookieService,
      this.dataMapService,
      this.copiesApiService,
      this.appUtilsService,
      this.virtualScroll,
      this.drawModalService,
      this.messageBox,
      this.copyActionService,
      this.systemTimeService,
      this.warningMessageService,
      this.copiesDetectReportService,
      this.apiStorageBackupPluginService
    );
    this.snapshotListComponent.getOptItems();
    this.optItems = cloneDeep(this.snapshotListComponent.optItems);
    if (this.isHistory) {
      this.optItems = filter(this.optItems, (item: any) => {
        item.divide = false;
        return includes(
          ['detectionResult', 'errorHandle', 'recovery'],
          item.id
        );
      });
    }
  }

  initTableConfig() {
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
    let cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('ID'),
        hidden: true
      },
      {
        key: 'name',
        name: this.i18n.get('protection_hyperdetect_copy_name_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'display_timestamp',
        name: this.i18n.get('explore_snapshot_create_time_label'),
        cellRender: this.timeTpl,
        sort: true,
        hidden: 'ignoring'
      },
      {
        key: 'generate_type',
        name: this.i18n.get('common_generated_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('snapshotGeneratetype')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('snapshotGeneratetype')
        }
      },
      {
        key: 'status',
        name: this.isHistory
          ? this.i18n.get('explore_safe_status_label')
          : this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.isHistory
            ? this.dataMapService
                .toArray('detectionSnapshotStatus')
                .filter(item =>
                  includes(
                    [
                      DataMap.detectionSnapshotStatus.infected.value,
                      DataMap.detectionSnapshotStatus.uninfected.value
                    ],
                    item.value
                  )
                )
            : [...copyStatus, ...antiStatus]
        },
        cellRender: this.statusTpl,
        hidden: 'ignoring'
      },
      {
        key: 'is_security_snapshot',
        name: this.i18n.get('explore_security_snapshot_label'),
        thExtra: this.snapshotHelpTpl,
        cellRender: this.securityTpl
      },
      {
        key: 'expiration_time',
        name: this.i18n.get('explore_snapshot_expire_time_label'),
        cellRender: this.expirationTimeTpl
      },
      {
        key: 'total_file_size',
        name: this.i18n.get('explore_total_file_size_label'),
        cellRender: this.totalFileSizeTpl,
        hidden: !this.isHistory,
        sort: !!this.isHistory
      },
      {
        key: 'added_file_count',
        name: this.i18n.get('explore_new_file_num_label'),
        hidden: !this.isHistory,
        sort: !!this.isHistory,
        thExtra: this.addFileCountTpl
      },
      {
        key: 'changed_file_count',
        name: this.i18n.get('explore_modify_file_count_label'),
        hidden: !this.isHistory,
        sort: !!this.isHistory,
        thExtra: this.changeFileCountTpl
      },
      {
        key: 'deleted_file_count',
        name: this.i18n.get('explore_delete_file_count_label'),
        hidden: !this.isHistory,
        sort: !!this.isHistory,
        thExtra: this.deleteFileCountTpl
      },
      {
        key: 'infected_file_count',
        name: this.i18n.get('explore_suspicious_file_num_label'),
        hidden: !this.isHistory,
        sort: !!this.isHistory,
        thExtra: this.infectedFileCountTpl
      },
      {
        key: 'detection_time',
        name: this.i18n.get('explore_detect_end_time_label'),
        hidden: !this.isHistory,
        sort: !!this.isHistory
      },
      {
        key: 'operation',
        width: 110,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        thExtra: this.snapshotrestoreHelpTpl,
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.optItems
          }
        }
      }
    ];

    if (this.isHistory) {
      cols = reject(cols, item =>
        includes(['is_security_snapshot', 'expiration_time'], item.key)
      );
    }

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        size: 'small',
        compareWith: 'uuid',
        columns: cols,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filters: Filters, args) => this.getData(filters, args),
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'default',
        pageSize: CommonConsts.PAGE_SIZE,
        showPageSizeOptions: true,
        winTablePagination: true
      }
    };
  }

  getData(filters: Filters, args) {
    const params: any = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      resourceId: this.rowData?.uuid || this.id,
      orders: ['+display_timestamp'],
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    const defaultParams = {};
    const restoringMap = [
      DataMap.snapshotCopyStatus.mounting.value,
      DataMap.snapshotCopyStatus.mounted.value,
      DataMap.snapshotCopyStatus.unmounting.value
    ];
    if (this.currentDate) {
      assign(defaultParams, {
        date: this.datePipe.transform(this.currentDate, 'yyyy-MM-dd')
      });
    }
    if (!isEmpty(this.status)) {
      assign(defaultParams, {
        anti_status: this.status
      });
    }
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.status) {
        let statusArr = conditions.status;
        if (includes(statusArr, DataMap.snapshotCopyStatus.restoring.value)) {
          statusArr = [...statusArr, ...restoringMap];
        }
        assign(defaultParams, {
          copy_status: filter(statusArr, item => isString(item)),
          anti_status: filter(statusArr, item => !isString(item))
        });
        delete conditions.status;
      }
      assign(defaultParams, conditions);
    }
    each(defaultParams, (value, key) => {
      if (isEmpty(value) && !isNumber(value)) {
        delete defaultParams[key];
      }
    });
    if (!isEmpty(defaultParams)) {
      assign(params, {
        conditions: JSON.stringify(defaultParams)
      });
    }
    if (!!size(filters.sort) && !isEmpty(filters.orders)) {
      assign(params, { orders: filters.orders });
    }
    this.copiesDetectReportService
      .ShowDetectionDetails(params)
      .subscribe(res => {
        each(res.items, item => {
          assign(item, {
            snapshotId: JSON.parse(item.properties || '{}')?.snapshotId
          });
        });
        this.tableData = {
          data: res.items,
          total: res.total
        };
      });
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
}
