import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { LocalStorageRoutingModule } from './local-storage-routing.module';
import { LocalStorageComponent } from './local-storage.component';
import { StorageAuthModule } from './storage-auth/storage-auth.module';
import { StorageSummaryModule } from './storage-summary/storage-summary.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [LocalStorageComponent],
  imports: [
    CommonModule,
    LocalStorageRoutingModule,
    BaseModule,
    StorageAuthModule,
    StorageSummaryModule,
    MultiClusterSwitchModule
  ]
})
export class LocalStorageModule {}
