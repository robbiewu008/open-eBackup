import {
  AfterViewInit,
  Component,
  EventEmitter,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageboxService } from '@iux/live';
import {
  AntiRansomwareAirgapApiService,
  ApiService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  getTableOptsItems,
  GROUP_COMMON,
  I18NService,
  MODAL_COMMON,
  OperateItems,
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
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  drop,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isUndefined,
  map,
  trim
} from 'lodash';
import { Subject } from 'rxjs';
import { DeviceDetailComponent } from './device-detail/device-detail.component';
import { GetTacticsComponent } from './get-tactics/get-tactics.component';

@Component({
  selector: 'aui-storage-device',
  templateUrl: './storage-device.component.html',
  styleUrls: ['./storage-device.component.less']
})
export class StorageDeviceComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  options = [];
  frequency = [];
  warningConfirm: boolean = false;
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isDataBackup = this.appUtilsService.isDataBackup;
  policyStatus = 'airgapPolicyStatus';
  valid$ = new Subject<boolean>();
  removeDangerTipLable = this.i18n.get(
    'common_remove_association_offline_tip_label'
  );
  removeInfoContentTpl = this.i18n.get(
    'common_remove_association_online_tip_label'
  );
  turnOnInfoContentTpl = this.i18n.get('common_turn_on_association_tip_label');
  turnOffInfoContentTpl = this.i18n.get(
    'common_turn_off_association_tip_label'
  );

  groupCommon = GROUP_COMMON;

  @Output() refreshDevice = new EventEmitter();
  optsConfig;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('warningContentTpl', { static: true })
  warningContentTpl: TemplateRef<any>;
  @ViewChild('removeContentTpl', { static: true })
  removeContentTpl: TemplateRef<any>;
  @ViewChild('turnOnContentTpl', { static: true })
  turnOnContentTpl: TemplateRef<any>;
  @ViewChild('turnOffContentTpl', { static: true })
  turnOffContentTpl: TemplateRef<any>;
  @ViewChild('timeTpl', { static: true })
  timeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private messageBox: MessageboxService,
    private apiService: ApiService,
    private appUtilsService: AppUtilsService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private antiRansomwareAirgapApiService: AntiRansomwareAirgapApiService
  ) {}

  ngOnInit(): void {
    this.policyStatus = this.isCyberengine
      ? 'airgapPolicyCyberStatus'
      : 'airgapPolicyStatus';
    this.initConfig();
  }

  ngAfterViewInit() {
    this.initSearchFilter();
    this.dataTable.fetchData();
  }

  initSearchFilter() {
    const nameFilter = this.appUtilsService.getCacheValue(
      'airgapFilter',
      false
    );
    if (nameFilter) {
      assign(this.dataTable.filterMap, {
        filters: [
          {
            filterMode: 'contains',
            caseSensitive: false,
            key: 'name',
            value: nameFilter
          }
        ]
      });
    }
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'associated_policy',
        label: this.i18n.get('explore_associate_policy_label'),
        permission: OperateItems.ModifyStorageDevice,
        disableCheck: data => {
          return (
            !data.length ||
            !!data[0].airGapPolicyInfo ||
            data[0].linkStatus === DataMap.airgapDeviceStatus.offline.value
          );
        },
        onClick: data => {
          this.getPolicy(data, false);
        }
      },
      {
        id: 'modify',
        permission: OperateItems.ModifyStorageDevice,
        disableCheck: data => {
          return (
            !data.length ||
            !data[0].airGapPolicyInfo ||
            data[0].policyStatus === DataMap.airgapPolicyStatus.invalid.value ||
            data[0].linkStatus === DataMap.airgapDeviceStatus.offline.value
          );
        },
        label: this.i18n.get('common_modify_association_label'),
        onClick: data => {
          this.getPolicy(data, true);
        }
      },
      {
        id: 'remove',
        permission: OperateItems.ModifyStorageDevice,
        divide: true,
        label: this.i18n.get('common_remove_association_label'),
        disableCheck: data => {
          return !data.length || !data[0].airGapPolicyInfo;
        },
        onClick: data => {
          this.removeTactics(data);
        }
      },
      {
        id: 'turn_on',
        permission: OperateItems.ModifyStorageDevice,
        disableCheck: data => {
          return (
            !data.length ||
            data[0].policyStatus === DataMap.airgapPolicyStatus.enable.value ||
            !data[0].airGapPolicyInfo ||
            data[0].linkStatus === DataMap.airgapDeviceStatus.offline.value
          );
        },
        label: this.i18n.get('common_turn_on_association_label'),
        onClick: data => {
          this.turnOnTactics(data);
        }
      },
      {
        id: 'turn_off',
        permission: OperateItems.ModifyStorageDevice,
        divide: this.isCyberengine || this.isDataBackup,
        disableCheck: data => {
          return (
            !data.length ||
            data[0].policyStatus === DataMap.airgapPolicyStatus.disable.value ||
            data[0].policyStatus ===
              DataMap.airgapPolicyCyberStatus.invalid.value ||
            !data[0].airGapPolicyInfo ||
            data[0].linkStatus === DataMap.airgapDeviceStatus.offline.value
          );
        },
        label: this.i18n.get('common_turn_off_association_label'),
        onClick: data => {
          this.turnOffTactics(data);
        }
      },
      {
        id: 'disconnect',
        permission: OperateItems.ModifyStorageDevice,
        label: this.i18n.get('explore_isconnect_linked_detection_label'),
        displayCheck: () => {
          return this.isCyberengine || this.isDataBackup;
        },
        disableCheck: data => {
          return (
            !data.length ||
            data[0].replicationLinkStatus !==
              DataMap.airgapLinkStatus.open.value
          );
        },
        onClick: data => {
          this.disconnectLink(data);
        }
      }
    ];

    this.options = opts;

    this.optsConfig = getPermissionMenuItem(
      filter(opts, item =>
        includes(['remove', 'turn_on', 'turn_off', 'disconnect'], item.id)
      )
    );

    const cols: TableCols[] = [
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
        key: 'esn',
        name: this.i18n.get('ESN'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_device_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('airgapDeviceStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('airgapDeviceStatus')
        }
      },
      {
        key: 'replicationLinkStatus',
        name: this.i18n.get('common_replication_link_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('airgapLinkStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('airgapLinkStatus')
        }
      },
      {
        key: 'associatedPolicy',
        name: this.i18n.get('common_associated_airgap_policy_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.timeTpl
      },
      {
        key: 'policyStatus',
        name: this.i18n.get('common_airgap_policy_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray(this.policyStatus)
            .filter(item => {
              if (!this.isCyberengine && !this.isDataBackup) {
                return !includes(
                  [DataMap.airgapPolicyStatus.invalid.value],
                  item.value
                );
              }
              return true;
            })
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray(this.policyStatus)
        }
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: getPermissionMenuItem(this.options)
          }
        }
      }
    ];
    this.tableConfig = {
      table: {
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        scrollFixed: true,
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      }
    };
  }

  getData(filters: Filters, args: { isAutoPolling: any }) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };
    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.linkStatus) {
        assign(params, {
          linkStatus: drop(conditionsTemp.linkStatus)
        });
      }
      if (conditionsTemp.replicationLinkStatus) {
        assign(params, {
          replicationLinkStatus: drop(conditionsTemp.replicationLinkStatus)
        });
      }
      if (conditionsTemp.esn) {
        assign(params, {
          esn: conditionsTemp.esn[1]
        });
      }
      if (conditionsTemp.policyStatus) {
        assign(params, {
          policyStatus: drop(conditionsTemp.policyStatus)
        });
      }
      if (conditionsTemp.name) {
        assign(params, {
          name: conditionsTemp.name[1]
        });
      }
      if (conditionsTemp.associatedPolicy) {
        assign(params, {
          policyName: conditionsTemp.associatedPolicy[1]
        });
      }
    }

    this.antiRansomwareAirgapApiService
      .ShowPageDevices(params)
      .subscribe((res: any) => {
        each(res.records, item => {
          if (!item.airGapPolicyInfo) {
            assign(item, {
              frequency: '--'
            });
          } else if (
            item.airGapPolicyInfo.triggerCycle === 'day' ||
            item.airGapPolicyInfo.triggerWeekFreq.split(',').length === 7
          ) {
            assign(item, {
              frequency: this.i18n.get('common_everyday_label')
            });
          } else {
            const time = map(
              item.airGapPolicyInfo.triggerWeekFreq.split(','),
              v => {
                return this.dataMapService.getLabel('Days_Of_Week', v);
              }
            ).join(', ');

            const tims = `${this.i18n.get('common_every_weeks_label')} ${time}`;
            assign(item, {
              frequency: tims
            });
          }
        });

        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.refreshDevice.emit();
        const nameFilter = this.appUtilsService.getCacheValue('airgapFilter');
        if (nameFilter && find(res.records, item => item.name === nameFilter)) {
          this.getDetail(find(res.records, item => item.name === nameFilter));
        }
      });
  }

  search(value) {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'name',
          value: trim(value)
        }
      ]
    });
    this.dataTable.fetchData();
  }

  refresh() {
    this.dataTable.fetchData();
  }

  modalCheckBoxChange(e) {
    this.valid$.next(e);
  }

  disconnectLink(data) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_disconnect_linked_detection_label'),
      onOK: () => {
        if (data.length > 1) {
          this.batchOperateService.selfGetResults(
            item => {
              return this.antiRansomwareAirgapApiService.DisconnectReplicationLink(
                {
                  deviceId: item.id,
                  akDoException: false,
                  akOperationTips: false,
                  akLoading: false
                }
              );
            },
            cloneDeep(data),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            },
            '',
            true
          );
        } else {
          this.antiRansomwareAirgapApiService
            .DisconnectReplicationLink({ deviceId: data[0].id })
            .subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      }
    });
  }

  removeTactics(data) {
    if (
      data.length === 1 &&
      find(
        data,
        item => item.linkStatus === DataMap.airgapDeviceStatus.offline.value
      )
    ) {
      this.removeDangerTipLable = this.i18n.get(
        'common_remove_association_offline_tip_label',
        [map(data, 'airGapPolicyInfo.name').join(this.i18n.isEn ? ',' : '，')]
      );
      this.messageBox.confirm({
        lvHeader: this.i18n.get('common_danger_label'),
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvContent: this.warningContentTpl,
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkType: 'primary',
        lvOkDisabled: !this.warningConfirm,
        lvCancelType: 'default',
        lvBeforeOpen: () => {
          this.warningConfirm = false;
        },
        lvAfterOpen: modal => {
          this.valid$.subscribe(e => {
            modal.lvOkDisabled = !e;
          });
        },
        lvOk: () => {
          this.antiRansomwareAirgapApiService
            .DeleteDevicePolicyRelation({
              deviceId: data[0].id,
              memberEsn: data[0]?.esn
            })
            .subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      });
    } else {
      this.removeInfoContentTpl = this.i18n.get(
        'common_remove_association_online_tip_label',
        [map(data, 'airGapPolicyInfo.name').join(this.i18n.isEn ? ',' : '，')]
      );
      this.messageBox.confirm({
        lvHeader: this.i18n.get('common_alarms_info_label'),
        lvDialogIcon: 'lv-icon-popup-info-48',
        lvContent: this.removeContentTpl,
        lvWidth: MODAL_COMMON.normalWidth - 50,
        lvOk: () => {
          if (data.length > 1) {
            this.batchOperateService.selfGetResults(
              item => {
                return this.antiRansomwareAirgapApiService.DeleteDevicePolicyRelation(
                  {
                    deviceId: item.id,
                    memberEsn: item?.esn,
                    akDoException: false,
                    akOperationTips: false,
                    akLoading: false
                  }
                );
              },
              cloneDeep(data),
              () => {
                this.selectionData = [];
                this.dataTable.setSelections([]);
                this.dataTable.fetchData();
              }
            );
          } else {
            this.antiRansomwareAirgapApiService
              .DeleteDevicePolicyRelation({
                deviceId: data[0].id,
                memberEsn: data[0]?.esn
              })
              .subscribe(() => {
                this.dataTable.fetchData();
                this.selectionData = [];
                this.dataTable.setSelections([]);
              });
          }
        }
      });
    }
  }

  turnOnTactics(data) {
    this.turnOnInfoContentTpl = this.i18n.get(
      'common_turn_on_association_tip_label',
      [map(data, 'airGapPolicyInfo.name').join(this.i18n.isEn ? ',' : '，')]
    );
    this.messageBox.confirm({
      lvHeader: this.i18n.get('common_alarms_info_label'),
      lvDialogIcon: 'lv-icon-popup-info-48',
      lvContent: this.turnOnContentTpl,
      lvWidth: MODAL_COMMON.normalWidth,
      lvOk: () => {
        if (data.length > 1) {
          this.batchOperateService.selfGetResults(
            item => {
              return this.antiRansomwareAirgapApiService.EnableRelatePolicy({
                deviceId: item.id,
                memberEsn: item?.esn,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            cloneDeep(data),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            },
            '',
            true
          );
        } else {
          this.antiRansomwareAirgapApiService
            .EnableRelatePolicy({
              deviceId: data[0].id,
              memberEsn: data[0]?.esn
            })
            .subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      }
    });
  }

  turnOffTactics(data) {
    this.turnOffInfoContentTpl = this.i18n.get(
      'common_turn_off_association_tip_label',
      [map(data, 'airGapPolicyInfo.name').join(this.i18n.isEn ? ',' : '，')]
    );
    this.messageBox.confirm({
      lvHeader: this.i18n.get('common_alarms_info_label'),
      lvDialogIcon: 'lv-icon-popup-info-48',
      lvContent: this.turnOffContentTpl,
      lvWidth: MODAL_COMMON.normalWidth,
      lvOk: () => {
        if (data.length > 1) {
          this.batchOperateService.selfGetResults(
            item => {
              return this.antiRansomwareAirgapApiService.DisableRelatePolicy({
                deviceId: item.id,
                memberEsn: item?.esn,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            cloneDeep(data),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            },
            '',
            true
          );
        } else {
          this.antiRansomwareAirgapApiService
            .DisableRelatePolicy({
              deviceId: data[0].id,
              memberEsn: data[0]?.esn
            })
            .subscribe(() => {
              this.dataTable.fetchData();
              this.selectionData = [];
              this.dataTable.setSelections([]);
            });
        }
      }
    });
  }

  getPolicy(data, isModify) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('explore_associate_policy_label'),
      lvModalKey: 'openGauss_cluster_detail',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: GetTacticsComponent,
      lvComponentParams: {
        data: data[0],
        isModify
      },
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as GetTacticsComponent;
        content.formGroup.statusChanges.subscribe(status => {
          modal.getInstance().lvOkDisabled = status !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as GetTacticsComponent;
          content.onOk().subscribe(
            res => {
              resolve(true);
              this.dataTable.fetchData();
            },
            err => {
              resolve(false);
            }
          );
        });
      }
    });
  }

  getDetail(data) {
    this.drawModalService.openDetailModal({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data.name,
      lvModalKey: 'openGauss_cluster_detail',
      lvWidth: this.i18n.isEn
        ? MODAL_COMMON.normalWidth + 200
        : MODAL_COMMON.normalWidth + 100,
      lvContent: DeviceDetailComponent,
      lvComponentParams: {
        data,
        optItems: getTableOptsItems(
          getPermissionMenuItem(cloneDeep(this.options)),
          data,
          this
        )
      },
      lvFooter: [
        {
          id: 'close',
          label: this.i18n.get('common_close_label'),
          onClick: modal => {
            modal.close();
          }
        }
      ]
    });
  }
}
