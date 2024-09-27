import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { StorageDeviceRoutingModule } from './storage-device-routing.module';
import { StorageDeviceComponent } from './storage-device.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { BaseModule } from 'app/shared';
import { AddStorageDeviceModule } from './add-storage-device/add-storage-device.module';

@NgModule({
  declarations: [StorageDeviceComponent],
  imports: [
    CommonModule,
    StorageDeviceRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    AddStorageDeviceModule
  ],
  exports: [StorageDeviceComponent]
})
export class StorageDeviceModule {}
