import {
  AfterViewInit,
  Component,
  EventEmitter,
  Input,
  OnChanges,
  OnInit,
  Output,
  SimpleChanges,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  getLabelList,
  getPermissionMenuItem,
  GROUP_COMMON,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  ProtectedResourceApiService,
  ResourceType,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  each,
  filter,
  includes,
  isArray,
  isEmpty,
  isUndefined,
  map,
  size,
  toString,
  trim
} from 'lodash';
import { AddTelnetComponent } from '../../huawei-stack/stack-list/add-telnet/add-telnet.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';

@Component({
  selector: 'aui-domain-list',
  templateUrl: './domain-list.component.html',
  styleUrls: ['./domain-list.component.less']
})
export class DomainListComponent implements OnInit, AfterViewInit, OnChanges {
  @Input() treeSelection: any;
  @Output() updateTable = new EventEmitter();

  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData: any = [];
  name;
  moreMenus = [];

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('projectCountTpl', { static: true }) projectCountTpl: TemplateRef<
    any
  >;
  @ViewChild('resourceTagTpl', { static: true })
  resourceTagTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private setResourceTagService: SetResourceTagService
  ) {}

  ngOnChanges(changes: SimpleChanges) {
    if (changes.treeSelection && this.dataTable) {
      this.dataTable.stopPolling();
      this.dataTable.fetchData();
    }
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(400);
    this.initConfig();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.ModifyHCSTenant,
        onClick: ([item]) => this.addDomain(item)
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteResource,
        disableCheck: data => {
          return !data.length;
        },
        onClick: data => this.deleteDomain(data)
      },
      {
        id: 'addTag',
        permission: OperateItems.AddTag,
        disableCheck: data => {
          return !data.length;
        },
        label: this.i18n.get('common_add_tag_label'),
        onClick: data => this.addTag(data)
      },
      {
        id: 'removeTag',
        permission: OperateItems.RemoveTag,
        disableCheck: data => {
          return !data.length;
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: data => this.removeTag(data)
      }
    ];
    this.moreMenus = getPermissionMenuItem(
      filter(opts, item => includes(['addTag', 'removeTag'], item.id)),
      this.cookieService.role
    );
    this.optsConfig = filter(opts, item =>
      includes(['add', 'delete', 'addTag', 'removeTag'], item.id)
    );
    this.tableConfig = {
      table: {
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'projectCount',
            name: this.i18n.get('protection_project_count_label'),
            cellRender: this.projectCountTpl
          },
          {
            key: 'labelList',
            name: this.i18n.get('common_tag_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            },
            cellRender: this.resourceTagTpl
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
                items: getPermissionMenuItem(
                  filter(opts, item =>
                    includes(
                      ['modify', 'delete', 'addTag', 'removeTag'],
                      item.id
                    )
                  ),
                  this.cookieService.role
                )
              }
            }
          }
        ],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        compareWith: 'uuid',
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filters: Filters, args: {}) => {
          this.getData(filters, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true
      }
    };
  }

  addTag(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: isArray(data) ? data : this.selectionData,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      }
    });
  }

  removeTag(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: isArray(data) ? data : this.selectionData,
      onOk: () => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      }
    });
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(this.name)
        }
      ]
    });
    this.dataTable.fetchData();
  }

  getData(filters, args) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    const defaultConditions = {
      rootUuid: this.treeSelection?.rootUuid,
      subType: [ResourceType.OpenStackDomain],
      visible: ['1']
    };
    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.labelList) {
        assign(conditionsTemp, {
          labelCondition: {
            labelName: conditionsTemp.labelList[1]
          }
        });
        delete conditionsTemp.labelList;
      }
      assign(defaultConditions, conditionsTemp);
    }
    assign(params, { conditions: JSON.stringify(defaultConditions) });
    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        // 获取标签数据
        const { showList, hoverList } = getLabelList(item);
        assign(item, {
          showLabelList: showList,
          hoverLabelList: hoverList
        });
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      if (isEmpty(filters.conditions_v2)) {
        this.updateTable.emit({
          resType: ResourceType.OpenStackDomain,
          total: res.totalCount
        });
      }
    });
  }

  addDomain(item?: any) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-domain-modal',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: AddTelnetComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          item,
          treeSelection: this.treeSelection
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddTelnetComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(formGroupStatus => {
            modalIns.lvOkDisabled = formGroupStatus === 'INVALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddTelnetComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.protectedResourceApiService
                  .ScanProtectedResources({
                    akOperationTips: false,
                    resId: this.treeSelection.uuid
                  })
                  .subscribe(() => this.dataTable.fetchData());
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  deleteDomain(datas) {
    this.warningMessageService.create({
      content: this.i18n.get('protection_resource_telnet_label', [
        toString(map(datas, 'name'))
      ]),
      onOK: () => {
        if (size(datas) === 1) {
          this.protectedResourceApiService
            .DeleteResource({
              resourceId: datas[0].uuid
            })
            .subscribe(() => {
              this.protectedResourceApiService
                .ScanProtectedResources({
                  resId: this.treeSelection.uuid
                })
                .subscribe();
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.fetchData([]);
            });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.protectedResourceApiService.DeleteResource({
                resourceId: item.uuid
              });
            },
            datas,
            () => {
              this.protectedResourceApiService
                .ScanProtectedResources({
                  akOperationTips: false,
                  resId: this.treeSelection.uuid
                })
                .subscribe();
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.fetchData([]);
            }
          );
        }
      }
    });
  }
}
