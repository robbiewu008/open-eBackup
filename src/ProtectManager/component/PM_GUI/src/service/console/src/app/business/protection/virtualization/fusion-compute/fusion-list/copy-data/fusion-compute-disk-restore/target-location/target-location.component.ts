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
import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  ProtectedResourceApiService,
  ResourceType,
  RestoreV2LocationType,
  DatastoreType,
  LANGUAGE,
  I18NService,
  DataMap,
  AppService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isNumber,
  last,
  map
} from 'lodash';
import { forkJoin, Observable, of } from 'rxjs';
import { mergeMap } from 'rxjs/operators';

@Component({
  selector: 'aui-target-location',
  templateUrl: './target-location.component.html',
  styleUrls: ['./target-location.component.less']
})
export class TargetLocationComponent implements OnInit {
  @Input() rowCopy;
  isEn = this.i18n.language.toLowerCase() === LANGUAGE.EN;
  formGroup: FormGroup;
  params;
  restoreLocationType = RestoreV2LocationType;
  resourceProp;
  properties;
  DatastoreType = DatastoreType;
  environmentOptions;
  dataStoreOptions;
  treeData;
  expandedNodeList = [];
  restoreToNewLocationOnly = false;
  existEnv;
  disabledOrigin = false;

  verifyStatus;
  copyVerifyDisableLabel: string;
  CopyDataVerifyStatus = DataMap.HCSCopyDataVerifyStatus;

