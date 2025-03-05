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
import { DatePipe } from '@angular/common';
import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnDestroy,
  OnInit,
  Pipe,
  PipeTransform,
  ViewChild
} from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import {
  DatatableComponent,
  MessageService,
  PaginatorComponent
} from '@iux/live';
import {
  AlarmAndEventApiService,
  AntiRansomwarePolicyApiService,
  ApiMultiClustersService,
  CookieService,
  GlobalService,
  WarningMessageService
} from 'app/shared';
import { AlarmVO } from 'app/shared/api/models';
import {
  ALARM_EVENT_EVENT_STATUS,
  ALARM_EVENT_EVENT_TYPE,
  ALARM_EVENT_TYPE,
  ALARM_NAVIGATE_STATUS,
  CommonConsts,
  DataMap,
  LANGUAGE,
  MODAL_COMMON,
  SYSTEM_TIME
} from 'app/shared/consts';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { I18NService } from 'app/shared/services/i18n.service';
import { RememberColumnsService } from 'app/shared/services/remember-columns.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  capitalize,
  cloneDeep,
  defer,
  each,
  eq,
  filter,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isNil,
  isNumber,
  isString,
  isUndefined,
  map,
  now,
  reject,
  remove,
  set,
  size,
  split,
  toLower,
  toString,
  trim
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { DataMapService } from './../../../shared/services/data-map.service';
import { AlarmsClearComponent } from './alarms-clear/alarms-clear.component';
import { AlarmsDetailsComponent } from './alarms-details/alarms-details.component';

@Pipe({ name: 'selectionEnable' })
export class SelectionPipe implements PipeTransform {
  constructor(private i18n: I18NService) {}
  transform(value: any[], exponent: string = 'sourceType') {
    return filter(
      value,
      item =>
        !(
          item[exponent] === 'operation_target_ibmc_label' &&
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value
        )
    );
  }
}

