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
  FileExtensionFilterManagementService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService
} from 'app/shared';
import { FsFileExtensionFilterManagementService } from 'app/shared/api/services/fs-file-extension-filter-management.service';
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
  filter,
  find,
  includes,
  isEmpty,
  isUndefined,
  map,
  values
} from 'lodash';
import { AssociateFileSystemComponent } from './associate-file-system/associate-file-system.component';
import { CreateFilterRuleComponent } from './create-filter-rule/create-filter-rule.component';
import { FileSystemNumComponent } from './file-system-num/file-system-num.component';

@Component({
  selector: 'aui-interception-filter-rule',
  templateUrl: './filter-rule.component.html',
  styleUrls: ['./filter-rule.component.less']
})
export class FilterRuleComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems = [];

  @Output() refreshRule = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('numTpl', { static: true })
  numTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private fileExtensionFilterManagementService: FileExtensionFilterManagementService,
    private fsFileExtensionFilterManagementService: FsFileExtensionFilterManagementService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        type: 'primary',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('common_create_label'),
        onClick: () => this.create()
      },
      associate: {
        id: 'associate',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('explore_associate_file_system_label'),
        disableCheck: data => {
          return isEmpty(data);
        },
        onClick: data => this.associate(data, true)
      },
      disassociate: {
        id: 'disassociate',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('explore_disassociate_vstore_label'),
        disableCheck: data => {
          return isEmpty(data);
        },
        onClick: data => this.associate(data, false)
      },
      delete: {
        id: 'delete',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return (
            isEmpty(data) ||
            !isEmpty(
              find(
                data,
                item =>
                  item.fileExtensionType ===
                  DataMap.File_Extension_Type.preset.value
              )
            )
          );
        },
        onClick: data => this.delete(data)
      }
    };

    this.optItems = filter(
      cloneDeep(getPermissionMenuItem(values(opts))),
      item => {
        if (item.id === 'disassociate') {
          item.divide = true;
        }
        return !includes(['create'], item.id);
      }
    );

    const cols: TableCols[] = [
      {
        key: 'fileExtensionName',
        name: this.i18n.get('explore_file_extension_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'fileExtensionType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('File_Extension_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('File_Extension_Type')
        }
      },
      {
        key: 'fileSystemInfos',
        name: this.i18n.get('explore_association_file_system_num_label'),
        cellRender: this.numTpl
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
        autoPolling: CommonConsts.TIME_INTERVAL * 3,
        compareWith: 'fileExtensionName',
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
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
          return item.fileExtensionName;
        }
      }
    };

    this.optsConfig = getPermissionMenuItem([
      opts.create,
      opts.associate,
      opts.disassociate,
      opts.delete
    ]);
  }

  getData(filters: Filters, args) {
    const params: any = {
      pageNum: filters.paginator.pageIndex + 1,
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
        orderBy: filters.sort.key
      });
    }

    this.fsFileExtensionFilterManagementService
      .getFsExtensionFilterUsingGET(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records || [],
          total: res.totalCount || 0
        };
        this.refreshRule.emit();
      });
  }

  getFileSystemDetail(rowData) {
    if (!rowData.fsNumber) {
      return;
    }
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('explore_associate_file_system_label'),
        lvContent: FileSystemNumComponent,
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

  create() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('common_create_label'),
        lvContent: CreateFilterRuleComponent,
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateFilterRuleComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(result => {
            modalIns.lvOkDisabled = result !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateFilterRuleComponent;
            content.onOK().subscribe(
              () => {
                resolve(true);
                this.dataTable.fetchData();
              },
              () => resolve(false)
            );
          });
        }
      })
    );
  }

  delete(datas) {
    this.warningMessageService.create({
      content: this.i18n.get(
        'explore_delete_blocking_file_extension_label',
        [map(datas, 'fileExtensionName').join(',')],
        false,
        true
      ),
      onOK: () => {
        this.fileExtensionFilterManagementService
          .deleteCustomizationFileExtensionFilterUsingDelete({
            deleteCustomFileExtensionRequest: {
              deleteExtensions: map(datas, 'fileExtensionName')
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

  associate(datas, isAssociate) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: isAssociate
          ? this.i18n.get('explore_associate_file_system_label')
          : this.i18n.get('explore_disassociate_vstore_label'),
        lvContent: AssociateFileSystemComponent,
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvComponentParams: { datas, isAssociate },
        lvOkDisabled: true,
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AssociateFileSystemComponent;
            content.onOK().subscribe(
              () => {
                resolve(true);
                this.dataTable.fetchData();
              },
              () => resolve(false)
            );
          });
        }
      })
    );
  }
}
