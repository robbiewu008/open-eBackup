import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BatchOperateServiceModule } from 'app/shared/services/batch-operate.service';
import { ArchiveStorageDetailModule } from './archive-storage-detail/archive-storage-detail.module';
import { StorageManagementRoutingModule } from './archive-storage-routing.module';
import {
  ArchiveStorageComponent,
  SelectionPipe
} from './archive-storage.component';
import { CreateArchiveStorageModule } from './create-archive-storage/create-archive-storage.module';
import { ModifyAlarmThresholdModule } from './modify-alarm-threshold/modify-alarm-threshold.module';
import { StoragePoolListModule } from './storage-pool-list/storage-pool-list.module';
import { StorageDeviceListModule } from './storage-device-list/storage-device-list.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [ArchiveStorageComponent, SelectionPipe],
  imports: [
    CommonModule,
    BaseModule,
    StoragePoolListModule,
    StorageDeviceListModule,
    StorageManagementRoutingModule,
    CreateArchiveStorageModule,
    ModifyAlarmThresholdModule,
    ArchiveStorageDetailModule,
    BatchOperateServiceModule,
    MultiClusterSwitchModule
  ]
})
export class ArchiveStorageModule {}
