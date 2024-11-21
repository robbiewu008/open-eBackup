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
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  DataMapService,
  I18NService,
  MODAL_COMMON,
  WarningMessageService,
  WhiteListManagementService
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
  includes,
  isEmpty,
  isNil,
  map,
  size,
  uniqBy
} from 'lodash';
import { AssociateVstoreComponent } from '../blocking-rule-list/associate-vstore/associate-vstore.component';
import { AddWhitelistRuleComponent } from './add-whitelist-rule/add-whitelist-rule.component';

@Component({
  selector: 'aui-detection-whitelist',
  templateUrl: './detection-whitelist.component.html',
  styleUrls: ['./detection-whitelist.component.less']
})
export class DetectionWhitelistComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('numTpl', { static: true }) numTpl: TemplateRef<any>;
  @ViewChild('contentTpl', { static: true }) contentTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private whiteListManagementService: WhiteListManagementService
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
          return !size(data);
        },
        onClick: data => {
          this.delete(data);
        }
      }
    ];
    this.optsConfig = opts;

    const cols: TableCols[] = [
      {
        key: 'content',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.contentTpl
      },
      {
        key: 'type',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Detection_Whitelist_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Detection_Whitelist_Type')
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
                ['associate-vstore', 'disassociate-vstore', 'modify', 'delete'],
                item.id
              );
            })
          }
        }
      }
    ];

    this.tableConfig = {
      filterTags: true,
      table: {
        compareWith: 'id',
        columns: cols,
        scrollFixed: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scroll: {
          ...this.virtualScroll.scrollParam,
          y: '65vh'
        },
        colDisplayControl: false,
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
    if (!isNil(filters.sort)) {
      const order = filters?.sort.direction;
      if (!!order) {
        assign(params, { order });
      }
    }
    if (!isEmpty(filters.filters)) {
      const content = filters?.filters.find(i => i.key === 'content')?.value;
      const type = filters?.filters.find(i => i.key === 'type')?.value;
      if (!!content) {
        assign(params, { content: content[0] });
      }
      if (!!type) {
        assign(params, { type });
      }
    }
    this.whiteListManagementService
      .getWhiteListInfoUsingGET(params)
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
          data: res.records.map(i => {
            return {
              ...i,
              content: i?.content.replace(new RegExp(/( )/g), '&nbsp;')
            };
          }),
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }
  delete(datas) {
    const deleteWhiteListRequest = {
      ids: map(datas, 'id')
    };
    this.warningMessageService.create({
      content: this.i18n.get('explore_delete_blocking_file_extension_label', [
        map(datas, 'content').toString()
      ]),
      onOK: () => {
        this.whiteListManagementService
          .deleteWhiteListUsingDELETE({
            deleteWhiteListRequest
          })
          .subscribe(res => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
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
        assciateVstores: uniqBy(assciateVstores, 'vstoreName'),
        ids: map(data, 'id'),
        isWhiteList: true
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

  addRule(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_create_label'),
      lvModalKey: 'add_extension',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: AddWhitelistRuleComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddWhitelistRuleComponent;
        content.formGroup.statusChanges.subscribe(res => {
          modal.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as AddWhitelistRuleComponent;
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
}
