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
import {
  FormGroup,
  FormBuilder,
  FormControl,
  Validators
} from '@angular/forms';
import {
  StorageLocation,
  VmwareService,
  LiveMountApiService,
  CAPACITY_UNIT
} from 'app/shared';
import { Observable, Observer, Subject } from 'rxjs';
import { each, find, isEmpty, assign, map, filter } from 'lodash';

@Component({
  selector: 'aui-live-mount-migrate',
  templateUrl: './live-mount-migrate.component.html',
  styleUrls: ['./live-mount-migrate.component.less']
})
export class LiveMountMigrateComponent implements OnInit {
  item;
  formGroup: FormGroup;
  storageLocation = StorageLocation;
  dataStoreOptions = [];
  diskData = [];
  diskValid$ = new Subject<boolean>();
  unitconst = CAPACITY_UNIT;
  cacheDatastore;
  constructor(
    public fb: FormBuilder,
    private vMwareService: VmwareService,
    private liveMountApiService: LiveMountApiService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      storageLocation: new FormControl(StorageLocation.Different),
      vmStorageLocation: new FormControl('', {
        validators: [Validators.required]
      })
    });
    this.formGroup.get('storageLocation').valueChanges.subscribe(res => {
      if (res === StorageLocation.Different) {
        this.diskDataStoreChange(null);
      } else {
        this.diskValid$.next(true);
      }
    });
  }

  getDisks() {
    this.vMwareService
      .listVmDiskV1VirtualMachinesVmUuidDisksGet({
        vmUuid: this.item.mounted_resource_id
      })
      .subscribe(res => {
        this.diskData = res;
      });
  }

  diskDataStoreChange(e) {
    const sameDatastoreDisk = filter(this.diskData, item => {
      return item.target_datastore === e;
    });
    const dataStore = find(this.cacheDatastore, { mo_id: e });
    if (dataStore && sameDatastoreDisk) {
      let useSpace = 0;
      each(sameDatastoreDisk, disk => {
        useSpace += disk.capacity;
      });
      let freeSpace = dataStore.free_space - useSpace;
      each(this.diskData, item => {
        if (item.target_datastore === e) {
          assign(item, {
            targetDatastoreFreeSpace: freeSpace < 0 ? 0 : freeSpace
          });
        }
      });
    }
    if (
      find(this.diskData, item => {
        return isEmpty(item.target_datastore);
      })
    ) {
      this.diskValid$.next(false);
    } else {
      this.diskValid$.next(true);
    }
  }

  getDataStore() {
    this.vMwareService
      .getVmInfoV1VirtualMachinesVmUuidGet({
        vmUuid: this.item.mounted_resource_id
      })
      .subscribe(vm => {
        this.vMwareService
          .listComputeResDatastoreV1ComputeResourcesComputeResUuidDatastoresGet(
            {
              computeResUuid: vm.runtime.host.uuid
            }
          )
          .subscribe(res => {
            const datastores = [];
            each(res, item => {
              datastores.push({
                key: item.mo_id,
                value: item.mo_id,
                label: item.name,
                isLeaf: true
              });
            });
            this.dataStoreOptions = datastores;
            this.cacheDatastore = res;
          });
      });
  }

  onOK(): Observable<void> {
    return new Observable<any>((observer: Observer<void>) => {
      const disks = map(this.diskData, item => {
        return {
          datastore: {
            moId: item.target_datastore
          },
          diskId: item.uuid
        };
      });
      const params = {
        liveMountId: this.item.id,
        liveMountMigrateRequest: {
          vmWareMigrateParam: {
            diskDatastoreType: this.formGroup.value.storageLocation,
            vmxDatastore: {
              moId: this.formGroup.value.vmStorageLocation
            }
          }
        }
      };
      if (this.formGroup.value.storageLocation === StorageLocation.Different) {
        assign(params.liveMountMigrateRequest.vmWareMigrateParam, {
          disk: disks
        });
      }
      this.liveMountApiService.migrateUsingPUT(params).subscribe(
        res => {
          observer.next();
          observer.complete();
        },
        err => {
          observer.error(err);
          observer.complete();
        }
      );
    });
  }

  ngOnInit() {
    this.initForm();
    this.getDisks();
    this.getDataStore();
  }
}
