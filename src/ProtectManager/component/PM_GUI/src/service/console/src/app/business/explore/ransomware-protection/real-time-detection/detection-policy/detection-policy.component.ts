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
  DataMapService,
  I18NService,
  IODETECTPOLICYService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService,
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
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SlaService } from 'app/shared/services/sla.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  find,
  includes,
  isEmpty,
  isUndefined,
  map,
  reject,
  size,
  values
} from 'lodash';
import { AssociatedFileSystemComponent } from './associated-file-system/associated-file-system.component';
import { AssociatedWhiteListComponent } from './associated-white-list/associated-white-list.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-real-detection-policy',
  templateUrl: './detection-policy.component.html',
  styleUrls: ['./detection-policy.component.less']
})
export class DetectionPolicyComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems;

  @Output() refreshSummary = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('fileSystemCount', { static: true })
  fileSystemCount: TemplateRef<any>;
  @ViewChild('whiteListCount', { static: true })
  whiteListCount: TemplateRef<any>;
  @ViewChild('periodTpl', { static: true })
  periodTpl: TemplateRef<any>;
  @ViewChild('retentionTol', { static: true })
  retentionTol: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService,
    private ioDetectPolicyService: IODETECTPOLICYService,
    private warningMessageService: WarningMessageService
  ) {}

  ngAfterViewInit(): void {
    // 由资源页面跳转过来
    const policyName = this.appUtilsService.getCacheValue(
      'ioDetectionPolicyName',
      false
    );
    if (policyName) {
      this.dataTable.setFilterMap(
        assign(this.dataTable.filterMap, {
          filters: [
            {
              caseSensitive: false,
              key: 'name',
              value: policyName
            }
          ]
        })
      );
    }
    this.dataTable?.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        type: 'primary',
        permission: OperateItems.CreateSLA,
        label: this.i18n.get('common_create_label'),
        onClick: () => this.create()
      },
      clone: {
        id: 'clone',
        disableCheck: data => {
          return !size(data);
        },
        permission: OperateItems.ModifySLA,
        label: this.i18n.get('common_clone_label'),
        onClick: ([data]) => this.create(data, true)
      },
      modify: {
        id: 'modify',
        disableCheck: data => {
          return !size(data);
        },
        permission: OperateItems.ModifySLA,
        label: this.i18n.get('common_modify_label'),
        onClick: ([data]) => this.create(data)
      },
      delete: {
        id: 'delete',
        disableCheck: data => {
          return !size(data);
        },
        permission: OperateItems.DeleteSLA,
        label: this.i18n.get('common_delete_label'),
        onClick: data => this.delete(data)
      }
    };

    this.optItems = cloneDeep(
      getPermissionMenuItem(
        reject(values(opts), item => includes(['create'], item.id))
      )
    );

    this.optsConfig = getPermissionMenuItem([opts.create, opts.delete]);

    const cols: TableCols[] = [
      {
        key: 'id',
        name: this.i18n.get('protection_id_label'),
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
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => this.getDetail(data)
          }
        }
      },
      {
        key: 'retentionDuration',
        name: this.i18n.get('explore_snapshot_lock_time_label'),
        sort: true,
        cellRender: this.retentionTol
      },
      {
        key: 'isIoEnhancedEnabled',
        name: this.i18n.get('explore_alarm_analysis_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('switchStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('switchStatus')
        }
      },
      {
        key: 'isHoneypotDetectEnable',
        name: this.i18n.get('explore_decoy_detection_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('switchStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('switchStatus')
        }
      },
      {
        key: 'period',
        name: this.i18n.get('explore_decoy_update_frequency_label'),
        sort: true,
        cellRender: this.periodTpl
      },
      {
        key: 'associationFsNum',
        name: this.i18n.get('explore_associate_file_system_label'),
        sort: true,
        cellRender: this.fileSystemCount
      },
      {
        key: 'whiteListNum',
        name: this.i18n.get('explore_associated_whitelist_count_label'),
        sort: true,
        cellRender: this.whiteListCount
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
      if (conditions.isHoneypotDetectEnable) {
        assign(params, {
          honeypotDetectStatus: conditions.isHoneypotDetectEnable
        });
        delete conditions.isHoneypotDetectEnable;
      }
      if (conditions.isIoEnhancedEnabled) {
        assign(params, {
          ioEnhancedStatus: conditions.isIoEnhancedEnabled
        });
        delete conditions.isIoEnhancedEnabled;
      }
      assign(params, conditions);
    }

    if (!isEmpty(filters.sort) && !isEmpty(filters.sort.key)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }

    const policyName = this.appUtilsService.getCacheValue(
      'ioDetectionPolicyName'
    );
    this.ioDetectPolicyService
      .pageQueryIoDetectPolicy(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.refreshSummary.emit();
        if (policyName && find(res.records, item => item.name === policyName)) {
          this.getDetail(find(res.records, item => item.name === policyName));
        }
      });
  }

  create(rowData?, isClone?: boolean) {
    this.slaService.createRealDetectionPolicy(
      () => this.dataTable.fetchData(),
      rowData,
      isClone
    );
  }

  delete(data) {
    if (size(data) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get(
          'explore_delete_real_time_policy_warn_label',
          [data[0].name],
          false,
          true
        ),
        onOK: () =>
          this.ioDetectPolicyService
            .deleteIoDetectPolicy({ policyId: data[0].id })
            .subscribe(() => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            })
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get(
          'explore_delete_real_time_policy_warn_label',
          [map(data, 'name').join(this.i18n.isEn ? ',' : '，')],
          false,
          true
        ),
        onOK: () =>
          this.batchOperateService.selfGetResults(
            item => {
              return this.ioDetectPolicyService.deleteIoDetectPolicy({
                policyId: item.id,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            data,
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            }
          )
      });
    }
  }

  getDetail(rowData) {
    this.slaService.getRealDetectionPolicyDetail(rowData);
  }

  associatedFileSystem(rowData) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('explore_associate_file_system_label'),
        lvContent: AssociatedFileSystemComponent,
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

  associatedWhiteList(rowData) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('explore_associated_whitelist_count_label'),
        lvContent: AssociatedWhiteListComponent,
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
}
