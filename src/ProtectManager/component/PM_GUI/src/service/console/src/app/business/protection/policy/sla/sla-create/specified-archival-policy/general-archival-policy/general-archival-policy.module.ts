import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { CreateArchiveStorageModule } from 'app/business/system/infrastructure/archive-storage/create-archive-storage/create-archive-storage.module';
import { CreateStoragePoolModule } from 'app/business/system/infrastructure/archive-storage/storage-pool-list/create-storage-pool/create-storage-pool.module';
import { BaseModule } from 'app/shared';
import { CurrentSystemTimeModule } from 'app/shared/components/current-system-time/current-system-time.module';
import { GeneralArchivalPolicyComponent } from './general-archival-policy.component';

@NgModule({
  declarations: [GeneralArchivalPolicyComponent],
  imports: [
    CommonModule,
    BaseModule,
    CreateArchiveStorageModule,
    CreateStoragePoolModule,
    CurrentSystemTimeModule
  ],
  exports: [GeneralArchivalPolicyComponent]
})
export class GeneralArchivalPolicyModule {}
