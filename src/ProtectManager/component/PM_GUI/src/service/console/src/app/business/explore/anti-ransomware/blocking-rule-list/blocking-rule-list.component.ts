import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  DataMap,
  DataMapService,
  FileExtensionFilterManagementService,
  I18NService,
  MODAL_COMMON,
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
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  filter,
  find,
  includes,
  isEmpty,
  map,
  size,
  uniqBy
} from 'lodash';
import { AddBlockingRuleComponent } from './add-blocking-rule/add-blocking-rule.component';
import { AssociateVstoreComponent } from './associate-vstore/associate-vstore.component';

@Component({
  selector: 'aui-blocking-rule-list',
  templateUrl: './blocking-rule-list.component.html',
  styleUrls: ['./blocking-rule-list.component.less']
})
export class BlockingRuleListComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('numTpl', { static: true }) numTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private fileExtensionFilterManagementService: FileExtensionFilterManagementService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_create_label'),
        onClick: () => {
          this.addRule();
        }
      },
      {
        id: 'associate-vstore',
        label: this.i18n.get('explore_associate_vstore_label'),
        disableCheck: data => {
          return !size(data);
        },
        onClick: data => {
          this.changeVstore(data, true);
        }
      },
      {
        id: 'disassociate-vstore',
        label: this.i18n.get('explore_disassociate_vstore_label'),
        disableCheck: data => {
          let assciateVstores = [];
          data.forEach(item => {
            assciateVstores =
              item['vstoreInfos'] && !!size(item['vstoreInfos'])
                ? assciateVstores.concat(item['vstoreInfos'])
                : assciateVstores;
          });
          return !size(data) || !size(assciateVstores);
        },
        onClick: data => {
          this.changeVstore(data, false);
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            find(data, {
              fileExtensionType: DataMap.File_Extension_Type.preset.value
            })
          );
        },
        onClick: data => {
          this.delete(data);
        }
      }
    ];
    this.optsConfig = opts;

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
        key: 'vstoreNames',
        name: this.i18n.get('explore_associated_vstores_label'),
        cellRender: this.numTpl
      },
      {
        key: 'createTime',
        name: this.i18n.get('explore_add_tiem_label'),
        sort: true
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: filter(opts, item => {
              return includes(
                ['associate-vstore', 'disassociate-vstore', 'delete'],
                item.id
              );
            })
          }
        }
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'fileExtensionName',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        scroll: {
          ...this.virtualScroll.scrollParam,
          y: '65vh'
        },
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
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize
    };
    if (!!size(filters.orders)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.fileExtensionName) {
        assign(params, { fileExtensionName: conditions.fileExtensionName });
      }
      if (conditions.fileExtensionType) {
        assign(params, { fileExtensionType: conditions.fileExtensionType });
      }
      if (conditions.vstoreNames) {
        assign(params, { vstoreName: conditions.vstoreNames });
      }
    }
    this.fileExtensionFilterManagementService
      .getFileExtensionFilterUsingGET(params)
      .subscribe(res => {
        res.records.filter(item => {
          assign(item, {
            vstoreNames:
              item['vstoreInfos'] && !!size(item['vstoreInfos'])
                ? size(item['vstoreInfos'])
                : 0
          });
        });
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  getVstoresDetail(item) {
    if (!item.vstoreNames) {
      return;
    }
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_blocking_files_rule_label'),
      lvModalKey: 'suffix_detail',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: AssociateVstoreComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        isDetail: true,
        vstoreInfos: item.vstoreInfos
      },
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ]
    });
  }

  addRule() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_add_blocking_file_extension_label'),
      lvModalKey: 'add_extension',
      lvWidth: this.i18n.isEn
        ? MODAL_COMMON.normalWidth + 100
        : MODAL_COMMON.normalWidth,
      lvContent: AddBlockingRuleComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddBlockingRuleComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddBlockingRuleComponent;
          content.onOK().subscribe({
            next: () => {
              resolve(true);
              this.dataTable.fetchData();
            },
            error: error => resolve(false)
          });
        });
      }
    });
  }

  changeVstore(data, isAssciate) {
    let assciateVstores = [];
    data.forEach(item => {
      assciateVstores =
        item['vstoreInfos'] && !!size(item['vstoreInfos'])
          ? assciateVstores.concat(item['vstoreInfos'])
          : assciateVstores;
    });
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get(
        isAssciate
          ? 'explore_associate_vstore_label'
          : 'explore_disassociate_vstore_label'
      ),
      lvModalKey: 'change_vstore',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: AssociateVstoreComponent,
      lvOkDisabled: true,
      lvComponentParams: {
        isAssciate,
        extensions: map(data, 'fileExtensionName'),
        assciateVstores: uniqBy(assciateVstores, 'vstoreName')
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AssociateVstoreComponent;
        content.valid$.subscribe(res => {
          modal.lvOkDisabled = !res;
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AssociateVstoreComponent;
          content.onOK().subscribe({
            next: () => {
              resolve(true);
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            },
            error: error => resolve(false)
          });
        });
      }
    });
  }

  delete(data) {
    const params = {
      deleteCustomFileExtensionRequest: {
        deleteExtensions: map(data, 'fileExtensionName')
      }
    };
    this.warningMessageService.create({
      content: this.i18n.get('explore_delete_blocking_file_extension_label', [
        map(data, 'fileExtensionName').toString()
      ]),
      onOK: () => {
        this.fileExtensionFilterManagementService
          .deleteCustomizationFileExtensionFilterUsingDelete(params)
          .subscribe(res => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          });
      }
    });
  }
}
