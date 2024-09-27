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
  IODETECTWHITELISTService,
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
import { DrawModalService } from 'app/shared/services/draw-modal.service';
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
import { CreateWhiteListComponent } from './create-white-list/create-white-list.component';
import { AssociatePolicyComponent } from './associate-policy/associate-policy.component';

@Component({
  selector: 'aui-real-detection-white-list',
  templateUrl: './white-list.component.html',
  styleUrls: ['./white-list.component.less']
})
export class WhiteListComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems;

  @Output() refreshSummary = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('policyTpl', { static: true })
  policyTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private ioDetectWhitelistService: IODETECTWHITELISTService
  ) {}

  ngAfterViewInit(): void {
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
        permission: OperateItems.RegisterApplication,
        label: this.i18n.get('common_create_label'),
        onClick: () => this.create()
      },
      associate: {
        id: 'associate',
        permission: OperateItems.RegisterApplication,
        label: this.i18n.get('explore_associate_policy_label'),
        disableCheck: data => {
          return !size(data);
        },
        onClick: data => this.associatePolicy(data)
      },
      disassociate: {
        id: 'disassociate',
        permission: OperateItems.RegisterApplication,
        divide: true,
        label: this.i18n.get('explore_disassociate_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            !isEmpty(find(data, item => item.associationPolicyNum === 0))
          );
        },
        onClick: data => this.associatePolicy(data, true)
      },
      delete: {
        id: 'delete',
        permission: OperateItems.RegisterApplication,
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return !size(data);
        },
        onClick: data => this.delete(data)
      }
    };

    this.optItems = cloneDeep(
      getPermissionMenuItem(
        reject(values(opts), item => includes(['create'], item.id))
      )
    );

    this.optsConfig = getPermissionMenuItem(values(opts));

    const cols: TableCols[] = [
      {
        key: 'content',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'type',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('whitelistType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('whitelistType')
        }
      },
      {
        key: 'associationPolicyNum',
        name: this.i18n.get('explore_associated_label'),
        cellRender: this.policyTpl,
        sort: true
      },
      {
        key: 'createTime',
        name: this.i18n.get('common_create_time_label'),
        sort: true
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
      assign(params, conditions);
    }

    if (!isEmpty(filters.sort) && !isEmpty(filters.sort.key)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy:
          filters.sort.key === 'associationPolicyNum'
            ? 'assocPolicyNum'
            : filters.sort.key
      });
    }

    this.ioDetectWhitelistService.getWhiteListInfo(params).subscribe(res => {
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      this.refreshSummary.emit();
    });
  }

  create(rowData?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_create_label'),
      lvModalKey: 'create-white-list-modal',
      lvWidth: this.i18n.isEn
        ? MODAL_COMMON.normalWidth + 200
        : MODAL_COMMON.normalWidth + 100,
      lvContent: CreateWhiteListComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        rowData
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateWhiteListComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateWhiteListComponent;
          content.onOK().subscribe(
            res => {
              resolve(true);
              this.dataTable?.fetchData();
            },
            () => resolve(false)
          );
        });
      }
    });
  }

  associatePolicy(data, isDisassociated?: boolean, isDetail?: boolean) {
    const modalParams = {
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: isDetail
        ? this.i18n.get('explore_associated_label')
        : isDisassociated
        ? this.i18n.get('explore_disassociate_label')
        : this.i18n.get('explore_associate_policy_label'),
      lvModalKey: 'associate-policy-modal',
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvContent: AssociatePolicyComponent,
      lvComponentParams: {
        rowData: data,
        isDisassociated,
        isDetail
      }
    };
    if (isDetail) {
      assign(modalParams, {
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      });
    } else {
      assign(modalParams, {
        lvOkDisabled: true,
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AssociatePolicyComponent;
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.dataTable?.fetchData();
                this.selectionData = [];
                this.dataTable?.setSelections([]);
              },
              () => resolve(false)
            );
          });
        }
      });
    }
    this.drawModalService.create(modalParams);
  }

  delete(datas) {
    this.warningMessageService.create({
      content: this.i18n.get(
        'explore_delete_white_list_label',
        [map(datas, 'content').join(',')],
        false,
        true
      ),
      onOK: () => {
        this.ioDetectWhitelistService
          .deleteWhiteListInfo({
            deleteWhiteListReq: {
              whitelistIds: map(datas, 'id')
            }
          })
          .subscribe(() => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          });
      }
    });
  }
}
