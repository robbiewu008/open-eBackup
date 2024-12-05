/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { Router } from '@angular/router';
import {
  DatatableComponent,
  MessageboxService,
  MessageService
} from '@iux/live';
import {
  ApiExportFilesApiService as ExportFileApiService,
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  EXPORT_LOG_MAXMUM,
  extendSlaInfo,
  getPermissionMenuItem,
  GlobalService,
  I18NService,
  MODAL_COMMON,
  MultiCluster,
  OperateItems,
  ProjectedObjectApiService,
  PROTECTION_NAVIGATE_STATUS,
  ProtectResourceAction,
  ProtectResourceCategory,
  RouterUrl,
  WarningMessageService,
  GROUP_COMMON,
  isRBACDPAdmin,
  RoleOperationMap,
  getLabelList,
  RoleOperationAuth,
  SetTagType
} from 'app/shared';
import {
  ClientManagerApiService,
  ComponentRestApiService,
  EnvironmentsService,
  HcsResourceServiceService,
  HostService,
  OpHcsServiceApiService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  SnmpApiService,
  SwitchService
} from 'app/shared/api/services';
import { NumberToFixed } from 'app/shared/components/pro-core';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ProtectService } from 'app/shared/services/protect.service';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { SlaService } from 'app/shared/services/sla.service';
import { TakeManualBackupService } from 'app/shared/services/take-manual-backup.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  first,
  get,
  has,
  head,
  includes,
  isArray,
  isBoolean,
  isEmpty,
  isNil,
  isNumber,
  join,
  map,
  map as _map,
  mapValues,
  reject,
  set,
  size,
  some,
  toString,
  trim,
  isUndefined,
  startsWith
} from 'lodash';
import {
  combineLatest,
  Observable,
  Observer,
  Subject,
  Subscription,
  timer
} from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { AddTagComponent } from './add-tag/add-tag.component';
import { ConfigLogLevelComponent } from './config-log-level/config-log-level.component';
import { DownloadProxyComponent } from './download-proxy/download-proxy.component';
import { LanFreeComponent } from './lan-free/lan-free.component';
import { ModifyAzComponent } from './modify-az/modify-az.component';
import { ModifyHostComponent } from './modify-host/modify-host.component';
import { ModifyResourceComponent } from './modify-resource/modify-resource.component';
import { UpdateAgentComponent } from './update-agent/update-agent.component';
import { SetResourceTagService } from 'app/shared/services/set-resource-tag.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
@Component({
  selector: 'aui-host',
  templateUrl: './host.component.html',
  styleUrls: ['./host.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class HostComponent implements OnInit, OnDestroy {
  NumberToFixed = NumberToFixed;
  formGroup: FormGroup;
  queryVersion;
  queryUuid;
  queryName;
  queryIp;
  querySlaName;
  queryAuthorizedUser;
  queryTrustworthiness;
  queryRemark;
  orderBy;
  orderType;
  selection = [];
  tableData = [];
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
    },
    {
      label: this.i18n.get('common_tag_label'),
      key: 'labelList',
      isShow: true
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
  isDataProtectionAdmin = isRBACDPAdmin(this.cookieService.role);
  isHcsEnvir =
    this.cookieService.get('serviceProduct') === CommonConsts.serviceProduct;
  azOptions = [];

  groupCommon = GROUP_COMMON;
  activeItem;

  roleOperationMap = RoleOperationMap;
  roleOperationAuth = RoleOperationAuth;

  registerTipShow = false;
  helpUrl = '';

  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  @ViewChild('contentTpl', { static: false }) contentTpl: TemplateRef<any>;

  constructor(
    private router: Router,
    private fb: FormBuilder,
    public i18n: I18NService,
    private cookieService: CookieService,
    private slaService: SlaService,
    private messageBox: MessageboxService,
    private hostApiService: HostService,
    private switchService: SwitchService,
    private detailService: ResourceDetailService,
    private protectService: ProtectService,
    private dataMapService: DataMapService,
    public warningMessageService: WarningMessageService,
    public projectedObjectApiService: ProjectedObjectApiService,
    public takeManualBackupService: TakeManualBackupService,
    public environmentsApiService: EnvironmentsService,
    private drawModalService: DrawModalService,
    private rememberColumnsService: RememberColumnsService,
    private globalService: GlobalService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private snmpApiService: SnmpApiService,
    private messageService: MessageService,
    public componentRestApiService: ComponentRestApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    public batchOperateService: BatchOperateService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private exportFilesApi: ExportFileApiService,
    private clientManagerApiService: ClientManagerApiService,
    public baseUtilService: BaseUtilService,
    private hcsResourceService: HcsResourceServiceService,
    private opHcsServiceApiService: OpHcsServiceApiService,
    private setResourceTagService: SetResourceTagService,
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
    this.removeLanFree();
    this.removeSrcDeduption();
    this.getColumnStatus();
    this.initHostTrust();
    this.getHosts();
    this.getMoreMenus();
    this.virtualScroll.getScrollParam(220);
    this.removeAz();
    this.getUserGuideState();
    this.showRegisterTip();
    this.helpUrl = this.appUtilsService.getHelpUrl();
  }

  getUserGuideState() {
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.showRegisterTip();
      });
  }

  // 判断是否为1.3版本
  compareVersion(item) {
    if (startsWith(item?.version, '1.3')) {
      return true;
    }
    return false;
  }

  showRegisterTip() {
    if (
      USER_GUIDE_CACHE_DATA.active &&
      USER_GUIDE_CACHE_DATA.showTips &&
      includes(this.roleOperationAuth, this.roleOperationMap.manageClient)
    ) {
      setTimeout(() => {
        this.registerTipShow = true;
        USER_GUIDE_CACHE_DATA.showTips = false;
        this.cdr.detectChanges();
      });
    }
  }

  lvPopoverBeforeClose = () => {
    this.registerTipShow = false;
    this.cdr.detectChanges();
  };

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

  initHostTrust() {
    this.switchService.ListSystemSwitchApi({}).subscribe((res: any) => {
      this.hostTrustOpen = !isEmpty(
        find(res.switches, item => {
          return item.name === 'HOST_TRUST' && item.status === 1;
        })
      );
      this.columns.find(
        item => item.key === 'trustworthiness'
      ).isShow = this.hostTrustOpen;

      this.moreMenus = this.moreMenus.filter(item => {
        if (
          item.id === 'trustworthinessHost' ||
          item.id === 'cancelTrustworthinessHost'
        ) {
          return this.hostTrustOpen;
        }
        return true;
      });
      defer(() => {
        // 从隐藏列中删除受信状态
        if (!this.hostTrustOpen) {
          this.columns = this.columns.filter(i => i.key !== 'trustworthiness');
        }
      });
    });
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

  getMoreMenus() {
    const menus = [
      {
        id: 'trustworthinessHost',
        disabled: true,
        label: this.i18n.get('common_host_trustworthiness_op_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => {
          this.trustworthinessHost(this.selection);
        }
      },

      {
        id: 'cancelTrustworthinessHost',
        disabled: true,
        label: this.i18n.get('common_host_cancel_trustworthiness_op_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => {
          this.cancelTrustworthinessHost(this.selection);
        }
      },
      {
        id: 'rescan',
        disabled: true,
        label: this.i18n.get('common_rescan_label'),
        permission: OperateItems.ModifyHost,
        onClick: () => this.rescan(this.selection)
      },
      {
        id: 'addTag',
        tips: '',
        label: this.i18n.get('common_set_remarks_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => {
          this.addTag(this.selection);
        }
      },
      {
        id: 'exportLog',
        tips: '',
        label: this.i18n.get('common_export_log_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => {
          this.exportLog(this.selection);
        }
      },
      {
        id: 'configLogLevel',
        tips: '',
        label: this.i18n.get('common_config_leg_level_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => {
          this.batchConfigLogLevel(this.selection);
        }
      },
      {
        id: 'openAutoSynchronizeHostName',
        label: this.i18n.get('common_config_open_sync_host_name_label'),
        permission: OperateItems.ModifyHost,
        onClick: () => {
          this.batchConfigSyncHostName(true);
        }
      },
      {
        id: 'closeAutoSynchronizeHostName',
        label: this.i18n.get('common_config_close_sync_host_name_label'),
        permission: OperateItems.ModifyHost,
        onClick: () => {
          this.batchConfigSyncHostName(false);
        }
      },
      {
        id: 'deleteHost',
        tips: '',
        disabled:
          size(
            filter(this.selection, val => {
              return (
                isEmpty(val.sla_id) &&
                val.link_status ===
                  DataMap.resource_Host_LinkStatus.offline.value
              );
            })
          ) !== size(this.selection) || !size(this.selection),
        label: this.i18n.get('protection_delete_host_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => this.removeHost(this.selection)
      },
      {
        id: 'addLabel',
        permission: OperateItems.AddTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return (
            !size(this.selection) ||
            some(this.selection, v => !this.hasClientPermission(v))
          );
        },
        label: this.i18n.get('common_add_tag_label'),
        onClick: () => this.addLabel(this.selection)
      },
      {
        id: 'removeLabel',
        permission: OperateItems.RemoveTag,
        displayCheck: data => {
          return true;
        },
        disableCheck: data => {
          return (
            !size(this.selection) ||
            some(this.selection, v => !this.hasClientPermission(v))
          );
        },
        label: this.i18n.get('common_remove_tag_label'),
        onClick: () => this.removeLabel(this.selection)
      }
    ];
    this.moreMenus = getPermissionMenuItem(menus, this.cookieService.role);
  }

  addLabel(data) {
    this.setResourceTagService.setTag({
      isAdd: true,
      rowDatas: data,
      type: SetTagType.Agent,
      onOk: () => {
        this.selection = [];
        this.getHosts();
      }
    });
  }

  removeLabel(data) {
    this.setResourceTagService.setTag({
      isAdd: false,
      rowDatas: data,
      type: SetTagType.Agent,
      onOk: () => {
        this.selection = [];
        this.getHosts();
      }
    });
  }

  refresh() {
    this.getHosts();
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
    if (this.hostListSub$) {
      this.hostListSub$.unsubscribe();
    }
    let manualRefresh = true;
    this.hostListSub$ = timer(0, CommonConsts.TIME_INTERVAL_RESOURCE)
      .pipe(
        switchMap(index => {
          manualRefresh = !index;
          return this.clientManagerApiService.queryAgentListInfoUsingGET({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(res => {
        each(res.records, (item: any) => {
          // 获取标签数据
          const { showList, hoverList } = getLabelList(item);

          const _trustworthiness = get(item, ['extendInfo', 'trustworthiness']);
          assign(item, {
            sub_type: item.subType,
            protection_status: item?.protectionStatus,
            link_status: Number(item?.linkStatus),
            os_type: item?.osType,
            authorized_user: item?.authorizedUser,
            trustworthiness: isNil(_trustworthiness)
              ? false
              : JSON.parse(_trustworthiness),
            availableZoneName: find(this.tableData, { uuid: item.uuid })
              ?.availableZoneName,
            showLabelList: showList,
            hoverLabelList: hoverList
          });
          extendSlaInfo(item);
        });
        this.total = res.totalCount;
        this.tableData = res.records;
        this.selectionChange(undefined);
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'detail-modal'
          ) &&
          refreshData &&
          manualRefresh
        ) {
          this.refreshDetail(refreshData, res.records);
        }
        if (
          includes(
            mapValues(this.drawModalService.modals, 'key'),
            'detail-modal'
          ) &&
          this.currentDetailItemUuid
        ) {
          this.globalService.emitStore({
            action: 'autoReshResource',
            state: find(res.records, { uuid: this.currentDetailItemUuid })
          });
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

        if (this.isHcsEnvir) {
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

        this.cdr.detectChanges();
      });
  }

  // 刷新资源详情
  refreshDetail(target, tableData) {
    if (find(tableData, { uuid: target.uuid })) {
      const findData = cloneDeep(find(tableData, { uuid: target.uuid }));
      const modalContent = find(this.drawModalService.modals, {
        key: 'detail-modal'
      }) as any;
      if (
        modalContent &&
        modalContent.modal &&
        modalContent.modal.contentInstance &&
        modalContent.modal.contentInstance.activeId
      ) {
        assign(findData, {
          activeId: modalContent.modal.contentInstance.activeId
        });
      }
      this.getDetail(findData);
    } else {
      this.drawModalService.destroyModal('detail-modal');
    }
  }

  getStatusDetail(data) {
    this.detailService.openLinkModal(data);
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
    if (!isEmpty(this.filterParams)) {
      if (has(this.filterParams, 'trustworthiness')) {
        get(this.filterParams, 'trustworthiness')[0] = ['=='];
        this.filterParams['extendInfo.trustworthiness'] = get(
          this.filterParams,
          'trustworthiness'
        );
        delete this.filterParams?.trustworthiness;
      }

      assign(params, {
        conditions: JSON.stringify(this.filterParams)
      });
    }

    return params;
  }

  getDetail(item) {
    if (
      includes(
        mapValues(this.drawModalService.modals, 'key'),
        'slaDetailModalKey'
      )
    ) {
      this.drawModalService.destroyModal('slaDetailModalKey');
    }
    this.detailService.openDetailModal(
      DataMap.Resource_Type.ABBackupClient.value,
      {
        data: assign(
          item,
          { optItems: this.getOptItems(item) },
          {
            optItemsFn: v => {
              return this.getOptItems(v);
            }
          }
        )
      },
      () => {
        this.activeItem = {};
        this.cdr.detectChanges();
      }
    );
    this.currentDetailItemUuid = item.uuid;
    this.activeItem = item;
  }

  getSlaDetail(item) {
    this.slaService.getDetail({ uuid: item.sla_id, name: item.sla_name });
  }

  optsCallback = data => {
    return this.getOptItems(data);
  };

  // 是否具有客户端权限
  hasClientPermission(item): boolean {
    if (isUndefined(item.resourceRoleAuth)) {
      return true;
    }
    return includes(
      _map(item.resourceRoleAuth, 'authOperation'),
      RoleOperationMap.manageClient
    );
  }

  getOptItems(data) {
    const menus = [
      {
        id: 'updateAgent',
        divide: true,
        permission: OperateItems.SynchTrapInfo,
        disabled:
          !(
            data.link_status ===
              DataMap.resource_Host_LinkStatus.normal.value &&
            data.extendInfo?.agentUpgradeable === '1'
          ) || !this.hasClientPermission(data),
        label: this.i18n.get('protection_update_agent_label'),
        onClick: () => this.updateAgent(data)
      },
      {
        id: 'synchTrapInfo',
        divide: true,
        permission: OperateItems.SynchTrapInfo,
        hidden: data.sub_type !== DataMap.Resource_Type.UBackupAgent.value,
        disabled:
          data.link_status !== DataMap.resource_Host_LinkStatus.normal.value ||
          data.subType === DataMap.Host_Proxy_Type.SBackupAgent.value ||
          !this.hasClientPermission(data),
        label: this.i18n.get('protection_synch_trap_info_label'),
        onClick: () => this.synchSnmpTrapInfo(data)
      },
      {
        id: 'configLanFree',
        divide: true,
        permission: OperateItems.SynchTrapInfo,
        hidden: this.isHcsUser || this.appUtilsService?.isDistributed,
        disabled:
          data.link_status !== DataMap.resource_Host_LinkStatus.normal.value ||
          (data.extendInfo?.src_deduption !== 'true' &&
            data.subType !== DataMap.Host_Proxy_Type.SBackupAgent.value &&
            data.osType !== DataMap.Os_Type.aix.value) ||
          !this.hasClientPermission(data),
        label: this.i18n.get('protection_config_lanfree_label'),
        onClick: () => this.configLanFree(data)
      },
      {
        id: 'trustworthinessHost',
        disabled: !!data.trustworthiness || !this.hasClientPermission(data),
        hidden: !this.hostTrustOpen,
        label: this.i18n.get('common_host_trustworthiness_op_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => {
          this.trustworthinessHost([data]);
        }
      },
      {
        id: 'cancelTrustworthinessHost',
        divide: true,
        disabled: !data.trustworthiness || !this.hasClientPermission(data),
        hidden: !this.hostTrustOpen,
        label: this.i18n.get('common_host_cancel_trustworthiness_op_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => {
          this.cancelTrustworthinessHost([data]);
        }
      },
      {
        id: 'modifyResource',
        divide: true,
        permission: OperateItems.SynchTrapInfo,
        hidden: data.subType !== DataMap.Host_Proxy_Type.UBackupAgent.value,
        disabled:
          data.link_status !== DataMap.resource_Host_LinkStatus.normal.value ||
          data.extendInfo?.agentUpgradeable === '1' ||
          !this.hasClientPermission(data) ||
          this.compareVersion(data),
        label: this.i18n.get('protection_modify_host_applications_label'),
        onClick: () => this.modifyResource(data)
      },
      {
        id: 'modifyAz',
        divide: true,
        permission: OperateItems.SynchTrapInfo,
        hidden: !this.isHcsUser && !this.isHcsEnvir,
        disabled:
          data.link_status !== DataMap.resource_Host_LinkStatus.normal.value ||
          data.extendInfo?.agentUpgradeable === '1' ||
          !this.hasClientPermission(data),
        label: this.i18n.get('common_modify_az_label'),
        onClick: () => this.modifyAz(data)
      },
      {
        id: 'rescan',
        permission: OperateItems.RescanVirtualizationPlatform,
        label: this.i18n.get('common_rescan_label'),
        divide: true,
        disabled:
          data.link_status !== DataMap.resource_Host_LinkStatus.normal.value ||
          !this.hasClientPermission(data) ||
          this.compareVersion(data),
        onClick: () => {
          this.protectedResourceApiService
            .ScanProtectedResources({
              resId: data.uuid
            })
            .subscribe(() => this.getHosts());
        }
      },
      {
        id: 'modifyHost',
        label: this.i18n.get('protection_modify_host_label'),
        permission: OperateItems.ModifyHost,
        disabled: !this.hasClientPermission(data),
        onClick: () => this.modifyHost(data)
      },
      {
        id: 'removeHost',
        divide: true,
        disabled:
          data.sla_id ||
          data.link_status !== DataMap.resource_Host_LinkStatus.offline.value ||
          !this.hasClientPermission(data),
        label: this.i18n.get('protection_delete_host_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => this.removeHost([data], data)
      },
      {
        id: 'addTag',
        divide: true,
        permission: OperateItems.RemoveHost,
        label: this.i18n.get('common_set_remarks_label'),
        disabled: !this.hasClientPermission(data),
        onClick: () => this.addTag(data)
      },
      {
        id: 'exportLog',
        tips: '',
        disabled:
          data.link_status !== DataMap.resource_Host_LinkStatus.normal.value ||
          !this.hasClientPermission(data),
        label: this.i18n.get('common_export_log_label'),
        permission: OperateItems.RemoveHost,
        onClick: () => {
          this.exportLog(data);
        }
      },
      {
        id: 'configLogLevel',
        disabled:
          data.link_status !== DataMap.resource_Host_LinkStatus.normal.value ||
          !this.hasClientPermission(data),
        permission: OperateItems.RemoveHost,
        label: this.i18n.get('common_config_leg_level_label'),
        onClick: () => {
          this.configLogLevel(data);
        }
      },
      {
        id: 'addLabel',
        permission: OperateItems.AddTag,
        label: this.i18n.get('common_add_tag_label'),
        disabled: !this.hasClientPermission(data),
        onClick: () => this.addLabel([data])
      },
      {
        id: 'removeLabel',
        permission: OperateItems.RemoveTag,
        label: this.i18n.get('common_remove_tag_label'),
        disabled: !this.hasClientPermission(data),
        onClick: () => this.removeLabel([data])
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  }

  batchConfigLogLevel(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'config-logLevel',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('common_config_leg_level_label'),
        lvContent: ConfigLogLevelComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          this.batchOperateService.selfGetResults(
            item => {
              const content = modal.getContentComponent() as ConfigLogLevelComponent;
              const params = {
                logLevel: content.formGroup.value.log_level
              };
              return this.clientManagerApiService.updateAgentLogConfigurationPUT(
                {
                  configParam: params,
                  agentId: item.uuid
                }
              );
            },
            cloneDeep(this.selection),
            () => {
              this.refresh();
            }
          );
        }
      })
    );
  }

  batchConfigSyncHostName(status: boolean) {
    if (isEmpty(this.selection)) {
      return;
    }
    this.batchOperateService.selfGetResults(
      item => {
        return this.protectedResourceApiService.UpdateResource({
          resourceId: item.uuid,
          akOperationTips: false,
          akLoading: false,
          UpdateResourceRequestBody: {
            extendInfo: {
              is_auto_synchronize_host_name: String(status)
            }
          }
        });
      },
      this.selection,
      () => {
        this.selection = [];
        this.getHosts();
      }
    );
  }

  configLogLevel(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'config-logLevel',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('common_config_leg_level_label'),
        lvContent: ConfigLogLevelComponent,
        lvOkDisabled: false,
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as ConfigLogLevelComponent;
            content.onOK().subscribe({
              next: () => {
                this.getHosts();
                resolve(true);
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  exportLog(data) {
    this.formGroup = this.fb.group({
      fileName: new FormControl('file_01', [
        this.baseUtilService.VALID.name(
          CommonConsts.REGEX.nameCombination,
          false
        ),
        this.baseUtilService.VALID.maxLength(251)
      ])
    });
    this.drawModalService.create({
      lvHeader: this.i18n.get('common_export_log_label'),
      lvContent: this.contentTpl,
      lvWidth: 600,
      lvOk: () => {
        const request = {
          type: 'AGENT_LOG',
          name: this.formGroup.value.fileName,
          params: {
            agentIds: []
          }
        };

        if (isArray(data)) {
          each(data, item => {
            request.params.agentIds.push(item.uuid);
          });
        } else {
          request.params.agentIds.push(data.uuid);
        }
        this.exportFilesApi
          .CreateExportFile({
            request,
            akOperationTipsContent: this.i18n.get(
              'common_export_files_result_label'
            )
          })
          .subscribe(res => {});
      }
    });
  }

  protect(datas, action: ProtectResourceAction) {
    const type =
      isArray(datas) && size(datas) > 1
        ? ProtectResourceCategory.hosts
        : ProtectResourceCategory.host;
    this.protectService.openProtectModal(type, action, {
      data: datas,
      width: 780,
      onOK: () => {
        let params;
        if (!isArray(datas)) {
          params = datas;
        } else {
          if (isArray(datas) && size(datas) === 1) {
            params = datas[0];
          } else {
            params = null;
          }
        }

        this.getHosts(params);
      }
    });
  }

  modifyResource(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvWidth: MODAL_COMMON.normalWidth,
      lvHeader: this.i18n.get('protection_modify_host_applications_label'),
      lvOkDisabled: true,
      lvContent: ModifyResourceComponent,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as ModifyResourceComponent;
        content.formGroup.statusChanges.subscribe(status => {
          modal.lvOkDisabled = status !== 'VALID';
        });
      },
      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Observable((observer: Observer<void>) => {
          const content = modal.getContentComponent() as ModifyResourceComponent;
          content.onOK().subscribe(
            () => {
              this.getHosts();
              observer.next();
              observer.complete();
            },
            () => {
              observer.error(null);
              observer.complete();
            }
          );
        });
      }
    });
  }

  modifyAz(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvWidth: MODAL_COMMON.normalWidth,
      lvHeader: this.i18n.get('common_modify_az_label'),
      lvContent: ModifyAzComponent,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as ModifyAzComponent;
        content.formGroup.statusChanges.subscribe(status => {
          modal.lvOkDisabled = status !== 'VALID';
        });
      },
      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Observable((observer: Observer<void>) => {
          const content = modal.getContentComponent() as ModifyAzComponent;
          content.onOK().subscribe(
            () => {
              this.getHosts();
              observer.next();
              observer.complete();
            },
            () => {
              observer.error(null);
              observer.complete();
            }
          );
        });
      }
    });
  }

  modifyHost(data) {
    this.environmentsApiService
      .queryResourcesV1EnvironmentsGet({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          uuid: data.uuid
        })
      })
      .subscribe(res => {
        this.drawModalService.create({
          ...MODAL_COMMON.generateDrawerOptions(),
          lvWidth: MODAL_COMMON.normalWidth,
          lvHeader: this.i18n.get('protection_modify_host_label'),
          lvOkDisabled: !data.name,
          lvContent: ModifyHostComponent,
          lvAfterOpen: modal => {
            const content = modal.getContentComponent() as ModifyHostComponent;
            content.formGroup.statusChanges.subscribe(status => {
              modal.lvOkDisabled = status !== 'VALID';
            });
          },
          lvComponentParams: {
            data
          },
          lvOk: modal => {
            return new Promise(resolve => {
              const content = modal.getContentComponent() as ModifyHostComponent;
              content.onOK().subscribe({
                next: () => {
                  resolve(true);
                  this.getHosts(data);
                },
                error: () => resolve(false)
              });
            });
          }
        });
      });
  }

  removeHost(datas, refreshData?) {
    const promises = [];
    const names = [];
    datas.forEach(data => {
      names.push(data.name);
    });
    this.warningMessageService.create({
      content: this.i18n.get('protection_resource_delete_label', [
        toString(names)
      ]),
      onOK: () => {
        each(datas, item => {
          if (item.sub_type === DataMap.Host_Proxy_Type.UBackupAgent.value) {
            promises.push(
              new Promise((resolve, reject) => {
                this.protectedEnvironmentApiService
                  .DeleteProtectedEnvironment({
                    envId: item.uuid
                  })
                  .subscribe(
                    res => {
                      resolve(res);
                    },
                    err => {
                      reject(err);
                    }
                  );
              })
            );
          } else {
            promises.push(
              new Promise((resolve, reject) => {
                this.hostApiService
                  .deleteHostV1ResourceHostHostIdDelete({
                    hostId: item.uuid
                  })
                  .subscribe(
                    res => {
                      resolve(res);
                    },
                    err => {
                      reject(err);
                    }
                  );
              })
            );
          }
        });
        Promise.all(promises)
          .then(() => {
            this.selection = [];
            this.getHosts(refreshData);
          })
          .catch(() => {
            this.selection = [];
            this.getHosts(refreshData);
          });
      }
    });
  }

  trustworthinessHost(data) {
    this.messageBox.confirm({
      lvHeader: this.i18n.get('common_alarms_info_label'),
      lvContent: this.i18n.get('common_host_trustworthiness_info_label'),
      lvOk: () => {
        if (size(data) <= 1) {
          this.callTrustworthinessHostApi(head(data)).subscribe(() => {
            this.refresh();
          });
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.callTrustworthinessHostApi(item);
            },
            cloneDeep(data),
            () => this.refresh()
          );
        }
      }
    });
  }

  callTrustworthinessHostApi(data) {
    return this.protectedResourceApiService.UpdateResource({
      resourceId: data.uuid,
      UpdateResourceRequestBody: {
        name: data.name,
        extendInfo: {
          trustworthiness: 'true'
        }
      }
    });
  }

  cancelTrustworthinessHost(data) {
    this.warningMessageService.create({
      content: this.i18n.get('common_host_cancel_trustworthiness_warn_label'),
      onOK: () => {
        if (size(data) <= 1) {
          this.callCancelTrustworthinessHostApi(head(data)).subscribe(() =>
            this.refresh()
          );
        } else {
          this.batchOperateService.selfGetResults(
            item => {
              return this.callCancelTrustworthinessHostApi(item);
            },
            cloneDeep(data),
            () => this.refresh()
          );
        }
      }
    });
  }

  callCancelTrustworthinessHostApi(data) {
    return this.protectedResourceApiService.UpdateResource({
      resourceId: data.uuid,
      UpdateResourceRequestBody: {
        name: data.name,
        extendInfo: {
          trustworthiness: 'false'
        }
      }
    });
  }

  manualBackup(data) {
    this.takeManualBackupService.execute(
      assign(data, {
        proxy_id: 'test123',
        host_ip: data.environment_endpoint,
        resource_id: data.uuid,
        resource_type: data.sub_type
      }),
      () => this.getHosts(data)
    );
  }

  downloadBackupProxy(datas) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvWidth: MODAL_COMMON.normalWidth,
      lvHeader: this.i18n.get('protection_protect_agent_pkg_mangament_label'),
      lvOkDisabled: true,
      lvContent: DownloadProxyComponent,
      lvComponentParams: {
        data: false
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as DownloadProxyComponent;
        content.formGroup.statusChanges.subscribe(res => {
          if (
            content.formGroup.value.backupProxyFile ===
            DataMap.Backup_Proxy_File.DownLoad.value
          ) {
            modal.lvOkDisabled = res !== 'VALID';
            return;
          }
          modal.lvOkDisabled = true;
          content.valid$.subscribe(res => {
            modal.lvOkDisabled = !res;
          });
        });
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as DownloadProxyComponent;
        if (
          content.formGroup.value.backupProxyFile ===
          DataMap.Backup_Proxy_File.DownLoad.value
        ) {
          content.download();
          return;
        }
        content.upload();
      }
    });
  }

  SynchronizingConfigurations() {
    this.clientManagerApiService
      .updateAgentContainerFrontEndIpPUT({})
      .subscribe();
  }

  clientRegister() {
    this.router.navigateByUrl(RouterUrl.ProtectionHostAppHostRegister);
  }

  updateAgent(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'updateAgentKey',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvHeader: this.i18n.get('protection_update_agent_label'),
        lvContent: UpdateAgentComponent,
        lvComponentParams: {
          selection: data ? [data] : this.selection
        },
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: true,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as UpdateAgentComponent;
          const modalIns = modal.getInstance();
          content.isChecked$.subscribe(res => {
            modalIns.lvOkDisabled = !res;
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as UpdateAgentComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.selection = [];
                this.getHosts(data);
              },
              error: () => resolve(false)
            });
          });
        }
      }
    });
  }

  synchSnmpTrapInfo(data) {
    const trapParams = this.snmpApiService.getTrapConfigUsingGET({});
    const trapList = this.snmpApiService.queryTrapAddressListUsingGET({});
    combineLatest([trapParams, trapList]).subscribe(res => {
      const trap1 = res[0],
        trap2 = res[1];
      if (!trap1.version || !size(trap2)) {
        this.messageService.error(
          this.i18n.get('protection_trap_sync_failed_label'),
          {
            lvMessageKey: 'lvMsg_key_trapSyncFailed',
            lvShowCloseButton: true
          }
        );
        return;
      }
      this.hostApiService
        .synchronizeSnmpToHostV1ResourceHostHostIdActionSyncSnmpConfPost({
          hostId: data.uuid,
          akOperationTipsContent: this.i18n.get(
            'protection_trap_command_successfully_label'
          )
        })
        .subscribe(() => this.getHosts());
    });
  }

  configLanFree(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvHeader: this.i18n.get('protection_config_lanfree_label'),
      lvOkDisabled: false,
      lvContent: LanFreeComponent,
      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Observable((observer: Observer<void>) => {
          const content = modal.getContentComponent() as LanFreeComponent;
          content.onOK().subscribe(
            () => {
              this.getHosts();
              observer.next();
              observer.complete();
            },
            () => {
              observer.error(null);
              observer.complete();
            }
          );
        });
      }
    });
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
    assign(this.filterParams, {
      uuid: trim(value)
    });
    this.getHosts();
  }

  searchByName(value) {
    assign(this.filterParams, {
      name: trim(value)
    });
    this.getHosts();
  }

  searchByVersion(value) {
    assign(this.filterParams, {
      version: trim(value)
    });
    this.getHosts();
  }

  searchByIp(value) {
    assign(this.filterParams, {
      endpoint: trim(value)
    });
    this.getHosts();
  }

  searchBySlaName(value) {
    assign(this.filterParams, {
      slaName: trim(value)
    });
    this.getHosts();
  }

  searchByAuthorizedUser(value) {
    assign(this.filterParams, {
      authorizedUser: trim(value)
    });
    this.getHosts();
  }

  searchByRemark(value) {
    assign(this.filterParams, {
      tag: trim(value)
    });
    this.getHosts();
  }

  searchByLabel(label) {
    assign(this.filterParams, {
      labelName: trim(label)
    });
    this.getHosts();
  }

  disableMorenBtn(item) {
    if (item.id === 'deleteHost') {
      item.disabled = some(this.selection, select => {
        return (
          select.sla_id ||
          select.link_status !==
            DataMap.resource_Host_LinkStatus.offline.value ||
          !this.hasClientPermission(select)
        );
      });
    } else if (item.id === 'trustworthinessHost') {
      item.disabled =
        size(
          filter(this.selection, val => {
            return !val.trustworthiness && this.hasClientPermission(val);
          })
        ) !== size(this.selection) || !size(this.selection);
    } else if (item.id === 'cancelTrustworthinessHost') {
      item.disabled =
        size(
          filter(this.selection, val => {
            return !!val.trustworthiness && this.hasClientPermission(val);
          })
        ) !== size(this.selection) || !size(this.selection);
    } else if (item.id === 'exportLog') {
      let uncontect = false;
      each(this.selection, select => {
        if (
          select.link_status !== DataMap.resource_Host_LinkStatus.normal.value
        ) {
          uncontect = true;
        }
      });
      item.disabled =
        size(this.selection) > this.VERSION_VERIFY ||
        uncontect ||
        some(this.selection, select => !this.hasClientPermission(select));
      if (!item.disabled) {
        item.tips = '';
      }

      if (uncontect) {
        item.tips = this.i18n.get('common_some_host_offline_label');
      }
      if (size(this.selection) > this.VERSION_VERIFY) {
        item.tips = this.i18n.get('common_export_most_log_label', [
          this.VERSION_VERIFY
        ]);
      }
    } else if (item.id === 'configLogLevel') {
      item.disabled = false;
      each(this.selection, select => {
        if (
          select.link_status !==
            DataMap.resource_Host_LinkStatus.normal.value ||
          !this.hasClientPermission(select)
        ) {
          item.disabled = true;
          item.tips = this.i18n.get('common_some_host_offline_label');
        }
      });
      if (!item.disabled) {
        item.tips = '';
      }
    } else if (item.id === 'addTag') {
      item.disabled = some(this.selection, select => {
        return !this.hasClientPermission(select);
      });
    } else if (item.id === 'rescan') {
      item.disabled = some(
        this.selection,
        select =>
          select.link_status !==
            DataMap.resource_Host_LinkStatus.normal.value ||
          !this.hasClientPermission(select) ||
          this.compareVersion(select)
      );
    } else if (
      includes(
        ['openAutoSynchronizeHostName', 'closeAutoSynchronizeHostName'],
        item.id
      )
    ) {
      item.disabled = some(
        this.selection,
        select => !this.hasClientPermission(select)
      );
    } else if (item.id === 'addLabel') {
      item.disabled = !size(this.selection);
    } else if (item.id === 'removeLabel') {
      item.disabled = !size(this.selection);
    }
  }

  selectionChange(source) {
    each(this.moreMenus, item => {
      if (!size(this.selection)) {
        return (item.disabled = true);
      }
      this.disableMorenBtn(item);
    });

    this.updateAgentBtnDisabled =
      some(this.selection, val => {
        return (
          !(
            val.link_status === DataMap.resource_Host_LinkStatus.normal.value &&
            val.extendInfo?.agentUpgradeable === '1'
          ) || !this.hasClientPermission(val)
        );
      }) ||
      !size(this.selection) ||
      size(this.selection) > 50;
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getHosts();
  }

  trackByUuid(index: number, list: any) {
    return list.uuid;
  }

  rescan(data) {
    this.batchOperateService.selfGetResults(
      item => {
        return this.protectedResourceApiService.ScanProtectedResources({
          akOperationTips: false,
          akDoException: false,
          akLoading: false,
          resId: item.uuid
        });
      },
      map(cloneDeep(data), v => {
        return assign(v, {
          isAsyn: true
        });
      }),
      () => {
        this.selection = [];
        this.getHosts();
      },
      '',
      true
    );
  }

  addTag(data) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'set-quota',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('common_setting_label'),
        lvContent: AddTagComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          item: data
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddTagComponent;
          const modalIns = modal.getInstance();

          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res === 'INVALID';
          });

          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddTagComponent;
            const params = {
              tagInfo: content.formGroup.value.remarks
            };

            if (isArray(data)) {
              set(params, 'resourceIds', join(map(data, 'uuid'), ','));
            } else {
              set(params, 'resourceIds', data.uuid);
            }
            this.clientManagerApiService
              .updateTagInfoUsingPUT(params as any)
              .subscribe({
                next: res => {
                  resolve(true);
                  this.getHosts(data);
                },
                error: error => resolve(false)
              });
          });
        }
      })
    );
  }

  isActive(item): boolean {
    return item.uuid === this.activeItem?.uuid;
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }
}
