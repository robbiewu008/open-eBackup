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
  AfterViewInit,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  GROUP_COMMON,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  SSOConfigService,
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
  each,
  isUndefined,
  set,
  size,
  trim,
  filter as _filter,
  first,
  cloneDeep,
  values,
  reject,
  includes,
  isEmpty
} from 'lodash';
import { SamlSsoDetailComponent } from './saml-sso-detail/saml-sso-detail.component';
import { CreateSamlSsoComponent } from './create-saml-sso/create-saml-sso.component';
import { combineLatest } from 'rxjs';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { map } from 'rxjs/operators';
import { Router } from '@angular/router';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
@Component({
  selector: 'aui-saml-sso-config',
  templateUrl: './saml-sso-config.component.html',
  styleUrls: ['./saml-sso-config.component.less']
})
export class SamlSsoConfigComponent implements OnInit, AfterViewInit {
  readonly INIT_SCROLL_HIGHT = 200;
  name;
  tableData: TableData;
  tableConfig: TableConfig;
  optsConfig;
  optsItems = [];
  selectionData = [];

  groupCommon = GROUP_COMMON;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('statusTpl', { static: true })
  statusTpl: TemplateRef<any>;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private ssoConfigService: SSOConfigService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    public virtualScroll: VirtualScrollService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.virtualScroll.getScrollParam(this.INIT_SCROLL_HIGHT);
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  onChange() {
    this.ngOnInit();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      create: {
        id: 'create',
        type: 'primary',
        permission: OperateItems.DeleteKerberos,
        label: this.i18n.get('common_create_label'),
        onClick: data => {
          this.create();
        }
      },
      exportMetaData: {
        id: 'exportMetaData',
        permission: OperateItems.DeleteKerberos,
        label: this.i18n.get('system_export_metadata_label'),
        onClick: data => {
          this.exportMetadata();
        }
      },
      active: {
        id: 'active',
        permission: OperateItems.DeleteKerberos,
        label: this.i18n.get('common_active_label'),
        disableCheck: data => {
          return (
            !size(data) ||
            size(
              _filter(data, val => {
                return val.status === DataMap.ssoStatus.deactivated.value;
              })
            ) !== size(data)
          );
        },
        onClick: data => {
          this.active(first(data));
        }
      },
      deactive: {
        id: 'deactive',
        divide: true,
        permission: OperateItems.DeleteKerberos,
        disableCheck: data => {
          return (
            !size(data) ||
            size(
              _filter(data, val => {
                return val.status === DataMap.ssoStatus.activated.value;
              })
            ) !== size(data)
          );
        },
        label: this.i18n.get('common_disable_label'),
        onClick: data => {
          this.deactive(first(data));
        }
      },
      modify: {
        id: 'modify',
        permission: OperateItems.DeleteKerberos,
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.create(data);
        }
      },
      delete: {
        id: 'delete',
        permission: OperateItems.DeleteKerberos,
        disableCheck: data => {
          return (
            !size(data) ||
            size(data) > 20 ||
            size(
              _filter(data, val => {
                return val.status === DataMap.ssoStatus.deactivated.value;
              })
            ) !== size(data)
          );
        },
        label: this.i18n.get('common_delete_label'),
        onClick: data => {
          this.deleteRes(data);
        }
      }
    };

    this.optsItems = cloneDeep(
      getPermissionMenuItem(
        values(
          reject(opts, opt => {
            return !includes(
              ['active', 'deactive', 'delete', 'modify'],
              opt.id
            );
          })
        )
      )
    );

    const cols: TableCols[] = [
      {
        key: 'configName',
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
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('ssoStatus')
        },
        cellRender: this.statusTpl
      },
      {
        key: 'protocol',
        name: this.i18n.get('common_protocol_label')
      },
      {
        key: 'description',
        name: this.i18n.get('common_desc_label')
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.optsItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'uuid',
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
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };

    this.optsConfig = cloneDeep(
      getPermissionMenuItem(
        values(
          reject(opts, opt => {
            return !includes(['create', 'exportMetaData', 'delete'], opt.id);
          })
        )
      )
    );
  }

  getData(filters: Filters, args) {
    const params = {
      startIndex: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    const filterParams = {};
    each(filters.filters, filter => {
      if (filter.value && size(filter.value)) {
        set(
          filterParams,
          filter.key,
          filter.key === 'configName' ? first(filter.value) : filter.value
        );
      }
    });
    set(params, 'filter', JSON.stringify(filterParams));

    this.ssoConfigService
      .GetAllConfig(params as any)
      .pipe(
        map(res => {
          each(res.records, item => {
            assign(item, {
              name: item.configName
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }

  search() {
    assign(this.dataTable.filterMap, {
      filters: [
        {
          filterMode: 'contains',
          caseSensitive: false,
          key: 'configName',
          value: [trim(this.name)]
        }
      ]
    });

    set(
      this.dataTable.filterMap,
      'paginator.pageIndex',
      CommonConsts.PAGE_START
    );
    this.dataTable.fetchData();
  }

  getDetail(data) {
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: data?.configName,
        lvContent: SamlSsoDetailComponent,
        lvModalKey: 'ssoDetailModalKey',
        lvWidth: MODAL_COMMON.normalWidth,
        lvModality: false,
        lvComponentParams: {
          rowData: data
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

  create(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'create-report',
        lvWidth: MODAL_COMMON.smallModal,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_create_label')
          : this.i18n.get('common_modify_label'),
        lvContent: CreateSamlSsoComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowData: first(data)
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateSamlSsoComponent;
          const modalIns = modal.getInstance();
          const combined = combineLatest([
            content.formGroup.statusChanges,
            content.validMetadata$
          ]);

          if (!content.rowData) {
            combined.subscribe(latestValues => {
              const [formGroupStatus, validMetadata] = latestValues;

              modalIns.lvOkDisabled = !(
                formGroupStatus === 'VALID' && validMetadata
              );
            });
          } else {
            content.formGroup.statusChanges.subscribe(res => {
              modalIns.lvOkDisabled = res !== 'VALID';
            });
          }
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateSamlSsoComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);

                this.dataTable.fetchData();
              },
              error: () => resolve(false)
            });
          });
        },
        lvCancel: modal => {}
      })
    );
  }

  exportMetadata() {
    this.ssoConfigService.DownloadMetadata({}).subscribe(blob => {
      const bf = new Blob([blob], {
        type: 'application/octet-stream'
      });
      this.appUtilsService.downloadFile(`metadata.xml`, bf);
    });
  }

  active(data) {
    this.ssoConfigService
      .ActiveSsoConfig({
        uuid: data.uuid,
        configName: data.configName
      })
      .subscribe(res => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      });
  }

  deactive(data) {
    this.ssoConfigService
      .DeActiveSsoConfig({
        uuid: data.uuid,
        configName: data.configName
      })
      .subscribe(res => {
        this.selectionData = [];
        this.dataTable.setSelections([]);
        this.dataTable.fetchData();
      });
  }

  deleteRes(data) {
    if (size(data) === 1) {
      this.warningMessageService.create({
        content: this.i18n.get('system_saml_sso_delete_label'),
        onOK: () => {
          this.ssoConfigService
            .DeleteSsoConfig({
              uuid: data[0].uuid,
              configName: data[0].configName
            })
            .subscribe(res => {
              this.selectionData = reject(
                this.dataTable.getAllSelections(),
                item => {
                  return item.uuid === data[0].uuid;
                }
              );
              this.dataTable.setSelections(this.selectionData);
              this.dataTable.fetchData();
            });
        }
      });
    } else {
      this.warningMessageService.create({
        content: this.i18n.get('system_saml_sso_delete_label'),
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item => {
              return this.ssoConfigService.DeleteSsoConfig({
                uuid: item.uuid,
                configName: item.configName,
                akDoException: false,
                akOperationTips: false,
                akLoading: false
              });
            },
            cloneDeep(this.selectionData),
            () => {
              this.selectionData = [];
              this.dataTable.setSelections([]);
              this.dataTable.fetchData();
            }
          );
        }
      });
    }
  }
}
