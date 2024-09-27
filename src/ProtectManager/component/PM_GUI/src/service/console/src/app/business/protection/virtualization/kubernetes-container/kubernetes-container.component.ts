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
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  DataMap,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  RoleOperationMap
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, isEmpty } from 'lodash';
import { BaseTemplateComponent } from '../kubernetes/base-template/base-template.component';
import { RegisterDatasetComponent } from './register-dataset/register-dataset.component';

@Component({
  selector: 'aui-kubernetes-container',
  templateUrl: './kubernetes-container.component.html',
  styleUrls: ['./kubernetes-container.component.less']
})
export class KubernetesContainerComponent implements OnInit {
  activeIndex = 'cluster';
  clusterSubType = DataMap.Resource_Type.kubernetesClusterCommon.value;
  namespaceSubType = DataMap.Resource_Type.kubernetesNamespaceCommon.value;
  datasetSubType = DataMap.Resource_Type.kubernetesDatasetCommon.value;
  namespaceExtraConfig = {};
  datasetExtraConfig = {
    register: {
      id: 'register',
      type: 'primary',
      permission: RoleOperationMap.manageResource,
      label: this.i18n.get('common_register_label'),
      onClick: () => this.registerDataset()
    },
    modify: {
      id: 'modify',
      permission: OperateItems.RegisterDatabase,
      label: this.i18n.get('common_modify_label'),
      onClick: ([data]) => this.registerDataset(data)
    }
  };
  namespaceColumns = [
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
      key: 'cluster',
      name: this.i18n.get('insight_report_belong_cluster_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'endpoint',
      name: this.i18n.get('protection_cluster_ip_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    }
  ];
  datasetColumns = [
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
      key: 'parentName',
      name: this.i18n.get('protection_belong_namespace_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'cluster',
      name: this.i18n.get('insight_report_belong_cluster_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    }
  ];

  @ViewChild(BaseTemplateComponent, { static: false })
  BaseTemplateComponent: BaseTemplateComponent;

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService
  ) {}

  onChange() {
    this.BaseTemplateComponent.ngOnInit();
  }

  ngOnInit() {}

  registerDataset(rowItem?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'reigster-kubernetes-cluster',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: isEmpty(rowItem)
          ? this.i18n.get('common_register_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RegisterDatasetComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          rowItem
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RegisterDatasetComponent;
            content.onOK().subscribe(
              _ => {
                resolve(true);
                this.BaseTemplateComponent.dataTable?.fetchData();
              },
              _ => resolve(false)
            );
          });
        }
      })
    );
  }
}
