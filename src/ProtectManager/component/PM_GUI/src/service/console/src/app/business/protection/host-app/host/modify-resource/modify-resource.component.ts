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
import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  ClientManagerApiService,
  DataMapService,
  I18NService,
  HostAgentUpdateControllerService,
  filterApplication
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { each, find, get, map, size } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-modify-resource',
  templateUrl: './modify-resource.component.html',
  styleUrls: ['./modify-resource.component.less']
})
export class ModifyResourceComponent implements OnInit {
  formGroup: FormGroup;
  treeData = [];
  data;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private clientManagerApiService: ClientManagerApiService,
    private hostAgentUpdateControllerService: HostAgentUpdateControllerService,
    private appUtilsService: AppUtilsService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.queryApplicationList();
  }

  initForm() {
    this.formGroup = this.fb.group({
      applications: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
  }

  updateData() {
    if (!this.data || !size(this.data)) {
      return;
    }
    const applicationsData = JSON.parse(this.data.extendInfo.agent_applications)
      .menus;

    const arr = [];
    each(this.treeData, item => {
      const parentRes = find(applicationsData, { menuValue: item.menuValue });
      if (parentRes) {
        if (parentRes.isChosen) {
          //选中父级
          arr.push(item);
        } else {
          //选中子级
          each(item.children, v => {
            if (find(parentRes.applications, { appValue: v.value })) {
              v.parent = {
                ...item
              };
              arr.push(v);
            }
          });
        }
      }
    });

    this.formGroup.get('applications').setValue(arr);
  }

  queryApplicationList() {
    this.clientManagerApiService
      .queryAgentApplicationsGET({
        lang: this.i18n.language,
        osType: this.data.osType,
        hostId: this.data.uuid
      })
      .subscribe(res => {
        const resourceArr = [];
        each(res as any, item => {
          resourceArr.push({
            ...item,
            label: item.menuDesc,
            key: item.menuValue,
            value: item.menuValue,
            disabled: false,
            isLeaf: false,
            children: map(filterApplication(item, this.appUtilsService), v => {
              return {
                ...v,
                label: v.appDesc,
                key: v.appValue,
                value: v.appValue,
                disabled: false,
                isLeaf: true
              };
            })
          });
          this.treeData = resourceArr;
          this.updateData();
        });
      });
  }
  getParams() {
    const paramArr = [];
    each(this.formGroup.value.applications, item => {
      if (!get(item, 'isLeaf')) {
        //父级
        each(item.applications, v => {
          v.isChosen = true;
          delete v.appDesc;
        });
        paramArr.push({
          menuValue: item.menuValue,
          menuLabel: item.menuLabel,
          isChosen: true,
          applications: item.applications
        });
      } else {
        //子级
        const childRes = {
          appValue: item.appValue,
          appLabel: item.appLabel,
          pluginName: item.pluginName,
          isChosen: true
        };
        const parentRes = find(paramArr, { menuValue: item.parent.menuValue });
        if (parentRes) {
          parentRes.applications.push(childRes);
        } else {
          paramArr.push({
            menuValue: item.parent.menuValue,
            menuLabel: item.parent.menuLabel,
            isChosen: false,
            applications: [childRes]
          });
        }
      }
    });
    return paramArr;
  }

  onOK(): Observable<void> {
    const applicationList = this.getParams();
    const params = {
      uuid: this.data.uuid,
      agentApplicationMenu: {
        menus: applicationList
      }
    };
    return new Observable<void>((observer: Observer<void>) => {
      this.hostAgentUpdateControllerService
        .UpdateAgentClientPluginTypeUsingPOST({ request: params })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }
}
