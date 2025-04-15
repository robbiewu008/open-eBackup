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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  RoleOperationMap
} from 'app/shared';
import { TableCols } from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, includes, isEmpty } from 'lodash';
import { BaseTemplateComponent } from '../../virtualization/kubernetes/base-template/base-template.component';
import { RegisterMongodbComponent } from './register-mongodb/register-mongodb.component';

@Component({
  selector: 'aui-mongodb',
  templateUrl: './mongodb.component.html',
  styleUrls: ['./mongodb.component.less']
})
export class MongodbComponent implements OnInit {
  subType = DataMap.Resource_Type.MongoDB.value;
  dataMap = DataMap;
  extraConfig;
  columns: TableCols[] = [];
  @ViewChild(BaseTemplateComponent, { static: false })
  BaseTemplateComponent: BaseTemplateComponent;
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService
  ) {}

  ngOnInit() {
    this.initTableConfig();
    this.initTableColumns();
  }

  private initTableConfig() {
    this.extraConfig = {
      register: {
        id: 'register',
        type: 'primary',
        permission: RoleOperationMap.manageResource,
        label: this.i18n.get('common_register_label'),
        onClick: () => this.register()
      },
      modify: {
        id: 'modify',
        permission: OperateItems.RegisterDatabase,
        label: this.i18n.get('common_modify_label'),
        onClick: ([data]) => this.register(data)
      }
    };
  }

  private initTableColumns() {
    this.columns = [
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
            click: data => {}
          }
        }
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      {
        key: 'version',
        name: this.i18n.get('common_version_label')
      },
      {
        key: 'clusterType',
        name: this.i18n.get('protection_instance_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('mongodbInstanceType')
            .filter(
              item =>
                !includes(
                  [DataMap.mongodbInstanceType.clusterPrimary.value],
                  item.value
                )
            )
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('mongodbInstanceType')
        }
      }
    ];
  }

  onChange() {
    if (this.BaseTemplateComponent) {
      this.BaseTemplateComponent.dataTable?.fetchData();
    }
  }

  register(rowItem?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-mongodb',
        lvWidth: this.i18n.isEn
          ? MODAL_COMMON.normalWidth + 150
          : MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(rowItem)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterMongodbComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowItem
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterMongodbComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.BaseTemplateComponent.dataTable?.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }
}
