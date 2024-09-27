import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormGroup } from '@angular/forms';
import { DatatableComponent } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  EXPORT_LOG_MAXMUM,
  extendSlaInfo,
  I18NService,
  MultiCluster,
  ProjectedObjectApiService,
  PROTECTION_NAVIGATE_STATUS,
  ProtectResourceAction,
  ResourceSetType,
  RoleType,
  WarningMessageService
} from 'app/shared';
import {
  ClientManagerApiService,
  ComponentRestApiService,
  EnvironmentsService,
  HcsResourceServiceService,
  HostService,
  OpHcsServiceApiService,
  ResourceSetApiService
} from 'app/shared/api/services';
import { NumberToFixed } from 'app/shared/components/pro-core';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  get,
  has,
  includes,
  isArray,
  isBoolean,
  isEmpty,
  isNil,
  isNumber,
  map as _map,
  reject,
  set,
  trim
} from 'lodash';
import { Subject, Subscription } from 'rxjs';

@Component({
  selector: 'aui-client-template',
  templateUrl: './client-template.component.html',
  styleUrls: ['./client-template.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ClientTemplateComponent {
  @Input() allSelectionMap;
  @Input() data;
  @Input() isDetail;
  @Output() allSelectChange = new EventEmitter<any>();

  NumberToFixed = NumberToFixed;
  formGroup: FormGroup;
  queryVersion;
  queryUuid;
  queryName;
  queryIp;
  querySlaName;
  queryTrustworthiness;
  queryRemark;
  orderBy;
  orderType;
  selection = [];
  filterParams: any = {};
  protectResourceAction = ProtectResourceAction;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  resourceType = DataMap.Resource_Type;
  updateAgentBtnDisabled = true;
  hostTrustOpen: boolean = false;
  VERSION_VERIFY = EXPORT_LOG_MAXMUM.x8000Num;
  isMulti = MultiCluster.isMulti;
  isAllSelect = false; // 用来标记是否全选
  buttonLabel = this.i18n.get('system_resourceset_all_select_label');
  moreMenus = [];
  columns = [
    {
      label: this.i18n.get('protection_resource_id_label'),
      key: 'uuid',
      isShow: false
    },
    {
      label: this.i18n.get('common_name_label'),
      key: 'name',
      isShow: true
    },
    {
      label: this.i18n.get('common_ip_address_label'),
      key: 'endpoint',
      isShow: true,
      width: '200px'
    },
    {
      label: this.i18n.get('common_status_label'),
      key: 'link_status',
      filter: true,
      filterMap: this.dataMapService.toArray('resource_Host_LinkStatus'),
      isShow: true
    },
    {
      label: this.i18n.get('protection_host_cpu_label'),
      key: 'cpuRate',
      isShow: true,
      isSort: true
    },
    {
      label: this.i18n.get('protection_host_mem_label'),
      key: 'memRate',
      isShow: true,
      isSort: true
    },
    {
      label: this.i18n.get('protection_os_type_label'),
      key: 'os_type',
      filter: true,
      filterMap: this.dataMapService.toArray('Os_Type').filter(item => {
        return includes(
          [
            DataMap.Os_Type.windows.value,
            DataMap.Os_Type.linux.value,
            DataMap.Os_Type.aix.value,
            DataMap.Os_Type.solaris.value
          ],
          item.value
        );
      }),
      isShow: true
    },
    {
      label: this.i18n.get('protection_proxy_type_label'),
      key: 'sub_type',
      filter: true,
      filterMap: reject(this.dataMapService.toArray('Host_Proxy_Type'), item =>
        includes(
          [
            DataMap.Host_Proxy_Type.DWSBackupAgent.value,
            DataMap.Host_Proxy_Type.DBBackupAgent.value
          ],
          item.value
        )
      ),
      isShow: true
    },
    {
      label: this.i18n.get('system_current_version_label'),
      key: 'version',
      isShow: true
    },
    {
      label: this.i18n.get('common_host_trustworthiness_status_label'),
      key: 'trustworthiness',
      isShow: this.hostTrustOpen
    },
    {
      label: this.i18n.get('common_source_deduplication_support_label'),
      key: 'src_deduption',
      isShow: false
    },
    {
      label: this.i18n.get('system_log_level_label'),
      key: 'log_level',
      isShow: true
    },
    {
      label: this.i18n.get('protection_lanfree_label'),
      key: 'lan_free',
      isShow: false
    },
    {
      label: this.i18n.get('protection_directory_label'),
      key: 'install_path',
      isShow: false
    },
    {
      label: this.i18n.get('protection_multi-tenant_sharing_label'),
      key: 'isShared',
      filter: true,
      filterMap: this.dataMapService.toArray('switchStatus'),
      isShow: false
    },
    {
      label: this.i18n.get('common_remarks_label'),
      key: 'memo',
      isShow: true
    },
    {
      label: this.i18n.get('common_az_label'),
      key: 'availableZoneName',
      isShow: false
    }
  ];
  tableColumnKey = 'protection_host_table';
  columnStatus = this.rememberColumnsService.getColumnsStatus(
    this.tableColumnKey
  );

  nameErrorTip = {
    invalidNameCombination: this.i18n.get(
      'common_valid_name_combination_label'
    ),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [251])
  };

  currentDetailItemUuid;
  hostListSub$: Subscription;
  destroy$ = new Subject();
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isDataProtectionAdmin =
    this.cookieService.role === RoleType.DataProtectionAdmin;
  isHcsEnvir =
    this.cookieService.get('serviceProduct') === CommonConsts.serviceProduct;
  azOptions = [];

  tableData;

  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  @ViewChild('ipPopover', { static: false }) ipPopover;
  @ViewChild('uuidPopover', { static: false }) uuidPopover;
  @ViewChild('namePopover', { static: false }) namePopover;
  @ViewChild('versionPopover', { static: false }) versionPopover;
  @ViewChild('slaNamePopover', { static: false }) slaNamePopover;
  @ViewChild('remarkPopover', { static: false }) remarkPopover;
  @ViewChild('contentTpl', { static: false }) contentTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    private cookieService: CookieService,
    private hostApiService: HostService,
    private dataMapService: DataMapService,
    public warningMessageService: WarningMessageService,
    public projectedObjectApiService: ProjectedObjectApiService,
    public takeManualBackupService: TakeManualBackupService,
    public environmentsApiService: EnvironmentsService,
    private rememberColumnsService: RememberColumnsService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    public componentRestApiService: ComponentRestApiService,
    public batchOperateService: BatchOperateService,
    private clientManagerApiService: ClientManagerApiService,
    public baseUtilService: BaseUtilService,
    private hcsResourceService: HcsResourceServiceService,
    private opHcsServiceApiService: OpHcsServiceApiService,
    private resourceSetService: ResourceSetApiService,
    public appUtilsService?: AppUtilsService
  ) {}
  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  onChange() {
    this.ngOnInit();
  }

  ngOnInit() {
    if (
      includes(
        [DataMap.Deploy_Type.x3000.value, DataMap.Deploy_Type.x6000.value],
        this.i18n.get('deploy_type')
      )
    ) {
      this.VERSION_VERIFY = EXPORT_LOG_MAXMUM.x6000Num;
    }
    if (!!this.allSelectionMap[ResourceSetType.Agent]?.isAllSelected) {
      this.isAllSelect = true;
    }
    this.removeLanFree();
    this.removeSrcDeduption();
    this.getColumnStatus();
    this.getHosts();
    this.removeAz();
    if (!isEmpty(this.allSelectionMap[ResourceSetType.Agent]?.data)) {
      this.selection = this.allSelectionMap[ResourceSetType.Agent].data;
    }
  }

  removeAz() {
    if (!this.isHcsEnvir && !this.isHcsUser) {
      this.columns = reject(
        this.columns,
        item => item.key === 'availableZoneName'
      );
    }
  }

  removeLanFree() {
    if (this.isHcsUser || this.appUtilsService.isDistributed) {
      this.columns = reject(this.columns, item => item.key === 'lan_free');
    }
  }

  removeSrcDeduption() {
    if (this.appUtilsService.isDistributed) {
      this.columns = reject(this.columns, item => item.key === 'src_deduption');
    }
  }

  getColumnStatus() {
    if (this.isHcsUser) {
      this.columns = this.columns.filter(i => i.key !== 'isShared');
    }
    if (!isEmpty(this.columnStatus)) {
      each(this.columns, col => {
        col.isShow = this.columnStatus[col.key];
      });
    }
  }

  getHosts(refreshData?) {
    each(this.moreMenus, item => {
      item.disabled = true;
    });
    if (
      !isEmpty(PROTECTION_NAVIGATE_STATUS.protectionStatus) ||
      isNumber(PROTECTION_NAVIGATE_STATUS.protectionStatus)
    ) {
      each(this.columns, item => {
        if (item.key === 'protection_status') {
          item.filterMap = filter(
            this.dataMapService.toArray('Protection_Status'),
            val => {
              if (val.value === PROTECTION_NAVIGATE_STATUS.protectionStatus) {
                val.selected = true;
              }
              return true;
            }
          );
        }
      });
      assign(this.filterParams, {
        protectionStatus: [['in'], PROTECTION_NAVIGATE_STATUS.protectionStatus]
      });
      PROTECTION_NAVIGATE_STATUS.protectionStatus = '';
    }
    if (!isEmpty(PROTECTION_NAVIGATE_STATUS.osType)) {
      each(this.columns, item => {
        if (item.key === 'os_type') {
          item.filterMap = filter(
            this.dataMapService.toArray('Os_Type').filter(item => {
              return includes(
                [
                  DataMap.Os_Type.windows.value,
                  DataMap.Os_Type.linux.value,
                  DataMap.Os_Type.aix.value,
                  DataMap.Os_Type.solaris.value
                ],
                item.value
              );
            }),
            val => {
              if (PROTECTION_NAVIGATE_STATUS.osType === val.value) {
                val.selected = true;
              }
              return true;
            }
          );
        }
      });
      assign(this.filterParams, {
        osType: [['in'], PROTECTION_NAVIGATE_STATUS.osType]
      });
      PROTECTION_NAVIGATE_STATUS.osType = '';
    }
    const params = this.getParams();
    this.clientManagerApiService
      .queryAgentListInfoUsingGET({ ...params, akLoading: true })
      .subscribe(res => {
        each(res.records, item => {
          const _trustworthiness = get(item, ['extendInfo', 'trustworthiness']);
          assign(item, {
            sub_type: item.subType,
            protection_status: item['protectionStatus'],
            link_status: +item['linkStatus'],
            os_type: item['osType'],
            trustworthiness: isNil(_trustworthiness)
              ? false
              : JSON.parse(_trustworthiness),
            availableZoneName: find(this.tableData, { uuid: item.uuid })
              ?.availableZoneName
          });
          extendSlaInfo(item);
        });
        this.total = res.totalCount;
        this.tableData = res.records;
        if (!isEmpty(this.allSelectionMap[ResourceSetType.Agent]?.data)) {
          // 重新进入时回显选中的数据
          this.selection = this.allSelectionMap[ResourceSetType.Agent]?.data;
        }

        if (!!this.allSelectionMap[ResourceSetType.Agent]?.isAllSelected) {
          this.allSelect(false);
        }

        if (
          !!this.data &&
          isEmpty(this.allSelectionMap[ResourceSetType.Agent]?.data) &&
          !this.isDetail
        ) {
          this.getSelectedData();
        }

        const hostUuids = _map(this.tableData, 'uuid');
        if (!isEmpty(hostUuids)) {
          this.hostApiService
            .getUpdateAgentHostV1ResourceHostsUpgradeableVersions({
              hostUuids,
              akLoading: false,
              akDoException: false
            })
            .subscribe(() => {
              this.cdr.detectChanges();
            });
        }

        if (this.isHcsUser) {
          this.parseHcs();
        }

        if (this.isHcsEnvir) {
          this.parseHcsEnv();
        }

        this.cdr.detectChanges();
      });
  }

  getSelectedData() {
    // 用于修改时回显
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: ResourceSetType.Agent,
      type: ResourceSetType.Agent
    };
    this.resourceSetService.QueryResourceObjectIdList(params).subscribe(res => {
      set(this.allSelectionMap, ResourceSetType.Agent, {
        data: _map(res, item => {
          return { uuid: item };
        })
      });
      this.selection = cloneDeep(
        this.allSelectionMap[ResourceSetType.Agent]?.data
      );
      this.allSelectChange.emit();
    });
  }

  parseHcsEnv() {
    this.opHcsServiceApiService
      .getAvailableZones({ akDoException: false, akLoading: false })
      .subscribe(res => {
        each(this.tableData, item => {
          assign(item, {
            availableZoneName:
              find(res.records, { azId: item.extendInfo?.availableZone })
                ?.name || ''
          });
        });
      });
  }

  parseHcs() {
    this.hcsResourceService
      .GetHcsAz({ akDoException: false, akLoading: false })
      .subscribe(res => {
        each(this.tableData, item => {
          assign(item, {
            availableZoneName:
              first(
                find(res.resources, {
                  resource_id: item.extendInfo?.availableZone
                })?.tags?.display_name
              ) || ''
          });
        });
      });
  }

  allSelect(turnPage?) {
    const isAllSelected = !!turnPage ? !this.isAllSelect : this.isAllSelect;
    set(this.allSelectionMap, ResourceSetType.Agent, { isAllSelected });
    this.selection = isAllSelected ? [...this.tableData] : [];
    each(this.tableData, item => {
      item.disabled = isAllSelected;
    });
    this.isAllSelect = isAllSelected;
    this.buttonLabel = this.i18n.get(
      isAllSelected
        ? 'system_resourceset_cancel_all_select_label'
        : 'system_resourceset_all_select_label'
    );
    this.allSelectChange.emit();
    this.cdr.detectChanges();
  }

  selectionChange(e) {
    set(this.allSelectionMap, ResourceSetType.Agent, {
      data: this.selection
    });
    this.allSelectChange.emit();
  }

  getParams() {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };

    assign(this.filterParams, {
      type: 'Host',
      subType: !this.filterParams['subType']
        ? [
            DataMap.Resource_Type.DBBackupAgent.value,
            DataMap.Resource_Type.VMBackupAgent.value,
            DataMap.Resource_Type.UBackupAgent.value,
            DataMap.Resource_Type.SBackupAgent.value
          ]
        : this.filterParams['subType'],
      scenario: '0',
      isCluster: false
    });

    if (window['queryDBBackupAgentParams']) {
      assign(this.filterParams, {
        ...window['queryDBBackupAgentParams']
      });
    }

    if (!trim(this.queryName)) {
      delete this.filterParams.name;
    } else {
      assign(this.filterParams, {
        name: trim(this.queryName)
      });
    }

    if (!trim(this.queryUuid)) {
      delete this.filterParams.uuid;
    } else {
      assign(this.filterParams, {
        uuid: trim(this.queryUuid)
      });
    }

    if (!trim(this.queryRemark)) {
      delete this.filterParams.tag;
    } else {
      assign(this.filterParams, {
        tag: trim(this.queryRemark)
      });
    }

    if (!trim(this.queryVersion)) {
      delete this.filterParams.version;
    } else {
      assign(this.filterParams, {
        version: trim(this.queryVersion)
      });
    }

    if (!trim(this.queryIp)) {
      delete this.filterParams.endpoint;
    } else {
      assign(this.filterParams, {
        endpoint: trim(this.queryIp)
      });
    }

    if (!trim(this.querySlaName)) {
      delete this.filterParams.slaName;
    } else {
      assign(this.filterParams, {
        slaName: trim(this.querySlaName)
      });
    }

    each(this.filterParams, (value, key) => {
      if (isEmpty(value) && !isBoolean(value)) {
        delete this.filterParams[key];
      }
      if (
        !includes(['scenario'], key) &&
        isArray(value) &&
        isEmpty(value[0]) &&
        !isBoolean(value[0]) &&
        !isNumber(value[0]) &&
        key !== 'subType'
      ) {
        delete this.filterParams[key];
      }
    });

    if (this.isDetail) {
      assign(this.filterParams, {
        resourceSetId: this.data[0].uuid
      });
    }

    if (!isEmpty(this.filterParams)) {
      if (has(this.filterParams, 'trustworthiness')) {
        get(this.filterParams, 'trustworthiness')[0] = ['=='];
        this.filterParams['extendInfo.trustworthiness'] = get(
          this.filterParams,
          'trustworthiness'
        );
        delete this.filterParams['trustworthiness'];
      }

      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }

    return params;
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getHosts();
  }

  trackByUuid(index: number, list: any) {
    return list.uuid;
  }

  filterChange(source) {
    const keyMap = {
      link_status: 'linkStatus',
      os_type: 'osType',
      sub_type: 'subType',
      sla_status: 'slaStatus',
      protection_status: 'protectionStatus',
      sla_compliance: 'slaCompliance'
    };
    if (source.key === 'link_status') {
      each(source.value, (v, index) => {
        source.value[index] = trim(v);
      });
    }

    assign(this.filterParams, {
      [keyMap[source.key] ? keyMap[source.key] : source.key]: [...source.value]
    });

    if (source.key === 'sub_type') {
      assign(this.filterParams, {
        subType: source.value
      });
    }

    this.getHosts();
  }

  sortChange(source) {
    const obj = {};
    obj[source.key] = source.direction;

    each(this.columns, item => {
      if (item.isSort) {
        delete this.filterParams[item.key];
      }
    });
    assign(this.filterParams, obj);
    this.getHosts();
  }

  searchByUuid(value) {
    if (this.uuidPopover) {
      this.uuidPopover.hide();
    }
    assign(this.filterParams, {
      uuid: trim(value)
    });
    this.getHosts();
  }

  searchByName(value) {
    if (this.namePopover) {
      this.namePopover.hide();
    }
    assign(this.filterParams, {
      name: trim(value)
    });
    this.getHosts();
  }

  searchByVersion(value) {
    if (this.versionPopover) {
      this.versionPopover.hide();
    }
    assign(this.filterParams, {
      version: trim(value)
    });
    this.getHosts();
  }

  searchByIp(value) {
    if (this.ipPopover) {
      this.ipPopover.hide();
    }
    assign(this.filterParams, {
      endpoint: trim(value)
    });
    this.getHosts();
  }

  searchBySlaName(value) {
    if (this.slaNamePopover) {
      this.slaNamePopover.hide();
    }
    assign(this.filterParams, {
      slaName: trim(value)
    });
    this.getHosts();
  }

  searchByRemark(value) {
    if (this.remarkPopover) {
      this.remarkPopover.hide();
    }
    assign(this.filterParams, {
      tag: trim(value)
    });
    this.getHosts();
  }
}
