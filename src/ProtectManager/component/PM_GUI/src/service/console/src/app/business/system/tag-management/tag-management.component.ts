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
  ChangeDetectorRef,
  Component,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  I18NService,
  LabelApiService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService,
  getPermissionMenuItem,
  RoleType,
  SYSTEM_TIME
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
import {
  filter as _filter,
  assign,
  cloneDeep,
  isEmpty,
  isUndefined,
  map,
  size,
  remove
} from 'lodash';
import { CreateTagComponent } from './create-tag/create-tag.component';

@Component({
  selector: 'aui-tag-management',
  templateUrl: './tag-management.component.html',
  styleUrls: ['./tag-management.component.less'],
  providers: [DatePipe]
})
export class TagManagementComponent {
  optsConfig;
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  optItems = [];
  visible;
  timeZone = SYSTEM_TIME.timeZone;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('createdTimeTpl', { static: true })
  createdTimeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private warningMessageService: WarningMessageService,
    private drawModalService: DrawModalService,
    private labelApiService: LabelApiService,
    private cdr: ChangeDetectorRef,
    public cookieService: CookieService
  ) {}

  ngOnInit(): void {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  refresh() {
    this.dataTable.fetchData();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'create',
        type: 'primary',
        permission: OperateItems.EditTag,
        label: this.i18n.get('common_create_label'),
        displayCheck: data => {
          // 用于预置标签
          return this.cookieService.role !== RoleType.Auditor;
        },
        onClick: data => this.createTag()
      },
      {
        id: 'delete',
        permission: OperateItems.DeleteTag,
        label: this.i18n.get('common_delete_label'),
        disableCheck: data => {
          // 用于预置标签
          return (
            size(
              _filter(data, item => {
                return item.isBuilt;
              })
            ) !== size(data) || !size(data)
          );
        },
        displayCheck: data => {
          return this.cookieService.role !== RoleType.Auditor;
        },
        onClick: data => this.deleteTag(this.selectionData)
      }
    ];
    this.optsConfig = opts;
    this.optItems = cloneDeep(getPermissionMenuItem(opts));

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'relatedResourceNum',
        name: this.i18n.get('common_associated_resource_label')
      },
      {
        key: 'builderName',
        name: this.i18n.get('common_create_user_label')
      },
      {
        key: 'createdTime',
        name: this.i18n.get('common_create_time_label'),
        cellRender: this.createdTimeTpl
      },
      {
        key: 'operation',
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 3,
            items: [
              {
                id: 'modify',
                permission: OperateItems.EditTag,
                label: this.i18n.get('common_modify_label'),
                displayCheck: ([data]) => {
                  // 用于预置标签
                  return data.isBuilt;
                },
                onClick: ([data]) => this.createTag(data)
              },
              {
                id: 'delete',
                permission: OperateItems.DeleteTag,
                label: this.i18n.get('common_delete_label'),
                displayCheck: ([data]) => {
                  // 用于预置标签
                  return data.isBuilt;
                },
                onClick: data => this.deleteTag(data)
              }
            ]
          }
        }
      }
    ];
    if (this.cookieService.role === RoleType.Auditor) {
      remove(cols, { key: 'operation' });
    }
    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'uuid',
        columns:
          this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE
            ? cols.filter(v => v.key !== 'builderName')
            : cols,
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
  }

  getData(filters: Filters, args) {
    const params = {
      startPage: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditionsTemp = JSON.parse(filters.conditions);
      if (conditionsTemp.name) {
        assign(params, {
          name: conditionsTemp.name
        });
      }
    }

    this.labelApiService.queryLabelUsingGET(params).subscribe(res => {
      const newArr = res?.records.map(item => {
        return {
          ...item,
          disabled: !item.isBuilt
        };
      });
      this.tableData = {
        total: res?.totalCount,
        data: newArr
      };
      this.cdr.detectChanges();
    });
  }

  createTag(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: data
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('common_create_label'),
        lvContent: CreateTagComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as CreateTagComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(status => {
            modal.getInstance().lvOkDisabled = status !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvComponentParams: {
          data
        },
        lvOk: modal => {
          this.dealCreateTag(modal);
        }
      }
    });
  }

  dealCreateTag(modal) {
    return new Promise(resolve => {
      const content = modal.getContentComponent() as CreateTagComponent;
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

  deleteTag(data) {
    let nameArr = [map(data, 'name').join(',')];
    const list = map(data, 'uuid');
    this.warningMessageService.create({
      header: this.i18n.get('system_delete_tags_header_label'),
      content: this.i18n.get('system_delete_tags_tip_label', nameArr),
      onOK: () => {
        this.labelApiService
          .deleteLabelUsingDELETE({
            DeleteLabelRequest: {
              uuidList: list
            }
          })
          .subscribe(res => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          });
      }
    });
  }
}