@Component({
  selector: 'alarms',
  templateUrl: './alarms.component.html',
  styleUrls: ['./alarms.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  providers: [DatePipe]
})
export class AlarmsComponent implements OnInit, OnDestroy {
  isSendTestAlarm = true;
  alarmIdSearched = false;
  alarmsData;
  eventsData;
  selected = [];
  eventsSections: Array<any> = [];
  alarmsSections: Array<any> = [];

  apiFilters = [];
  eventFilters = [];

  clusterMenus = [];
  nodeName = '';
  activeNode;
  activeNodeName;

  columns: any = [];
  eventFiltersMap;
  alarmFiltersMap;
  alarmEventType = ALARM_EVENT_TYPE;
  activeIndex = ALARM_EVENT_TYPE.ALARM;

  @ViewChild('alarmsTable', { static: false }) alarmsTable: DatatableComponent;
  @ViewChild('alarmsPage', { static: true }) alarmsPage: PaginatorComponent;
  @ViewChild('eventsTable', { static: false }) eventsTable: DatatableComponent;
  @ViewChild('eventsPage', { static: true }) eventsPage: PaginatorComponent;
  @ViewChild('colPopover', { static: false }) popover;

  queryAlarmId;
  queryNodeNameId;
  queryAlarmObject = [];

  alarmTotalCount = 0;
  alarmPageSize = CommonConsts.PAGE_SIZE;
  alarmStartPage = CommonConsts.PAGE_START;
  alarmSeverity = [];
  alarmCols = [];
  alarmSizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  eventSizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  objTypes = [];
  eventTotalCount = 0;
  eventPageSize = CommonConsts.PAGE_SIZE;
  eventStartPage = CommonConsts.PAGE_START;
  eventSeverity = [];
  deviceNames = [];
  eventCols = [];
  eventType = [];
  @ViewChild('datePopover', { static: false }) datePopover;
  dateMap: any = {};
  rangeDate;

  eventTypeParam: any;
  eventColFilterMap: any = {};

  timeSub1$: Subscription;
  destroy1$ = new Subject();

  timeSub2$: Subscription;
  destroy2$ = new Subject();

  timeSub3$: Subscription;
  destroy3$ = new Subject();

  detailStore$: Subscription = new Subscription();

  alarmColumnStatus = this.rememberColumnsService.getColumnsStatus(
    'alarm_table'
  );
  eventColumnStatus = this.rememberColumnsService.getColumnsStatus(
    'event_table'
  );

  useStaticSourceType =
    this.appUtilsService.isDecouple || this.appUtilsService.isDistributed;
  isHyperDetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;

  constructor(
    public alarmApiService: AlarmAndEventApiService,
    public drawModalService: DrawModalService,
    public i18n: I18NService,
    public router: Router,
    public route: ActivatedRoute,
    public datePipe: DatePipe,
    public globalService: GlobalService,
    public batchOperateService: BatchOperateService,
    private dataMapService: DataMapService,
    private rememberColumnsService: RememberColumnsService,
    public virtualScroll: VirtualScrollService,
    private cdr: ChangeDetectorRef,
    private messageService: MessageService,
    private cookieService: CookieService,
    private multiClustersServiceApi: ApiMultiClustersService,
    public appUtilsService: AppUtilsService,
    private warningMessageService: WarningMessageService,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService
  ) {}

  id = this.i18n.get('insight_alarm_id_label');
  alarm = this.i18n.get('common_alarms_label');
  event = this.i18n.get('insight_event_title_label');
  clear = this.i18n.get('common_clear_label');
  object = this.i18n.get('insight_alarm_object_label');
  typeLabel = this.i18n.get('common_type_label');
  alarmName = this.i18n.get('common_name_label');
  occurred = this.i18n.get('insight_alarm_occurred_label');
  severity = this.i18n.get('insight_alarm_severity_label');
  statusLabel = this.i18n.get('common_status_label');
  sequenceNo = this.i18n.get('insight_event_squence_no_label');
  recovered = this.i18n.get('insight_event_recovered_time_label');
  alarmDetail = this.i18n.get('insight_alarm_detail_title_label');
  eventDetail = this.i18n.get('insight_alarm_event_title_label');
  exportAlarmTipLabel = this.i18n.get('common_export_all_label', [this.alarm]);
  exportEventTipLabel = this.i18n.get('common_export_all_label', [this.event]);
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isDecouple = this.appUtilsService.isDecouple;
  isV1AlarmQuery =
    this.isCyberEngine || this.isDecouple || this.appUtilsService.isDistributed;
  deviceNameMap = [];
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  isDistributed = this.appUtilsService.isDistributed;
  alarmHelp = this.isDistributed
    ? this.i18n.get('protetion_e6000_alarm_help_label')
    : this.i18n.get('protetion_alarm_help_label');
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );
  ngOnDestroy() {
    this.destroy1$.next(true);
    this.destroy1$.complete();
    this.destroy2$.next(true);
    this.destroy2$.complete();
    this.destroy3$.next(true);
    this.destroy3$.complete();
    this.detailStore$.unsubscribe();
    this._clearAlarmNavigateStatus();
    this.appUtilsService.clearCacheValue('alarmSeverity');
  }

  ngOnInit() {
    this.initFilter();
    this.updateFilter();
    this.updateEventFilter();
    this.initCols();
    this.columns = [].concat(this.alarmCols);
    this.virtualScroll.getScrollParam(300, 3);
    this.setClusterMenuHeight();
    this.detailStore$ = this.globalService
      .getState('getAlarmDetail')
      .subscribe(() => {
        this.activeIndex = ALARM_EVENT_TYPE.ALARM;
        this.jumpToAlarmAndOpenDetail();
      });
    this.alarmApiService
      .findObjectsPageUsingGET({
        language: this.i18n.language === 'zh-cn' ? 'ZH' : 'EN'
      })
      .subscribe(res => {
        this.objTypes = map(res as any[], item => ({
          key: this.isV1AlarmQuery ? item?.name : item?.id,
          value: this.isV1AlarmQuery
            ? capitalize(get(split(item?.name, '_'), '2', '')) // 'operation_target_xxx_label'
            : item?.type,
          label: this.i18n.get(item?.name)
        }));
        if (this.isDataBackup) {
          this.initCols();
          this.getClusterNodes();
        } else {
          this.initCols().refreshAlarms();
        }
      });
    window.addEventListener('resize', () => {
      this.virtualScroll.getScrollParam(300, 3);
      this.setClusterMenuHeight();
    });
  }

  private jumpToAlarmAndOpenDetail() {
    if (!isEmpty(ALARM_NAVIGATE_STATUS.sequence)) {
      const selectedAlarm = this.alarmsData.find(
        item => item.sequence.toString() === ALARM_NAVIGATE_STATUS.sequence
      );
      if (selectedAlarm) {
        this.alarmsTable.clearSelection();
        if (selectedAlarm.sourceType !== 'operation_target_ibmc_label') {
          this.alarmsTable.toggleSelection(selectedAlarm);
        }
        this.openDetailModal(selectedAlarm);
      }
      ALARM_NAVIGATE_STATUS.sequence = '';
    }
  }

  alarmHelpHover() {
    const url = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#admin_email_save_000000.html'
      : '/console/assets/help/a8000/zh-cn/index.html#helpcenter_000192.html';
    this.appUtilsService.openSpecialHelp(url);
  }

  initSeverityFilter() {
    if (!this.isCyberEngine) {
      return;
    }
    const cacheSeverity = this.appUtilsService.getCacheValue(
      'alarmSeverity',
      false
    );
    if (cacheSeverity) {
      this.alarmSeverity = [cacheSeverity];
      const _valMapping = {
        WARNING: 1,
        MAJOR: 3,
        CRITICAL: 4
      };
      each(this.alarmCols, col => {
        if (col.key === 'severity') {
          col.filterMap = filter(col.filterMap, val => {
            if (_valMapping[val.value] === cacheSeverity) {
              val.selected = true;
            }
            return true;
          });
        }
      });
    }
  }

  setClusterMenuHeight() {
    defer(() => {
      const clusterMenu = first(
        document.getElementsByClassName('cluster-menus')
      );
      if (clusterMenu) {
        clusterMenu.setAttribute(
          'style',
          `max-height: ${parseInt(this.virtualScroll.scrollParam.y) + 30}px`
        );
      }
    });
  }

  getClusterNodes(nodeName?) {
    const params = {};
    set(params, 'nodeName', nodeName || '');
    let isFirst = true;

    if (this.timeSub3$) {
      this.timeSub3$.unsubscribe();
    }

    this.timeSub3$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          isFirst = index === 0;
          return this.alarmApiService.getMultiClusterNodeAlarmsUsingGET({
            ...params,
            akLoading: !index
          });
        }),
        takeUntil(this.destroy3$)
      )
      .subscribe(res => {
        if (!size(res)) {
          this.alarmsData = [];
          this.alarmTotalCount = 0;
          this.activeNode = null;
          this.activeNodeName = '';
          this.clusterMenus = [];
          this.cdr.detectChanges();
          if (this.timeSub1$) {
            this.timeSub1$.unsubscribe();
          }
          return;
        }
        const nodes = map(res, item => {
          return {
            ...item,
            id: get(item, 'esn'),
            label: item.nodeName,
            disabled: item.status !== DataMap.Cluster_Status.online.value
          };
        });

        this.clusterMenus = nodes;
        this.activeNode = this.activeNode || get(this.clusterMenus, '[0].id');
        this.activeNodeName =
          this.activeNodeName || get(this.clusterMenus, '[0].label');
        this.refreshAlarms(isFirst);
        this.cdr.detectChanges();
      });
  }

  search() {
    this.getClusterNodes(this.nodeName);
  }

  refresh() {
    this.getClusterNodes(this.nodeName);
  }

  nodeChange(event) {
    this.alarmsSections = [];
    this.activeNode = event.data.esn;
    this.activeNodeName = event.data.label;
    this.refreshAlarms();
  }

  onChange() {
    this.ngOnInit();
    if (this.activeIndex === ALARM_EVENT_TYPE.ALARM) {
      this.refreshAlarms();
    } else {
      this.refreshEvent();
    }
  }
  getObjType(val) {
    return this.objTypes.find(item => item.value === val)?.label || val;
  }

  filterEventSourceType(sourceTypes) {
    if (this.cookieService.isCloudBackup) {
      return reject(sourceTypes, item => {
        return includes(
          [
            'Cluster',
            'LiveMount',
            'antiRansomwarePolicy',
            'Kerberos',
            'archive',
            'externalSystem',
            'resourceset',
            'role',
            'agentManager',
            'vmware'
          ],
          item.key
        );
      });
    }
    if (this.isX3000) {
      return reject(sourceTypes, item => {
        return includes(
          ['anonymization', 'detection', 'antiRansomwarePolicy', 'archive'],
          item.key
        );
      });
    }
    if (this.appUtilsService.isDistributed) {
      return reject(sourceTypes, item => {
        return includes(['Replication'], item.key);
      });
    }
    return sourceTypes;
  }

  getSourceTypes() {
    if (this.isV1AlarmQuery && !this.useStaticSourceType) {
      return this.objTypes;
    }
    const sourceTypes = [
      {
        key: 'User',
        value: 'User',
        label: this.i18n.get('operation_target_user_label')
      },
      {
        key: 'Alarm',
        value: 'Alarm',
        label: this.i18n.get('operation_target_alarm_label')
      },
      {
        key: 'Cluster',
        value: 'Cluster',
        label: this.i18n.get('operation_target_cluster_label')
      },
      {
        key: 'Repository',
        value: 'Repository',
        label: this.i18n.get('operation_target_repository_label')
      },
      {
        key: 'License',
        value: 'License',
        label: this.i18n.get('operation_target_license_label')
      },
      {
        key: 'KMS',
        value: 'KMS',
        label: this.i18n.get('operation_target_kms_label')
      },
      {
        key: 'Certificate',
        value: 'Certificate',
        label: this.i18n.get('operation_target_certificate_label')
      },
      {
        key: 'Resource',
        value: 'Resource',
        label: this.i18n.get('operation_target_resource_label')
      },
      {
        key: 'SLA',
        value: 'SLA',
        label: this.i18n.get('operation_target_sla_label')
      },
      {
        key: 'ProtectedObject',
        value: 'ProtectedObject',
        label: this.i18n.get('operation_target_protectedobject_label')
      },
      {
        key: 'Protection',
        value: 'Protection',
        label: this.i18n.get('operation_target_protection_label')
      },
      {
        key: 'Scheduler',
        value: 'Scheduler',
        label: this.i18n.get('operation_target_scheduler_label')
      },
      {
        key: 'LiveMount',
        value: 'LiveMount',
        label: this.i18n.get('operation_target_livemount_label')
      },
      {
        key: 'Restore',
        value: 'Restore',
        label: this.i18n.get('operation_target_restore_label')
      },
      {
        key: 'CopyCatalog',
        value: 'CopyCatalog',
        label: this.i18n.get('operation_target_copycatalog_label')
      },
      {
        key: 'BackUpCluster',
        value: 'BackUpCluster',
        label: this.i18n.get('operation_target_backupcluster_label')
      },
      {
        key: 'System',
        value: 'System',
        label: this.i18n.get('operation_target_system_label')
      },
      {
        key: 'Replication',
        value: 'Replication',
        label: this.i18n.get('operation_target_replication_label')
      },
      {
        key: 'Localstorage',
        value: 'Localstorage',
        label: this.i18n.get('operation_target_localstorage_label')
      },
      {
        key: 'anonymization',
        value: 'anonymization',
        label: this.i18n.get('operation_target_anonymization_label')
      },
      {
        key: 'job',
        value: 'job',
        label: this.i18n.get('operation_target_job_label')
      },
      {
        key: 'detection',
        value: 'detection',
        label: this.i18n.get('common_detection_setting_label')
      },
      {
        key: 'antiRansomwarePolicy',
        value: 'antiRansomwarePolicy',
        label: this.i18n.get('operation_target_antiransomwarepolicy_label')
      },
      {
        key: 'environment',
        value: 'environment',
        label: this.i18n.get('operation_target_environment_label')
      },
      {
        key: 'Kerberos',
        value: 'Kerberos',
        label: this.i18n.get('Kerberos')
      },
      {
        key: 'FileExport',
        value: 'FileExport',
        label: this.i18n.get('operation_target_fileexport_label')
      },
      {
        key: 'report',
        value: 'report',
        label: this.i18n.get('operation_target_report_label')
      },
      {
        key: 'archive',
        value: 'archive',
        label: this.i18n.get('operation_target_archive_label')
      },
      {
        key: 'backupStorageUnitGroup',
        value: 'backup_storage_unit_group',
        label: this.i18n.get('operation_target_backup_storage_unit_group_label')
      },
      {
        key: 'externalSystem',
        value: 'externalsystem',
        label: this.i18n.get('operation_target_externalsystem_label')
      },
      {
        key: 'resourceset',
        value: 'resourceset',
        label: this.i18n.get('operation_target_resourceset_label')
      },
      {
        key: 'role',
        value: 'role',
        label: this.i18n.get('operation_target_role_label')
      },
      {
        key: 'agentManager',
        value: 'agent_manager',
        label: this.i18n.get('operation_target_agent_manager_label')
      },
      {
        key: 'vmware',
        value: 'vmware',
        label: this.i18n.get('operation_target_vmware_label')
      }
    ];
    return this.filterEventSourceType(sourceTypes);
  }

  initCols() {
    const cols = [];
    const alarmsCol = [
      {
        label: this.severity,
        key: 'severity',
        isShow: true,
        filter: true,
        filterMap: this.alarmFiltersMap.AlarmSeverityOptions,
        width: 150
      },
      {
        label: this.i18n.get('common_desc_label'),
        isShow: true,
        key: this.isV1AlarmQuery ? 'desc' : 'description',
        width: 400
      },
      {
        label: this.i18n.get('protection_equipment_name_label'),
        isShow: true,
        key: 'deviceName',
        filter: true,
        filterMap: this.deviceNameMap
      },
      {
        label: this.object,
        key: this.isV1AlarmQuery ? 'sourceType' : 'objType',
        isShow: true,
        filter: true,
        filterMap: this.useStaticSourceType
          ? this.getSourceTypes()
          : this.objTypes
      },
      {
        label: this.occurred,
        key: this.isV1AlarmQuery ? 'firstTimeStr' : 'alarmTimeStr',
        isShow: true
      },
      {
        label: this.id,
        key: 'alarmId',
        isShow: false
      },
      {
        label: this.sequenceNo,
        key: 'sequence',
        isShow: false
      }
    ];
    if (!isEmpty(this.alarmColumnStatus)) {
      each(alarmsCol, col => {
        col.isShow = this.alarmColumnStatus[col.key];
      });
    }

    const eventsCol = [
      {
        label: this.severity,
        key: 'severity',
        isShow: true,
        filter: true,
        filterMap: this.eventFiltersMap.EventSeverityOptions,
        width: 150
      },
      {
        label: this.i18n.get('common_desc_label'),
        isShow: true,
        key: 'desc',
        width: 400
      },
      {
        label: this.i18n.get('protection_equipment_name_label'),
        isShow: true,
        key: 'deviceName',
        filter: true,
        filterMap: this.deviceNameMap
      },
      {
        label: this.object,
        key: 'sourceType',
        isShow: true,
        filter: true,
        filterMap: this.getSourceTypes()
      },
      {
        label: this.i18n.get('system_servers_label'),
        key: 'nodeName',
        isShow: true
      },
      {
        label: this.recovered,
        key: 'clearTimeStr',
        isShow: true
      },
      {
        label: this.occurred,
        key: 'firstTimeStr',
        isShow: true
      },
      {
        label: this.id,
        key: 'alarmId',
        isShow: false
      },
      {
        label: this.sequenceNo,
        key: 'sequence',
        isShow: false
      },
      {
        label: this.typeLabel,
        key: 'type',
        isShow: false,
        filter: true,
        filterMap: this.eventFiltersMap.typeOptions
      },
      {
        label: this.statusLabel,
        key: 'confirmStatus',
        isShow: false,
        filter: true,
        filterMap: this.eventFiltersMap.confirmStatusOptions
      }
    ];
    if (!isEmpty(this.eventColumnStatus)) {
      each(eventsCol, col => {
        col.isShow = this.eventColumnStatus[col.key];
      });
    }

    if (!this.isCyberEngine) {
      each([alarmsCol, eventsCol], col =>
        remove(col, item => eq(item.key, 'deviceName'))
      );
    }
    if (!this.isDataBackup) {
      each([alarmsCol, eventsCol], col =>
        remove(col, item => eq(item.key, 'nodeName'))
      );
    }
    this.alarmCols = cloneDeep(cols.concat(alarmsCol));
    this.eventCols = cloneDeep(cols.concat(eventsCol));
    this.initSeverityFilter();
    return this;
  }

  initFilter() {
    const AlarmSeverityOptions = this.dataMapService
        .toArray('Alarm_Severity_Type')
        .filter(item => item.value !== DataMap.Alarm_Severity_Type.info.value)
        .map(item => ({ ...item, value: item.key.toUpperCase() })),
      EventSeverityOptions = this.dataMapService.toArray('Alarm_Severity_Type'),
      typeOptions = this.getEventTypeOptions(),
      confirmStatusOptions = this.getEventConfirmStatusOptions();
    this.alarmFiltersMap = cloneDeep({ AlarmSeverityOptions });
    this.eventFiltersMap = cloneDeep({
      EventSeverityOptions,
      typeOptions,
      confirmStatusOptions
    });
    this.isCyberEngine &&
      this.alarmApiService
        .findPageCEDevUsingGET({
          language: this.i18n.language === 'zh-cn' ? 'ZH' : 'EN',
          akLoading: false
        })
        .subscribe(res => {
          const getName = item => trim(get(item, 'name', '--'));
          if (!size(res)) {
            return;
          }
          this.deviceNameMap = map(res, item => ({
            key: getName(item),
            value: getName(item),
            label: getName(item),
            selected: false
          }));
          this.initCols();
        });
  }

  beforeChange = (origin: any, active: number) => {
    this.alarmsData = [];
    this.eventsData = [];
    this._clearAlarmNavigateStatus();
  };

  afterChange = (activeIndex: any) => {
    this.eventsSections = [];
    this.alarmsSections = [];
    this.queryAlarmId = '';
    this.dateMap = {};
    this.rangeDate = null;
    if (activeIndex === this.alarmEventType.ALARM) {
      this.columns = this.alarmCols;
      if (this.timeSub2$) {
        this.timeSub2$.unsubscribe();
      }
      this.refreshAlarms();
    } else {
      this.clearAllEventTag();
      this.columns = this.eventCols;
      if (this.timeSub1$) {
        this.timeSub1$.unsubscribe();
      }
      if (this.timeSub3$) {
        this.timeSub3$.unsubscribe();
      }
      this.refreshEvent();
    }
  };

  /**
   * 安全一体机 设备名称（后端将字段放在了 params 最后一项）
   * @param row 行信息
   * @returns 处理后的行信息
   */
  private setRowDeviceName(row: AlarmVO) {
    return row;
  }

  private _clearAlarmNavigateStatus() {
    ALARM_NAVIGATE_STATUS.location = '';
    ALARM_NAVIGATE_STATUS.alarmId = '';
  }

  refreshAlarms(isFirst?) {
    const params: { [key: string]: any } = {
      pageSize: this.alarmPageSize,
      pageNo: this.isV1AlarmQuery ? void 0 : this.alarmStartPage,
      pageNum: this.isV1AlarmQuery ? this.alarmStartPage : void 0,
      memberEsn: this.activeNode || ''
    };

    if (size(this.alarmSeverity) === 1 && !this.isV1AlarmQuery) {
      params.severity = first(this.alarmSeverity);
    }
    if (this.isV1AlarmQuery && size(this.alarmSeverity)) {
      remove(this.alarmSeverity, val => eq(val, 0)); // 告警不包含提示
      const _valMapping = {
        warning: 1,
        major: 3,
        critical: 4
      };
      const filtersMap = map(
        this.alarmFiltersMap.AlarmSeverityOptions,
        item => {
          return { ...item, value: _valMapping[item.key] };
        }
      );
      params.severities = map(this.alarmSeverity, item => {
        if (isString(item)) {
          return get(
            find(filtersMap, _item => eq(_item.key, item.toLowerCase())),
            'value'
          );
        }
        return item;
      });
    }
    if (size(this.queryAlarmObject) === 1 && !this.isV1AlarmQuery) {
      params.objType = first(this.queryAlarmObject);
    }
    if (this.isV1AlarmQuery) {
      params.sourceTypes = this.queryAlarmObject;
      params.deviceNames = this.deviceNames;

      if (this.dateMap.begin && this.dateMap.end) {
        params.createTimeStart = this.datePipe.transform(
          this.dateMap.begin,
          'yyyy-MM-dd HH:mm:ss'
        );
        params.createTimeEnd = this.datePipe.transform(
          this.dateMap.end,
          'yyyy-MM-dd HH:mm:ss'
        );
      }
    }

    if (!isEmpty(this.queryAlarmId)) {
      params.alarmId = this.queryAlarmId;
    }
    if (this.dateMap.begin && this.dateMap.end && !this.isV1AlarmQuery) {
      params.startTime = parseInt(
        (
          this.appUtilsService.toSystemTimeLong(this.dateMap.begin) / 1000
        ).toFixed(0)
      );
      params.endTime = parseInt(
        (
          this.appUtilsService.toSystemTimeLong(this.dateMap.end) / 1000
        ).toFixed(0)
      );
    }

    each(params, (value, key) => {
      if (!isNumber(value) && isEmpty(value)) {
        delete params[key];
      }
    });

    if (ALARM_NAVIGATE_STATUS.location && ALARM_NAVIGATE_STATUS.alarmId) {
      assign(params, {
        location: ALARM_NAVIGATE_STATUS.location,
        alarmId: ALARM_NAVIGATE_STATUS.alarmId
      });
    }

    if (this.timeSub1$) {
      this.timeSub1$.unsubscribe();
    }
    const intervalTime = this.isDataBackup ? null : CommonConsts.TIME_INTERVAL;
    this.timeSub1$ = timer(0, intervalTime)
      .pipe(
        switchMap(index => {
          if (this.isV1AlarmQuery) {
            return this.alarmApiService.getAlarmListUsingGET({
              ...params,
              language: this.i18n.language === 'zh-cn' ? 'zh' : 'en',
              akLoading: !index
            });
          }
          return this.alarmApiService.findPageUsingGET({
            ...params,
            language: this.i18n.language === 'zh-cn' ? 'ZH' : 'EN',
            akLoading: this.isDataBackup ? isFirst : !index
          });
        }),
        takeUntil(this.destroy1$)
      )
      .subscribe(res => {
        this.alarmTotalCount = get(
          res,
          this.isV1AlarmQuery ? 'total' : 'totalCount'
        );
        this.alarmsData = res?.records || [];
        this.alarmsData = this.alarmsData.map(val => {
          this.setRowDeviceName(val);
          const severityTypes = this.dataMapService.toArray(
            'Alarm_Severity_Type'
          );
          const severityType = find(severityTypes, item => {
            return item.value === val.severity;
          });
          val.severityLabel = this.dataMapService.getLabel(
            'Alarm_Severity_Type',
            val.severity
          );
          val.severity = severityType ? severityType.key : '';
          val.sequence = val.sequence === -1 ? '--' : val.sequence;
          if (!this.appUtilsService.isDistributed) {
            val.alarmTimeStr = this.getAlarmTimeStr(val.alarmTime);
          }
          val.objType = this.getObjType(val.objType);
          return val;
        });
        this.jumpToAlarmAndOpenDetail();
        this.cdr.detectChanges();
        this.isV1AlarmQuery
          ? this.getAnalogCyberAlarms()
          : this.getAnalogAlarms();
      });
  }

  private getAlarmTimeStr(timestamp, isSeconds = true) {
    if (isSeconds) timestamp = timestamp * 1000;
    return this.datePipe.transform(
      timestamp,
      'yyyy-MM-dd HH:mm:ss',
      SYSTEM_TIME.timeZone
    );
  }

  getAnalogAlarms() {
    const params = {
      severities: [DataMap.Alarm_Severity_Type.warning.value],
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START,
      language: (this.i18n.language === 'zh-cn' ? 'ZH' : 'EN') as 'ZH' | 'EN',
      akLoading: false,
      memberEsn: this.activeNode || ''
    };
    this.alarmApiService
      .findPageUsingGET({
        ...params
      })
      .subscribe(
        res => {
          this.isSendTestAlarm = !!res.records?.length;
        },
        () => {
          this.isSendTestAlarm = false;
        }
      );
  }

  getAnalogCyberAlarms() {
    const params = {
      severities: [DataMap.Alarm_Severity_Type.warning.value],
      pageSize: CommonConsts.PAGE_SIZE,
      pageNum: CommonConsts.PAGE_START,
      language: (this.i18n.language === 'zh-cn' ? 'zh' : 'en') as 'zh' | 'en',
      akLoading: false
    };
    if (this.isV1AlarmQuery) {
      set(params, 'language', get(params, 'language')?.toLowerCase());
    }
    this.alarmApiService
      .getAlarmListUsingGET({
        ...params
      })
      .subscribe(
        res => {
          this.isSendTestAlarm = !!res.records.length;
        },
        () => {
          this.isSendTestAlarm = false;
        }
      );
  }

  alarmObjectFilterChange(event) {
    this.queryAlarmObject = event.value;
    this.alarmStartPage = CommonConsts.PAGE_START;
    !this.isV1AlarmQuery && this._updateFiltersTag(event.key, event.value);
    this.refreshAlarms();
  }
  alarmDeviceChange(event) {
    if (!this.isCyberEngine) {
      return;
    }
    this.deviceNames = event.value;

    if (this.alarmEventType.ALARM === this.activeIndex) {
      this.refreshAlarms();
    } else {
      this.refreshEvent();
    }
  }

  searchByAlarmId(event) {
    this.queryAlarmId = trim(event);
    if (this.alarmEventType.ALARM === this.activeIndex) {
      !this.isV1AlarmQuery && this._updateFiltersTag('alarmId', event);
      this.alarmStartPage = CommonConsts.PAGE_START;
      this.refreshAlarms();
    } else {
      let tmpFilter = find(this.eventFilters, { key: 'alarmId' });
      if (tmpFilter) {
        assign(tmpFilter, {
          key: 'alarmId',
          value: this.queryAlarmId
        });
      } else {
        this.eventFilters.push({
          key: 'alarmId',
          value: this.queryAlarmId
        });
      }
      this.eventFilters = [...this.eventFilters];
      this.eventStartPage = CommonConsts.PAGE_START;
      this.refreshEvent();
    }
  }

  searchByNodeNameId(event) {
    this.queryNodeNameId = trim(event);
    this.eventStartPage = CommonConsts.PAGE_START;
    this.refreshEvent();
  }

  selectDate(e) {
    if (e) {
      this.dateMap = { begin: e[0] ? e[0] : '', end: e[1] ? e[1] : '' };
    }
    if (this.alarmEventType.ALARM === this.activeIndex) {
      this.alarmStartPage = CommonConsts.PAGE_START;
      !this.isV1AlarmQuery && this._updateFiltersTag('alarmTimeStr', e);
      this.refreshAlarms();
    } else {
      let tmpFilter = find(this.eventFilters, { key: 'firstTimeStr' });
      if (tmpFilter) {
        assign(tmpFilter, {
          key: 'firstTimeStr',
          value: e
        });
      } else {
        this.eventFilters.push({
          key: 'firstTimeStr',
          value: e
        });
      }
      this.eventFilters = [...this.eventFilters];
      this.eventStartPage = CommonConsts.PAGE_START;
      this.refreshEvent();
    }
    if (!isUndefined(this.datePopover)) {
      this.datePopover.hide();
    }
  }

  cancelDate(e) {
    this.dateMap = {};
    this.rangeDate = null;
    if (this.alarmEventType.ALARM === this.activeIndex) {
      this.refreshAlarms();
    } else {
      this.refreshEvent();
    }
    this.datePopover.hide();
  }

  alarmPageChange = e => {
    this.alarmPageSize = e.pageSize;
    this.alarmStartPage = e.pageIndex;
    this.refreshAlarms();
  };

  eventPageChange = e => {
    this.eventPageSize = e.pageSize;
    this.eventStartPage = e.pageIndex;
    this.refreshEvent();
  };

  changeVmwareType(params) {
    return map(params, p => {
      if (p === 'vim.VirtualMachine') {
        return 'common_vm_virtual_machine_label';
      } else if (p === 'vim.HostSystem') {
        return 'common_vm_host_system_label';
      } else if (p === 'vim.ClusterComputeResource') {
        return 'common_vm_virtual_cluster_label';
      } else {
        return p;
      }
    });
  }

  refreshEvent() {
    const params = {
      alarmId: this.queryAlarmId,
      nodeName: this.queryNodeNameId,
      pageSize: this.eventPageSize,
      pageNum: this.eventStartPage,
      severities: this.eventColFilterMap.severity,
      types: this.eventColFilterMap.type,
      confirmStatuses: this.eventColFilterMap.confirmStatus,
      sourceTypes: this.eventColFilterMap.sourceType,
      createTimeStart: this.datePipe.transform(
        this.dateMap.begin,
        'yyyy-MM-dd HH:mm:ss'
      ),
      createTimeEnd: this.datePipe.transform(
        this.dateMap.end,
        'yyyy-MM-dd HH:mm:ss'
      )
    };
    if (this.isCyberEngine) {
      set(params, 'deviceNames', this.deviceNames);
    }

    each(params, (value, key) => {
      if (!isNumber(value) && isEmpty(value)) {
        delete params[key];
      }
    });

    if (this.timeSub2$) {
      this.timeSub2$.unsubscribe();
    }

    this.timeSub2$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.alarmApiService.getEventListUsingGET({
            ...params,
            akLoading: !index,
            language: this.i18n.language
          });
        }),
        takeUntil(this.destroy2$)
      )
      .subscribe(res => {
        this.eventTotalCount = res.total;
        this.eventsData = res.records;
        this.eventsData = this.eventsData.map(val => {
          this.setRowDeviceName(val);
          val.confirmStatus = this.i18n.get(
            `common_alarms_status_${val.confirmStatus}_label`
          );
          val.type = this.i18n.get(`common_alarms_type_${val.type}_label`);
          const severityTypes = this.dataMapService.toArray(
            'Alarm_Severity_Type'
          );
          const severityType = find(severityTypes, item => {
            return item.value === val.severity;
          });
          val.severityLabel = this.dataMapService.getLabel(
            'Alarm_Severity_Type',
            val.severity
          );
          val.severity = severityType ? severityType.key : '';
          val.sequence = val.sequence === -1 ? '--' : val.sequence;
          if (!isEmpty(val.params)) {
            val.params = this.changeVmwareType(val.params);
          }
          return val;
        });
        this.cdr.detectChanges();
      });
  }

  clearAlarms() {
    const selectionData = cloneDeep(this.alarmsSections);
    const isAlarm = this.activeIndex === this.alarmEventType.ALARM;
    const isCyberEngine = this.isV1AlarmQuery;
    selectionData.map(item => {
      item.alarmName = this.i18n.get(item.alarmName, item.params || []);
      return item;
    });

    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvType: 'modal',
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('common_alarms_warning_label'),
        lvCloseButtonDisplay: true,
        lvContent: AlarmsClearComponent,
        lvComponentParams: {
          selectionData,
          isAlarm,
          isCyberEngine
        },
        lvOk: () => {
          this.clearCallback();
          this.isV1AlarmQuery
            ? this.getAnalogCyberAlarms()
            : this.getAnalogAlarms();
        }
      })
    );
  }

  clearCallback() {
    const isAlarm = this.activeIndex === this.alarmEventType.ALARM;
    const options = this.alarmsTable.getSelection();
    this.batchOperateService.selfGetResults(
      item => {
        if (this.appUtilsService.isDistributed || this.isDecouple) {
          return this.alarmApiService.clearAlarmsUsingPUT({
            alarmClearParam: { entityId: item.entityId },
            akDoException: false,
            akOperationTips: false,
            akLoading: false,
            memberEsn: this.activeNode || ''
          });
        }
        return this.alarmApiService.clearAlarmsBySequenceUsingPUT({
          sequence: item.sequence,
          akDoException: false,
          akOperationTips: false,
          akLoading: false,
          memberEsn: this.activeNode || ''
        });
      },
      map(cloneDeep(options), d => {
        if (this.isV1AlarmQuery) {
          return assign(d, {
            name: this.i18n.get(d.alarmName, d.params || [])
          });
        }
        return assign(d, {
          name: this.i18n.get(isAlarm ? d.name : d.alarmName, d.params || [])
        });
      }),
      () => {
        this.alarmsSections = [];
        if (this.isDataBackup) {
          this.getClusterNodes();
        }
        this.cdr.detectChanges();
      }
    );
  }

  exportCurrentAlarms() {
    this.alarmApiService
      .exportAlarmsUsingPOST({
        entityIds: { entityIdSet: [] },
        lang: this.i18n.language,
        akOperationTips: false
      })
      .subscribe(blob => {
        const file = new File([blob], `CurrentAlarms_${now()}.xls`, {
          type: 'application/vnd.ms.excel'
        });
        this.appUtilsService.downloadFile(
          this.activeNodeName
            ? `HistoryAlarms_${now()}_Node_${this.activeNodeName}.xls`
            : `HistoryAlarms_${now()}.xls`,
          file
        );
        this.cdr.detectChanges();
      });
  }

  exportHistoryAlarms() {
    const alarmIdSet = [];
    const options = this.eventsTable.getSelection();
    options.forEach(val => {
      alarmIdSet.push(val.entityId);
    });
    this.alarmApiService
      .exportEventsUsingPOST({
        entityIds: { entityIdSet: alarmIdSet },
        lang: this.i18n.language,
        akOperationTips: false
      })
      .subscribe(blob => {
        const bf = new Blob([blob], {
          type: 'application/vnd.ms.excel'
        });
        this.appUtilsService.downloadFile(
          this.activeNodeName
            ? `HistoryAlarms_${now()}_Node_${this.activeNodeName}.xls`
            : `HistoryAlarms_${now()}.xls`,
          bf
        );
        this.cdr.detectChanges();
      });
  }

  // 防勒索告警误处理
  errorHandle(alarm, modal) {
    if (
      !isArray(alarm.params) ||
      isEmpty(alarm.params[0]) ||
      isEmpty(alarm.params[1])
    ) {
      modal.close();
    }
    this.warningMessageService.create({
      content: this.i18n.get('explore_alarm_error_handle_label'),
      onOK: () => {
        this.antiRansomwarePolicyApiService
          .HandleRealtimeAlarm({
            vStoreName: alarm.params[0],
            filesystem: alarm.params[1],
            alarmEntityId: alarm.entity
          })
          .subscribe(() => {
            modal.close();
            this.refreshAlarms();
          });
      }
    });
  }

  openDetailModal(item) {
    const isAlarm = this.activeIndex === this.alarmEventType.ALARM;
    let request;
    if (isAlarm) {
      const params = {
        sequence: item.sequence,
        pageSize: 5,
        pageNo: 0,
        language: this.i18n.language === 'zh-cn' ? 'ZH' : 'EN'
      };
      const cluster = JSON.parse(
        decodeURIComponent(this.cookieService.get('currentCluster'))
      ) || {
        clusterId: DataMap.Cluster_Type.local.value,
        clusterType: DataMap.Cluster_Type.local.value
      };
      set(params, 'memberEsn', this.activeNode);

      request = this.isV1AlarmQuery
        ? this.alarmApiService.getAlarmListUsingGET({
            pageSize: 5,
            pageNum: 0,
            entityIdList: [item.entityId],
            language: this.i18n.language === 'zh-cn' ? 'zh' : 'en'
          })
        : this.alarmApiService.findPageUsingGET(params as any);
    } else {
      request = this.alarmApiService.getAlarmUsingPOST({
        akOperationTips: false,
        perAlarmQueryParam: {
          entityId: item.entityId,
          language: this.i18n.language
        },
        memberEsn: this.activeNode || ''
      });
    }
    request.subscribe(data => {
      if (isAlarm && isNil(data.records)) {
        return;
      }
      if (!isAlarm && !isEmpty(data.params)) {
        data.params = this.changeVmwareType(data.params);
      }
      this.drawModalService.openDetailModal(
        assign({}, MODAL_COMMON.generateDrawerOptions(), {
          lvModalKey: 'alarms-detail-modal',
          lvWidth:
            this.i18n.language === LANGUAGE.CN
              ? MODAL_COMMON.normalWidth
              : MODAL_COMMON.normalWidth + 50,
          lvHeader: !item.type ? this.alarmDetail : this.eventDetail,
          lvContent: AlarmsDetailsComponent,
          lvComponentParams: {
            data: this.setRowDeviceName(isAlarm ? first(data?.records) : data),
            isAlarm
          },
          lvFooter:
            this.isHyperDetect && isAlarm && item.alarmId === '0x5F025D0004'
              ? [
                  {
                    label: this.i18n.get('explore_error_feedbac_label'),
                    onClick: modal =>
                      this.errorHandle(first(data?.records), modal)
                  },
                  {
                    label: this.i18n.get('common_close_label'),
                    onClick: modal => modal.close()
                  }
                ]
              : [
                  {
                    label: this.i18n.get('common_close_label'),
                    onClick: modal => modal.close()
                  }
                ]
        })
      );
    });
  }

  getEventTypeOptions() {
    const options = [];
    map(ALARM_EVENT_EVENT_TYPE, item => {
      if (isNaN(item)) {
        options.push({
          label: this.i18n.get(
            `common_alarms_type_${ALARM_EVENT_EVENT_TYPE[item]}_label`
          ),
          value: ALARM_EVENT_EVENT_TYPE[item],
          key: item
        });
      }
    });
    return options;
  }

  getEventConfirmStatusOptions() {
    const options = [];
    map(ALARM_EVENT_EVENT_STATUS, item => {
      if (isNaN(item)) {
        options.push({
          label: this.i18n.get(
            `common_alarms_status_${ALARM_EVENT_EVENT_STATUS[item]}_label`
          ),
          value: ALARM_EVENT_EVENT_STATUS[item],
          key: item
        });
      }
    });
    return options;
  }

  handleTagsclearAll() {
    this._clearIdTag()
      ._clearObjTypeTag()
      ._clearSeverityTag()
      ._clearTimeTag();
  }
  handleTagRemove(e) {
    const { key } = e;
    if (key === 'severity') {
      this._clearSeverityTag();
    }
    if (key === 'objType') {
      this._clearObjTypeTag();
    }
    if (key === 'alarmTimeStr') {
      this._clearTimeTag();
    }
    if (key === 'alarmId') {
      this._clearIdTag();
    }
  }
  initFilterMap(key) {
    const _cols = cloneDeep(this.alarmCols);
    _cols
      .find(item => item.key === key)
      ?.filterMap.forEach(item => (item.selected = false));
    this.alarmCols = _cols;
  }

  private _clearSeverityTag() {
    this.alarmFilterChange({ key: 'severity', value: [] });
    this.initFilterMap('severity');
    return this;
  }
  private _clearObjTypeTag() {
    this.alarmObjectFilterChange({ key: 'objType', value: [] });
    this.initFilterMap('objType');
    return this;
  }

  private _clearTimeTag() {
    this.rangeDate = null;
    this.selectDate([null, null]);
    return this;
  }

  private _clearIdTag() {
    this.queryAlarmId = '';
    this.searchByAlarmId('');
    return this;
  }

  private _updateFiltersTag(key: string, value: any) {
    const _tags = cloneDeep(this.apiFilters);
    const tagItem = _tags.find(item => item.key === key);
    const genLabel = (key: string, value: any): string | void => {
      const name = this.alarmCols.find(item => item.key === key).label;
      const tagValue = this._genTagValue(key, value);
      return isEmpty(tagValue) ? null : `${name}:${tagValue}`;
    };
    const label = genLabel(key, value);
    if (tagItem) {
      tagItem.label = label;
    } else {
      _tags.push({
        removeable: true,
        label,
        key
      });
    }
    this.apiFilters = _tags.filter(item => item.label);
  }

  private _genTagValue(key: string, value: any) {
    let result: string = toString(value);
    if (isEmpty(value)) return null;
    if (key === 'severity') {
      result = this.i18n.get(`common_alarms_${toLower(value)}_label`);
    }
    if (key === 'objType') {
      const typeValue = first(value);
      result = this.objTypes.find(item => item.value === typeValue).label;
    }
    if (key === 'alarmTimeStr') {
      if (first(value) == null) return null;
      result = value
        .map(item =>
          this.getAlarmTimeStr(
            this.appUtilsService.toSystemTimeLong(item),
            false
          )
        )
        .join(' ~ ');
    }
    return result;
  }

  alarmFilterChange = e => {
    !this.isV1AlarmQuery && this._updateFiltersTag(e.key, e.value);
    this.alarmSeverity = e.value;
    this.alarmStartPage = CommonConsts.PAGE_START;
    this.refreshAlarms();
  };

  updateFilter() {
    const filterMap = this.alarmFiltersMap;
    if (
      !filterMap.AlarmSeverityOptions ||
      !filterMap.AlarmSeverityOptions.length
    ) {
      return;
    }
    this.alarmSeverity = [];
    (filterMap.AlarmSeverityOptions as any).map(item => {
      item.selected = false;
      return item;
    });
  }

  clearEventTag(e) {
    if (e.key === 'alarmId') {
      this.queryAlarmId = '';
    } else if (e.key === 'alarmTimeStr') {
      this._clearTimeTag();
    } else {
      this.eventColFilterMap[e.key] = [];
      let tmpCol = find(this.eventCols, { key: e.key });
      each(tmpCol.filterMap, item => {
        item.selected = false;
      });
      tmpCol.filterMap = [...tmpCol.filterMap];
    }
    this.eventFilters = this.eventFilters.filter(item => item.key !== e.key);
    this.eventFilters = [...this.eventFilters];
    this.eventStartPage = CommonConsts.PAGE_START;
    this.refreshEvent();
  }

  clearAllEventTag() {
    this.eventColFilterMap = {};
    each(this.eventCols, item => {
      if (!!item?.filterMap) {
        each(item?.filterMap, val => {
          val.selected = false;
        });
        item.filterMap = [...item.filterMap];
      }
    });
    this.eventFilters = [];
    this.queryAlarmId = '';
    this._clearTimeTag();
    this.refreshEvent();
  }

  updateEventFilter() {
    const filterMap = this.eventFiltersMap;
    if (
      !filterMap.EventSeverityOptions ||
      !filterMap.EventSeverityOptions.length
    ) {
      return;
    }
    this.alarmSeverity = [];
    !this.isV1AlarmQuery &&
      (filterMap.EventSeverityOptions as any).map(item => {
        item.selected = false;
        this.alarmSeverity.push(item.value);
        return item;
      });
  }

  eventFilterChange = e => {
    let hasKey = false;
    each(this.eventFilters, item => {
      if (item.key === e.key) {
        assign(item, e);
        hasKey = true;
      }
    });
    if (!hasKey) {
      this.eventFilters.push(e);
    }
    this.eventFilters = [...this.eventFilters];
    this.eventColFilterMap[e.key] = e.value;
    this.eventStartPage = CommonConsts.PAGE_START;
    this.refreshEvent();
  };

  eventTypeFilterChange = e => {
    this.eventType = e.value;
    this.eventStartPage = CommonConsts.PAGE_START;
    this.refreshEvent();
  };

  toggleAlarmSelect(source) {
    source.isShow = !source.isShow;
    this.columns = [...this.columns];
    const columnsStatus = {};
    each(this.columns, col => {
      assign(columnsStatus, {
        [col.key]: col.isShow
      });
    });
    if (this.activeIndex === this.alarmEventType.ALARM) {
      this.alarmCols = this.columns;
      this.rememberColumnsService.setColumnsStatus(
        'alarm_table',
        columnsStatus
      );
    }
    if (this.activeIndex === this.alarmEventType.EVENT) {
      this.eventCols = this.columns;
      this.rememberColumnsService.setColumnsStatus(
        'event_table',
        columnsStatus
      );
    }
  }

  selectionAlarmChange(source) {
    this.exportAlarmTipLabel = this.i18n.get('common_export_all_label', [
      this.alarm
    ]);
  }

  selectionEventChange(source) {
    const array = this.eventsTable.getSelection();
    if (!!array.length) {
      this.exportEventTipLabel = this.i18n.get('common_export_selected_label', [
        this.event
      ]);
    } else {
      this.exportEventTipLabel = this.i18n.get('common_export_all_label', [
        this.event
      ]);
    }
  }

  trackById = (_, item) => {
    return item.sequence;
  };
}
