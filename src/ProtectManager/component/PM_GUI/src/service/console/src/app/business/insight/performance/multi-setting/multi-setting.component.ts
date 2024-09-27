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
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageboxService } from '@iux/live';
import {
  DataMap,
  DataMapService,
  I18NService,
  PerformanceApiDescService,
  WarningMessageService
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { cloneDeep } from 'lodash';

@Component({
  selector: 'aui-multi-setting',
  templateUrl: './multi-setting.component.html',
  styleUrls: ['./multi-setting.component.less']
})
export class MultiSettingComponent implements OnInit {
  tableConfig: TableConfig;
  tableData: TableData;
  dataMap = DataMap;
  isDeleteHistory = false;
  loading = false;
  data;
  performanceSub;
  switchOffContent = this.i18n.get('insight_performance_switch_off_label');

  @ViewChild('statusTpl', { static: true }) statusTpl: TemplateRef<any>;
  @ViewChild('switchTpl', { static: true }) switchTpl: TemplateRef<any>;
  @ViewChild('tipContentTpl', { static: false }) tipContentTpl: TemplateRef<
    any
  >;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private messageBox: MessageboxService,
    private infoMessageService: InfoMessageService,
    public performanceApiService: PerformanceApiDescService,
    public dataMapService: DataMapService,
    public warningMessageService: WarningMessageService
  ) {}

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'nodeName',
        name: this.i18n.get('system_servers_label')
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        cellRender: this.statusTpl
      },
      {
        key: 'open',
        name: this.i18n.get('insight_performance_monitor_label'),
        cellRender: this.switchTpl
      }
    ];
    this.tableConfig = {
      pagination: {
        showPageSizeOptions: false
      },
      table: {
        compareWith: 'esn',
        async: false,
        columns: cols,
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        }
      }
    };

    this.tableData = {
      data: this.data || [],
      total: this.data?.length
    };
    this.cdr.detectChanges();
  }

  switchChange(data) {
    const val = cloneDeep(data.open);
    if (data.open) {
      this.isDeleteHistory = false;
      this.messageBox.info({
        lvHeader: this.i18n.get('insight_switch_off_label'),
        lvContent: this.tipContentTpl,
        lvWidth: 600,
        lvFooter: [
          {
            label: this.i18n.get('common_ok_label'),
            onClick: modal => {
              if (this.isDeleteHistory) {
                this.warningMessageService.create({
                  content: this.i18n.get(
                    'insight_performance_remove_history_label'
                  ),
                  onOK: () => {
                    this.performanceApiService
                      .enablePerformanceUsingPUT({
                        enable: false,
                        hasRemoveHistoryData: true,
                        memberEsn: data.esn
                      })
                      .subscribe(res => {
                        this.loading = false;
                        data.open = !val;
                        this.performanceSub.unsubscribe();
                      });
                  },
                  onCancel: () => {
                    this.loading = false;
                  },
                  lvAfterClose: result => {
                    if (result && result.trigger === 'close') {
                      this.loading = false;
                    }
                  }
                });
              } else {
                this.performanceApiService
                  .enablePerformanceUsingPUT({
                    enable: false,
                    hasRemoveHistoryData: false,
                    memberEsn: data.esn
                  })
                  .subscribe(res => {
                    this.loading = false;
                    data.open = !val;
                  });
              }
              modal.close();
            }
          },
          {
            type: 'primary',
            label: this.i18n.get('common_cancel_label'),
            onClick: modal => {
              this.loading = false;
              modal.close();
            }
          }
        ]
      });
    } else {
      this.infoMessageService.create({
        content: this.i18n.get('insight_enable_performance_label'),
        onOK: () => {
          this.performanceApiService
            .enablePerformanceUsingPUT({
              enable: true,
              hasRemoveHistoryData: false,
              memberEsn: data.esn
            })
            .subscribe(res => {
              this.loading = false;
              data.open = !val;
            });
        },
        onCancel: () => {
          this.loading = false;
        }
      });
    }
  }
}
