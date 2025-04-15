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
import { Injectable } from '@angular/core';
import {
  CookieService,
  DataMap,
  E6000SupportApplication,
  I18NService,
  hcsNoSupportApplication
} from 'app/shared';
import {
  filter,
  find,
  map,
  size,
  includes,
  reject,
  each,
  isEmpty,
  assign
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { RESOURCE_CATALOGS } from '..';
import {
  CatalogName,
  CommonConsts,
  SupportLicense
} from '../consts/common.const';

@Injectable({
  providedIn: 'root'
})
export class ResourceCatalogsService {
  private catalogIds = [];

  constructor(
    private cookieService: CookieService,
    private i18n: I18NService
  ) {}

  getResourceCatalog(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      this.catalogIds = this.parseCatalogs(RESOURCE_CATALOGS);
      observer.next(this.catalogIds);
      observer.complete();
    });
  }

  parseCatalogs(result) {
    each(result, item => {
      if (!isEmpty(item.children)) {
        each(item.children, child => {
          if (child.catalogName) {
            assign(child, {
              catalog_name: child.catalogName
            });
          }
        });
      }
    });
    let items = [];
    const hostAppsItems = find(result, item => {
      return item.catalog_name === CatalogName.HostApps;
    });
    if (hostAppsItems && hostAppsItems.show) {
      items.push(hostAppsItems.catalog_name);
      items = items.concat(
        map(
          filter(hostAppsItems.children, item => item.show),
          'catalog_name'
        )
      );
    }

    const VmItems = find(result, item => {
      return item.catalog_name === CatalogName.Virtualization;
    });
    if (VmItems && VmItems.show) {
      items.push(VmItems.catalog_name);
      items = items.concat(
        map(
          filter(VmItems.children, item => item.show),
          'catalog_name'
        )
      );
    }

    const copiesItems = find(result, item => {
      return item.catalog_name === CatalogName.Copies;
    });
    if (copiesItems && copiesItems.show) {
      items.push(copiesItems.catalog_name);
      items = items.concat(
        map(
          filter(copiesItems.children, item => item.show),
          'catalog_name'
        )
      );
    }

    const bigDataItems = find(result, item => {
      return item.catalog_name === CatalogName.BigData;
    });
    if (bigDataItems && bigDataItems.show) {
      items.push(bigDataItems.catalog_name);
      items = items.concat(
        map(
          filter(bigDataItems.children, item => item.show),
          'catalog_name'
        )
      );
    }

    const storageItems = find(result, item => {
      return item.catalog_name === CatalogName.Storage;
    });
    if (storageItems && storageItems.show) {
      items.push(storageItems.catalog_name);
      items = items.concat(
        map(
          filter(storageItems.children, item => item.show),
          'catalog_name'
        )
      );
    }

    const cloudItems = find(result, item => {
      return item.catalog_name === CatalogName.Cloud;
    });
    if (cloudItems && cloudItems.show) {
      items.push(cloudItems.catalog_name);
      items = items.concat(
        map(
          filter(cloudItems.children, item => item.show),
          'catalog_name'
        )
      );
    }

    const applicationItems = find(result, item => {
      return item.catalogName === CatalogName.Application;
    });
    if (applicationItems && applicationItems.show) {
      items.push(applicationItems.catalogName);
      items = items.concat(
        map(
          filter(applicationItems.children, item => item.show),
          'catalogName'
        )
      );
    }

    const vesselItems = find(result, item => {
      return item.catalog_name === CatalogName.Vessel;
    });
    if (vesselItems && vesselItems.show) {
      items.push(vesselItems.catalog_name);
      items = items.concat(
        map(
          filter(vesselItems.children, item => item.show),
          'catalog_name'
        )
      );
    }

    if (this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE) {
      return reject(items, item => includes(hcsNoSupportApplication, item));
    }

    if (this.i18n.get('deploy_type') === DataMap.Deploy_Type.e6000.value) {
      return reject(items, item => !includes(E6000SupportApplication, item));
    }

    if (
      includes(
        [
          DataMap.Deploy_Type.cloudbackup2.value,
          DataMap.Deploy_Type.cloudbackup.value
        ],
        this.i18n.get('deploy_type')
      )
    ) {
      return filter(items, item =>
        includes(
          [CatalogName.Storage, DataMap.Resource_Type.LocalFileSystem.value],
          item
        )
      );
    } else if (
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
    ) {
      return SupportLicense.isBoth
        ? filter(items, item =>
            includes(
              [
                CatalogName.Storage,
                DataMap.Resource_Type.LocalFileSystem.value,
                DataMap.Resource_Type.LocalLun.value
              ],
              item
            )
          )
        : SupportLicense.isFile
        ? filter(items, item =>
            includes(
              [
                CatalogName.Storage,
                DataMap.Resource_Type.LocalFileSystem.value
              ],
              item
            )
          )
        : filter(items, item =>
            includes(
              [CatalogName.Storage, DataMap.Resource_Type.LocalLun.value],
              item
            )
          );
    } else {
      return reject(items, item =>
        includes(
          [
            DataMap.Resource_Type.LocalFileSystem.value,
            DataMap.Resource_Type.LocalLun.value
          ],
          item
        )
      );
    }
  }
}
