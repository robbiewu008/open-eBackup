import {
  AfterViewInit,
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { OptionItem } from '@iux/live';
import {
  AntiRansomwarePolicyApiService,
  CookieService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  hasWormPermission,
  I18NService,
  MODAL_COMMON,
  ResourceSetApiService,
  ResourceSetType,
  RoleOperationMap,
  RoleType,
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
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  first,
  includes,
  isEmpty,
  map,
  parseInt,
  reject,
  set,
  some
} from 'lodash';
import { AntiPolicyDetailComponent } from './anti-policy-detail/anti-policy-detail.component';
import {
  CreateAntiPolicyComponent,
  EXCLUDE_RESOURCE_TYPES
} from './create-anti-policy/create-anti-policy.component';

@Component({
  selector: 'aui-anti-policy',
  templateUrl: './anti-policy.component.html',
  styleUrls: ['./anti-policy.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class AntiPolicyComponent implements OnInit, AfterViewInit {
  @Input() isResourceSet = false; // 用于资源集判断
  @Input() allSelectionMap;
  @Input() data;
  @Input() isDetail;
  @Output() allSelectChange = new EventEmitter<any>();

  optsConfig;
  selectionData = [];
  tableData: TableData;
  tableConfig: TableConfig;
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  isAllSelect = false; // 用来标记是否全选
  allSelectDisabled = true;
  buttonLabel = this.i18n.get('system_resourceset_all_select_label');

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('scheduleTpl', { static: true }) scheduleTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private resourceSetService: ResourceSetApiService,
    private warningMessageService: WarningMessageService,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService,
    public appUtilsService: AppUtilsService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    if (
      this.isResourceSet &&
      !!this.allSelectionMap[ResourceSetType.Worm]?.isAllSelected
    ) {
      this.isAllSelect = true;
    }
    if (this.appUtilsService.isDistributed || this.appUtilsService.isDecouple) {
      EXCLUDE_RESOURCE_TYPES.push(DataMap.Job_Target_Type.NASFileSystem);
    }
    this.initConfig();
  }

  onChange() {
    this.selectionData = [];
    this.tableData = {
      data: [],
      total: 0
    };
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'create',
        type: 'primary',
        label: this.i18n.get('common_create_label'),
        permission: RoleOperationMap.preventExtortionAndWorm,
        displayCheck: data => {
          return !(this.cookieService.role === RoleType.Auditor);
        },
        onClick: () => {
          this.create();
        }
      },
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        displayCheck: data => {
          return this.cookieService.role === parseInt(data[0].roleId, 0);
        },
        disableCheck: data => {
          return !data.length || some(data, item => !hasWormPermission(item));
        },
        onClick: data => {
          this.modify(first(data));
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        onClick: data => {
          this.delete(data);
        },
        disableCheck: data => {
          return !data.length || some(data, item => !hasWormPermission(item));
        }
      }
    ];
    this.optsConfig = getPermissionMenuItem(
      filter(opts, item => {
        return includes(['create'], item.id);
      })
    );
    const resourceSubTypeOptions = this.dataMapService
      .toArray('Job_Target_Type')
      .filter((v: OptionItem) => {
        return (
          (v.isLeaf = true) &&
          !includes(
            EXCLUDE_RESOURCE_TYPES.map(i => i.value),
            v.value
          )
        );
      })
      .map(item => {
        if (item.value === 'Host__and__FusionCompute')
          return { ...item, value: 'FusionComputeHost' };
        if (item.value === 'Cluster__and__FusionCompute')
          return { ...item, value: 'FusionComputeCluster' };
        return item;
      });

    const cols: TableCols[] = [
      {
        key: 'policyName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.isResourceSet
          ? null
          : {
              type: 'text',
              config: {
                id: 'outerClosable',
                click: data => {
                  this.getPolicyDetail(data);
                }
              }
            }
      },
      {
        key: 'schedule',
        name: this.i18n.get('common_scheduled_label'),
        cellRender: this.scheduleTpl,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Scheduling_Plan')
        }
      },
      {
        key: 'dataSourceType',
        name: this.i18n.get('explore_data_source_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Detecting_Data_Source')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Detecting_Data_Source')
        }
      },
      {
        key: 'resourceSubType',
        name: this.i18n.get('common_resource_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          showSearch: true,
          options: resourceSubTypeOptions
        },
        cellRender: {
          type: 'status',
          config: resourceSubTypeOptions
        }
      },
      {
        key: 'description',
        name: this.i18n.get('common_desc_label')
      },
      {
        key: 'operation',
        width: 144,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: reject(opts, { id: 'create' })
          }
        }
      }
    ];

    if (this.cookieService.role === RoleType.Auditor) {
      cols.splice(5, 1);
    }

    if (this.isResourceSet) {
      cols.pop();
    }

    this.tableConfig = {
      table: {
        compareWith: 'id',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        rows:
          this.isDetail || !this.isResourceSet
            ? null
            : {
                selectionMode: 'multiple',
                selectionTrigger: 'selector',
                showSelector: true
              },
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        selectionChange: selection => {
          this.selectionData = selection;
          if (this.isResourceSet) {
            set(this.allSelectionMap, ResourceSetType.Worm, {
              data: this.selectionData
            });
            this.allSelectChange.emit();
          }
        }
      }
    };

    if (this.isResourceSet) {
      delete this.tableConfig.table.autoPolling;
    }
  }

  allSelect(turnPage?) {
    // 用于资源集全选资源
    const isAllSelected = !!turnPage ? !this.isAllSelect : this.isAllSelect;
    set(this.allSelectionMap, ResourceSetType.Worm, { isAllSelected });
    this.selectionData = isAllSelected ? [...this.tableData.data] : [];
    each(this.tableData.data, item => {
      item.disabled = isAllSelected;
    });
    this.dataTable.setSelections(cloneDeep(this.selectionData));
    this.isAllSelect = isAllSelected;
    this.buttonLabel = this.i18n.get(
      isAllSelected
        ? 'system_resourceset_cancel_all_select_label'
        : 'system_resourceset_all_select_label'
    );
    this.allSelectChange.emit();
    this.cdr.detectChanges();
  }

  getData(filters?: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.policyName) {
        assign(params, { policyName: conditions.policyName.toString() });
      }
      if (conditions.schedule) {
        assign(params, { schedulePolicies: conditions.schedule });
      }
      if (conditions.dataSourceType) {
        assign(params, { dataSourceTypes: conditions.dataSourceType });
      }
      if (conditions.resourceSubType) {
        assign(params, { resourceSubTypes: conditions.resourceSubType });
      }
    }

    if (this.isDetail) {
      assign(params, {
        resourceSetId: this.data[0].uuid
      });
    }

    this.antiRansomwarePolicyApiService
      .ShowAntiRansomwarePolicies(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
        if (this.isResourceSet) {
          this.allSelectDisabled = res.totalCount === 0;
          this.dataCallBack();
          if (!!this.allSelectionMap[ResourceSetType.Worm]?.isAllSelected) {
            this.allSelect(false);
          }
        }
      });
  }

  dataCallBack() {
    // 用于资源集各类情况下的资源回显
    if (!isEmpty(this.allSelectionMap[ResourceSetType.Worm]?.data)) {
      // 重新进入时回显选中的数据
      this.selectionData = cloneDeep(
        this.allSelectionMap[ResourceSetType.Worm].data
      );
      this.dataTable.setSelections(cloneDeep(this.selectionData));
    }

    if (
      !!this.data &&
      isEmpty(this.allSelectionMap[ResourceSetType.Worm]?.data) &&
      !this.isDetail
    ) {
      this.getSelectedData();
    }
  }

  getSelectedData() {
    // 用于修改时回显
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: ResourceSetType.Worm,
      type: ResourceSetType.Worm
    };
    this.resourceSetService.QueryResourceObjectIdList(params).subscribe(res => {
      set(this.allSelectionMap, ResourceSetType.Worm, {
        data: map(res, item => {
          return { id: Number(item) };
        })
      });
      this.selectionData = cloneDeep(
        this.allSelectionMap[ResourceSetType.Worm].data
      );
      this.dataTable.setSelections(cloneDeep(this.selectionData));
      this.allSelectChange.emit();
    });
  }

  modify(item) {
    this.create(item);
  }

  create(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: !data
        ? this.i18n.get('common_create_label')
        : this.i18n.get('common_modify_label'),
      lvModalKey: 'anti-policy',
      lvWidth: MODAL_COMMON.largeWidth,
      lvContent: CreateAntiPolicyComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateAntiPolicyComponent;
        const modalIns = modal.getInstance();
        content.formGroup.statusChanges.subscribe(res => {
          /**
           * 防勒索、放篡改开关同时关闭时禁用 ok 按钮
           * @returns {Boolean}
           */
          const _checkSwitch = (): boolean => {
            const needDetect = content.formGroup.get('needDetect').value;
            const setWorm = content.formGroup.get('setWorm').value;
            if (!needDetect && !setWorm) return true;
            return false;
          };

          defer(() => {
            const resourceSubType = content.formGroup.get('resourceSubType')
              .value;
            modalIns.lvOkDisabled =
              res !== 'VALID' ||
              _checkSwitch() ||
              isEmpty(resourceSubType) ||
              content.resourceCheckFaild;
          });
        });
      },
      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateAntiPolicyComponent;
          content.onOK().subscribe({
            next: () => {
              resolve(true);
              this.dataTable.fetchData();
              // 通过本地存储让另一标签页的保护自动获取worm策略适配状态
              localStorage.setItem('addWormComplete', '1');
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  getPolicyDetail(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'anti-policy-detail',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: data.policyName,
        lvContent: AntiPolicyDetailComponent,
        lvComponentParams: {
          data
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

  delete(data) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_delete_anti_policy_label', [
        data[0].policyName
      ]),
      onOK: () => {
        this.antiRansomwarePolicyApiService
          .DeleteAntiRansomwarePolicy({
            id: data[0].id
          })
          .subscribe(() => {
            this.dataTable.fetchData();
          });
      }
    });
  }
}
