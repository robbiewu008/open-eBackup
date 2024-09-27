import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap, CookieService } from 'app/shared';
import { each } from 'lodash';
import { StorageAuthComponent } from './storage-auth/storage-auth.component';
import { StorageSummaryComponent } from './storage-summary/storage-summary.component';

@Component({
  selector: 'aui-local-storage',
  templateUrl: './local-storage.component.html',
  styleUrls: ['./local-storage.component.less']
})
export class LocalStorageComponent implements OnInit {
  @ViewChild(StorageSummaryComponent, { static: false })
  storageSummaryComponent: StorageSummaryComponent;

  @ViewChild(StorageAuthComponent, { static: false })
  StorageAuthComponent: StorageAuthComponent;

  constructor(public cookieService: CookieService) {}

  ngOnInit() {}

  onStatusChange(res) {
    each(res, item => {
      if (
        item.authType === 'serviceAuth' &&
        item.status === DataMap.Storage_Status.normal.value
      ) {
        this.storageSummaryComponent.getData();
      }

      if (item.authType === 'managerAuth') {
        this.storageSummaryComponent.ableJump =
          item.status === DataMap.Storage_Status.normal.value ? true : false;
      }
    });
  }

  openDeviceChange() {
    this.StorageAuthComponent.ngOnInit();
  }

  onChange() {
    this.StorageAuthComponent.ngOnInit();
    this.storageSummaryComponent.ngOnInit();
  }
}