  proxyOptions = [];

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private appService: AppService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.expandedNodeList = [];
    this.treeData = [];
    this.initForm();
    this.initCopyVerifyDisableLabel();
    this.getProxyOptions();
  }

  initCopyVerifyDisableLabel() {
    this.verifyStatus = JSON.parse(
      this.rowCopy.properties || '{}'
    ).verifyStatus;
    if (
      includes([this.CopyDataVerifyStatus.noGenerate.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_generate_verify_file_disable_label'
      );
    }
    if (
      includes([this.CopyDataVerifyStatus.Invalid.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_invalid_verify_file_disable_label'
      );
    }
  }

  initForm() {
    this.resourceProp = JSON.parse(this.rowCopy.resource_properties);
    this.properties = JSON.parse(this.rowCopy.properties);

    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) ||
      this.rowCopy.is_replicated ||
      !this.existEnv;

    if (!isEmpty(this.params?.newPosition)) {
      this.environmentOptions = this.params.newPosition?.environmentOptions;
      this.dataStoreOptions = this.params.newPosition?.dataStoreOptions;
    }

    if (!isEmpty(this.params?.newPosition)) {
      this.treeData = this.params?.newPosition.treeData;
    }
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(
        !isEmpty(this.params?.newPosition)
          ? RestoreV2LocationType.NEW
          : RestoreV2LocationType.ORIGIN
      ),
      originPosition: new FormControl({
        value: `${this.resourceProp.name}(${this.resourceProp.environment_endpoint})`,
        disabled: true
      }),
      environment: new FormControl(
        !isEmpty(this.params?.newPosition)
          ? this.params.newPosition.environment.uuid
          : ''
      ),
      host: new FormControl(
        !isEmpty(this.params?.newPosition) ? this.params.newPosition.host : []
      ),
      storage: new FormControl(
        !isEmpty(this.params) && this.params?.isSame
          ? DatastoreType.SAME
          : DatastoreType.DIFFERENT
      ),
      dataStore: new FormControl(
        !isEmpty(this.params?.newPosition)
          ? this.params.newPosition?.dataStore?.uuid || []
          : []
      ),
      proxyHost: new FormControl(this.params?.agents || []),
      power_on: new FormControl(
        isEmpty(this.params)
          ? false
          : this.params?.extendInfo.power_on === 'true'
      ),
      copyVerify: new FormControl(
        !isEmpty(this.params) ? this.params?.extendInfo.copyVerify : false
      )
    });

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.NEW) {
        this.formGroup
          .get('environment')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('host')
          .setValidators([this.baseUtilService.VALID.required()]);
        if (this.formGroup.value.storage === DatastoreType.SAME) {
          this.formGroup
            .get('dataStore')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
        this.getEnvironment();
      } else {
        this.formGroup.get('environment').clearValidators();
        this.formGroup.get('host').clearValidators();
        this.formGroup.get('dataStore').clearValidators();
      }
      this.formGroup.get('environment').updateValueAndValidity();
      this.formGroup.get('host').updateValueAndValidity();
      this.formGroup.get('dataStore').updateValueAndValidity();
    });

    if (
      this.rowCopy?.resource_status === 'NOT_EXIST' ||
      this.restoreToNewLocationOnly
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
      this.disabledOrigin = true;
    }

    this.formGroup.get('storage').valueChanges.subscribe(res => {
      if (res === DatastoreType.SAME) {
        this.formGroup
          .get('dataStore')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('dataStore').clearValidators();
      }
      this.formGroup.get('dataStore').updateValueAndValidity();
    });

    this.formGroup.get('environment').valueChanges.subscribe(res => {
      this.formGroup.get('host').setValue('', { emitEvent: false });
      this.expandedNodeList = [];
      this.treeData = [];
      if (this.formGroup.get('storage').value === DatastoreType.SAME) {
        this.dataStoreOptions = [];
        this.formGroup.get('dataStore').setValue('', { emitEvent: false });
      }
      if (isEmpty(res)) {
        return;
      }
      defer(() => this.getTreeData());
    });

    this.formGroup.get('host').valueChanges.subscribe(res => {
      if (this.formGroup.get('storage').value === DatastoreType.SAME) {
        this.dataStoreOptions = [];
        this.formGroup.get('dataStore').setValue('', { emitEvent: false });
      }
      if (isEmpty(res)) {
        return;
      }
      defer(() => this.getDataStores());
    });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${this.rowCopy.resource_sub_type ||
            DataMap.Application_Type.FusionCompute.value}Plugin`
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value &&
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getEnvironment(recordsTemp?: any[], startPage?: number) {
    const conditions = {
      subType: [
        this.rowCopy.resource_sub_type ||
          DataMap.Resource_Type.FusionCompute.value
      ],
      type: ResourceType.PLATFORM
    };

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        recordsTemp = recordsTemp.filter(
          item =>
            item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
        );
        this.environmentOptions = map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });
        this.formGroup.updateValueAndValidity();
        return;
      }
      this.getEnvironment(recordsTemp, startPage);
    });
  }

  getTreeData(event?, startPage?) {
    const conditions = {
      rootUuid: this.formGroup.value.environment,
      type: [ResourceType.CLUSTER]
    };

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify(conditions)
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        const node = {
          label: item.name,
          name: item.name,
          contentToggleIcon: this.getResourceIcon(item),
          path: item.path,
          parentUuid: item.parentUuid,
          type: item.type,
          uuid: item.uuid,
          rootUuid: item.rootUuid,
          children: [],
          disabled: item.type !== ResourceType.VM,
          isLeaf: false,
          moReference: item.extendInfo?.moReference,
          expanded: this.getExpandedIndex(item.uuid) !== -1
        };
        if (node.expanded) {
          this.getExpandedChangeData(CommonConsts.PAGE_START, node);
        }

        if (!find(this.treeData, { uuid: node.uuid })) {
          this.treeData.push(node);
        }
      });
      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getTreeData(null, startPage);
        return;
      }
      this.treeData = [...this.treeData];
    });
  }

  expandedChange(event) {
    if (event.expanded) {
      this.expandedNodeList.push({
        uuid: event.uuid,
        rootUuid: event.rootUuid
      });
    } else {
      this.expandedNodeList.splice(this.getExpandedIndex(event.uuid), 1);
    }
    if (!event.expanded || event.children.length) {
      return;
    }
    event.children = [];
    this.getExpandedChangeData(CommonConsts.PAGE_START, event);
  }

  getExpandedIndex(id) {
    return this.expandedNodeList.findIndex(node => node.uuid === id);
  }

  getExpandedChangeData(startPage, event) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        parentUuid: event.uuid
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      res.records.sort((a, b) => {
        if (a.type === ResourceType.HOST) {
          return -1;
        } else if (a.type === b.type) {
          return 0;
        } else {
          return 1;
        }
      });

      res.records.forEach(item => {
        const rootNode = this.getRootNode(event),
          node = {
            label:
              item.type !== ResourceType.VM
                ? item.name
                : `${item.name}(${last(
                    item.extendInfo?.moReference?.split('/')
                  )})`,
            contentToggleIcon: this.getResourceIcon(item),
            path: item.path,
            type: item.type,
            uuid: item.uuid,
            rootUuid: event.rootUuid,
            disabled: item.type !== ResourceType.VM,
            parentUuid: item.parentUuid,
            children: [],
            isLeaf: item.type === ResourceType.VM,
            moReference: item.extendInfo?.moReference,
            location: item.extendInfo?.location,
            expanded: this.getExpandedIndex(item.uuid) !== -1
          };
        if (node.expanded) {
          this.getExpandedChangeData(CommonConsts.PAGE_START, node);
        }
        event.children.push(node);
      });
      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getExpandedChangeData(startPage, event);
        return;
      }
      this.treeData = [...this.treeData];
    });
  }

  getResourceIcon(node) {
    const nodeResource = find(
      DataMap.Resource_Type,
      item => item.value === node.type
    );
    return nodeResource['icon'] + '';
  }

  getRootNode(node) {
    if (!!node.parent) {
      return this.getRootNode(node.parent);
    } else {
      return node;
    }
  }

  getDataStores() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      queryDependency: true,
      conditions: JSON.stringify({
        subType:
          this.rowCopy.resource_sub_type ||
          DataMap.Resource_Type.FusionCompute.value,
        uuid: this.formGroup.value.environment
      })
    };
    this.protectedResourceApiService
      .ListResources(params)
      .subscribe((res: any) => {
        if (res.records?.length) {
          const onlineAgents = res.records[0]?.dependencies?.agents?.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          if (isEmpty(onlineAgents)) {
            this.dataStoreOptions = [];
            return;
          }
          const agentsId = onlineAgents[0].uuid;
          this.getShowData(agentsId).subscribe(response => {
            const totalData = [];
            for (const item of response) {
              totalData.push(...item.records);
            }
            each(totalData, (item: any) => {
              assign(item, {
                key: item.uuid,
                value: item.uuid,
                label: item.name,
                isLeaf: true,
                moReference: item?.extendInfo?.datastoreUri
              });
            });
            this.dataStoreOptions = totalData;
          });
        }
      });
  }

  getShowData(agentsId): Observable<any> {
    const params = {
      agentId: agentsId,
      envId: this.formGroup.value.environment,
      pageNo: 1,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      resourceIds: [this.formGroup.value?.host[0]?.parentUuid]
    };
    let curData = [];
    return this.appService.ListResourcesDetails(params).pipe(
      mergeMap((response: any) => {
        curData = [of(response)];

        const totalCount = response.totalCount;
        const pageCount = Math.ceil(totalCount / (CommonConsts.PAGE_SIZE * 10));
        for (let i = 2; i <= pageCount; i++) {
          curData.push(
            this.appService.ListResourcesDetails({
              agentId: agentsId,
              envId: this.formGroup.value.environment,
              pageNo: i,
              pageSize: CommonConsts.PAGE_SIZE * 10,
              resourceIds: [this.formGroup.value?.host[0]?.parentUuid]
            })
          );
        }
        return forkJoin(curData);
      })
    );
  }

  getTargetParams() {
    let params = {};
    if (
      this.formGroup.value.restoreLocation === this.restoreLocationType.ORIGIN
    ) {
      params = {
        agents: this.formGroup.value.proxyHost,
        path: this.resourceProp.path,
        extendInfo: {
          power_on: this.formGroup.value.power_on ? 'true' : 'false',
          copyVerify: this.formGroup.value.copyVerify
        }
      };
    } else {
      params = {
        agents: this.formGroup.value.proxyHost,
        location: this.formGroup.value.host[0]?.location,
        environment: find(this.environmentOptions, {
          uuid: this.formGroup.value.environment
        }),
        vm: this.formGroup.value.host[0],
        extendInfo: {
          power_on: this.formGroup.value.power_on ? 'true' : 'false',
          copyVerify: this.formGroup.value.copyVerify
        },
        path: this.formGroup.value.host[0].path,
        newPosition: {
          environmentOptions: this.environmentOptions,
          treeData: this.treeData,
          host: this.formGroup.value.host,
          dataStoreOptions: this.dataStoreOptions,
          dataStore: find(this.dataStoreOptions, {
            uuid: this.formGroup.value.dataStore
          }),
          environment: find(this.environmentOptions, {
            uuid: this.formGroup.value.environment
          })
        }
      };

      if (this.formGroup.value.storage === DatastoreType.SAME) {
        const selectDataStore = find(this.dataStoreOptions, {
          uuid: this.formGroup.value.dataStore
        });
        params['dataStore'] = [
          assign(selectDataStore, {
            size: +selectDataStore.extendInfo?.freeSizeGB,
            id: selectDataStore.uuid
          })
        ];
        params['path'] = this.formGroup.value.host[0].path;
        params['isSame'] = true;
      } else {
        params['isSame'] = false;
        params['dataStore'] = map(this.dataStoreOptions, item => {
          return {
            ...item,
            size: +item.extendInfo?.freeSizeGB,
            id: item.uuid
          };
        });
      }
    }

    return params;
  }
}
