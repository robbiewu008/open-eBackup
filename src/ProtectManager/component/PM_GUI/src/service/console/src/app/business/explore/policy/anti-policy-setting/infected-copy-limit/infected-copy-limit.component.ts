import { DatePipe } from '@angular/common';
import {
  AfterViewInit,
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild
} from '@angular/core';
import { Router } from '@angular/router';
import { MessageService } from '@iux/live';
import {
  CookieService,
  DataMap,
  DataMapService,
  extendSlaInfo,
  getPermissionMenuItem,
  hasWormPermission,
  I18NService,
  MODAL_COMMON,
  ProtectedResourceApiService,
  RoleOperationMap,
  RoleType,
  WarningMessageService
} from 'app/shared';
import { AntiRansomwareInfectConfigApiService } from 'app/shared/api/services/anti-ransomware-infect-config-api.service';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  includes,
  isEmpty,
  isUndefined,
  mapValues,
  omit,
  reject,
  set,
  size
} from 'lodash';
import { AddLimitComponent } from './add-limit/add-limit.component';

@Component({
  selector: 'aui-infected-copy-limit',
  templateUrl: './infected-copy-limit.component.html',
  styleUrls: ['./infected-copy-limit.component.less'],
  providers: [DatePipe]
})
export class InfectedCopyLimitComponent implements OnInit, AfterViewInit {
  optsConfig;
  selectionData = [];
  tableConfig: TableConfig;
  tableData: TableData;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private datePipe: DatePipe,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private messageService: MessageService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private detailService: ResourceDetailService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private antiRansomwareInfectedCopyService: AntiRansomwareInfectConfigApiService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      addLimit: {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('explore_copy_limit_add_limit_label'),
        onClick: () => {
          this.configLimit();
        },
        permission: RoleOperationMap.preventExtortionAndWorm,
        displayCheck: data => {
          return !(this.cookieService.role === RoleType.Auditor);
        }
      },
      modifyLimit: {
        id: 'modify',
        type: 'default',
        label: this.i18n.get('explore_copy_limit_modify_limit_label'),
        disableCheck: data => {
          return (
            !data.length ||
            size(
              filter(data, val => {
                return (
                  hasWormPermission(val) ||
                  val.copyType === DataMap.copyTypes.replicate.value
                );
              })
            ) !== size(data)
          );
        },
        onClick: data => {
          this.configLimit(data);
        }
      },
      deleteLimit: {
        id: 'delete',
        type: 'default',
        label: this.i18n.get('explore_copy_limit_delete_limit_label'),
        disableCheck: data => {
          return (
            !data.length ||
            size(
              filter(data, val => {
                return (
                  hasWormPermission(val) ||
                  val.copyType === DataMap.copyTypes.replicate.value
                );
              })
            ) !== size(data)
          );
        },
        onClick: data => {
          this.deleteLimit(data);
        }
      }
    };

    this.optsConfig = getPermissionMenuItem([
      opts.addLimit,
      opts.modifyLimit,
      opts.deleteLimit
    ]);

    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'resourceName',
        name: this.i18n.get('common_name_label'),
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            click: data => {
              this.getDetail(data);
            }
          }
        },
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'resourceSubType',
        name: this.i18n.get('common_resource_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Detecting_Resource_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Detecting_Resource_Type')
        }
      },
      {
        key: 'resourceLocation',
        name: this.i18n.get('common_location_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'copyType',
        name: this.i18n.get('explore_copy_limit_copy_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('copyTypes')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('copyTypes')
        }
      },
      {
        key: 'operations',
        name: this.i18n.get('explore_copy_limit_operation_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('copyDataLimitType')
            .filter(
              item => item.value !== DataMap.copyDataLimitType.replication.value
            )
        }
      },
      {
        key: 'createTime',
        name: this.i18n.get('common_create_time_label'),
        sort: true
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: reject(opts, { id: 'add' })
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'id',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (index, item) => {
          return item.id;
        }
      }
    };
  }

  getData(filters?: Filters, args?: { isAutoPolling: any }) {
    const params = {
      pageNo: filters?.paginator.pageIndex,
      pageSize: filters?.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (conditionsTemp.resourceSubType) {
        assign(conditionsTemp, {
          resourceSubTypes: [conditionsTemp.resourceSubType]
        });
        delete conditionsTemp.resourceSubType;
      }
      if (conditionsTemp.copyType) {
        assign(conditionsTemp, {
          copyTypes: [conditionsTemp.copyType]
        });
        delete conditionsTemp.copyType;
      }
      assign(params, conditionsTemp);
    }

    if (!!size(filters?.sort)) {
      set(params, filters.sort.direction, [filters.sort.key]);
    }

    this.antiRansomwareInfectedCopyService
      .antiRansomwareInfectedCopyConfigGet(params)
      .subscribe(res => {
        if (!res?.records) {
          return;
        }
        each(res.records, (item: any) => {
          let tmpOps = item.infectedCopyOperations.split(',');
          tmpOps = tmpOps.map(val =>
            this.dataMapService.getLabel('copyDataLimitType', val)
          );
          assign(item, {
            operations: tmpOps.join(','),
            createTime: this.datePipe.transform(
              item.createTime,
              'yyyy/MM/dd HH:mm:ss'
            )
          });
        });
        this.tableData = {
          data: res.records || [],
          total: res.totalCount
        };
      });
  }

  configLimit(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-copy-limit',
        lvWidth: MODAL_COMMON.xLargeWidth,
        lvHeader: !!data
          ? this.i18n.get('explore_copy_limit_modify_limit_label')
          : this.i18n.get('explore_copy_limit_add_limit_label'),
        lvContent: AddLimitComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            if (!!data) {
              this.modifyMessage(data, modal, resolve);
            } else {
              this.parseAddLimit(modal, resolve);
            }
          });
        }
      })
    );
  }

  modifyMessage(data: any, modal: any, resolve: (value: unknown) => void) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_copy_limit_modify_limit_tip_label', [
        data.map(item => item.resourceName).join(',')
      ]),
      width: 700,
      onOK: () => {
        this.parseAddLimit(modal, resolve);
      },
      onCancel: () => resolve(false),
      lvAfterClose: result => {
        if (result && result.trigger === 'close') {
          resolve(false);
        }
      }
    });
  }

  parseAddLimit(modal: any, resolve: (value: unknown) => void) {
    const content = modal.getContentComponent() as AddLimitComponent;
    content.onOK().subscribe({
      next: res => {
        resolve(true);
        this.dataTable.fetchData();
        this.selectionData = [];
        this.dataTable.setSelections([]);
        localStorage.setItem('addCopyLimitComplete', '1');
      },
      error: () => {
        resolve(false);
      }
    });
  }

  getDetail(res) {
    this.protectedResourceApiService
      .ShowResource({ resourceId: res.resourceId })
      .subscribe(item => {
        if (!item || isEmpty(item)) {
          this.messageService.error(
            this.i18n.get('common_resource_not_exist_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'resNotExistMesageKey'
            }
          );
          return;
        }
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'slaDetailModalKey'
          )
        ) {
          this.drawModalService.destroyModal('slaDetailModalKey');
        }
        extendSlaInfo(item);
        this.detailService.openDetailModal(res.resourceSubType, {
          data: assign(omit(cloneDeep(res), ['sla_id', 'sla_name']), item)
        });
      });
  }

  deleteLimit(data) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_copy_limit_delete_limit_tip_label', [
        data.map(item => item.resourceName).join(',')
      ]),
      width: 700,
      onOK: () => {
        this.antiRansomwareInfectedCopyService
          .antiRansomwareInfectedCopyConfigDelete({
            configDeleteReq: {
              ids: data.map(item => item.id)
            }
          })
          .subscribe(res => {
            this.dataTable.fetchData();
            this.selectionData = [];
            this.dataTable.setSelections([]);
          });
      }
    });
  }
}
