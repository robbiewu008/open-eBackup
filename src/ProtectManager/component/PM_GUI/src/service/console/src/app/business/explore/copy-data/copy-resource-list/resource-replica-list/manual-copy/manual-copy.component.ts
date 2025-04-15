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
import { OptionItem } from '@iux/live';
import {
  BaseUtilService,
  ClustersApiService,
  DataMap,
  I18NService,
  ProtectedCopyObjectApiService,
  QosService,
  SlaApiService
} from 'app/shared';
import { SlaParseService } from 'app/shared/services/sla-parse.service';
import { assign, defer, find, includes } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-manual-copy',
  templateUrl: './manual-copy.component.html',
  styleUrls: ['./manual-copy.component.less']
})
export class ManualCopyComponent implements OnInit {
  rowItem;
  formGroup: FormGroup;
  replicationPolicyOptions: OptionItem[] = [];
  replicationItem;
  externalSystems = [];
  qosNames = [];
  isDistributed =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.e6000.value;
  find = find;
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    private qosServiceApi: QosService,
    private slaApiService: SlaApiService,
    private slaParseService: SlaParseService,
    private baseUtilService: BaseUtilService,
    private clusterApiService: ClustersApiService,
    private protectedCopyObjectApiService: ProtectedCopyObjectApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getExtSystems();
    this.getQosNames();
    this.getSlaInfo();
  }

  initForm() {
    this.formGroup = this.fb.group({
      replication_policy: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    this.formGroup.get('replication_policy').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      defer(() => {
        this.replicationItem = find(this.replicationPolicyOptions, {
          uuid: res
        });
      });
    });
  }

  getSlaInfo() {
    this.slaApiService
      .querySLAUsingGET({ slaId: this.rowItem?.protected_sla_id })
      .subscribe(res => {
        if (res) {
          this.replicationPolicyOptions = this.slaParseService
            .getReplication(res, [])
            .policyList.map(item => {
              return assign(item, { label: item.name, isLeaf: true });
            });
        }
      });
  }

  getExtSystems() {
    this.clusterApiService
      .getClustersInfoUsingGET({
        startPage: 0,
        pageSize: 200,
        typeList: [DataMap.Cluster_Type.target.value]
      })
      .subscribe(res => {
        this.externalSystems = res.records;
      });
  }

  getQosNames() {
    this.qosServiceApi
      .queryResourcesV1QosGet({
        pageNo: 0,
        pageSize: 100
      })
      .subscribe(res => {
        this.qosNames = res.items;
      });
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      this.protectedCopyObjectApiService
        .manualReplicateUsiungPOST({
          manualReplicateReq: {
            policy_id: this.formGroup.value.replication_policy,
            sla_id: this.rowItem?.protected_sla_id,
            resource_id: this.rowItem?.resource_id
          }
        })
        .subscribe(
          res => {
            observer.next(res);
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
