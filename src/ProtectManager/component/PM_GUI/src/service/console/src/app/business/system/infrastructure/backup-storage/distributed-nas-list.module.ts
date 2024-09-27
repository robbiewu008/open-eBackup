import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';
import { AddBackupNodeModule } from '../cluster-management/add-backup-node/add-backup-node.module';
import { BackupStorageDeviceModule } from './backup-storage-device/backup-storage-device.module';
import { BackupStorageUnitModule } from './backup-storage-unit/backup-storage-unit.module';
import { CreateDistributedNasModule } from './create-distributed-nas/create-distributed-nas.module';
import { CreateStorageDeviceComponent } from './create-storage-device/create-storage-device.component';
import { CreateStorageUnitComponent } from './create-storage-unit/create-storage-unit.component';
import { ClusterDetailModule } from './distributed-nas-detail/cluster-detail.module';
import { DistributedNasListComponent } from './distributed-nas-list.component';
import { DistributedNasListRoutingModule } from './distributed-nas-routing.module';

@NgModule({
  declarations: [
    DistributedNasListComponent,
    CreateStorageUnitComponent,
    CreateStorageDeviceComponent
  ],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ClusterDetailModule,
    ProButtonModule,
    CreateDistributedNasModule,
    SelectProtectObjectsModule,
    MultiClusterSwitchModule,
    DistributedNasListRoutingModule,
    BackupStorageUnitModule,
    AddBackupNodeModule,
    BackupStorageDeviceModule
  ],
  exports: [DistributedNasListComponent]
})
export class DistributedNasListModule {}
