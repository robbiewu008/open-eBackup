import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AddStorageModule } from './add-storage/add-storage.module';
import { StorageDeviceInfoRoutingModule } from './storage-device-info-routing.module';
import { StorageDeviceInfoComponent } from './storage-device-info.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [StorageDeviceInfoComponent],
  imports: [
    CommonModule,
    StorageDeviceInfoRoutingModule,
    BaseModule,
    AddStorageModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    MultiClusterSwitchModule
  ]
})
export class StorageDeviceInfoModule {}
