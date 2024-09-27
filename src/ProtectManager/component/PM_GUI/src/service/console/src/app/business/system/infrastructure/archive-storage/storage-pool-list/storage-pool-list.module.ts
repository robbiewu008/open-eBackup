import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CreateStoragePoolModule } from './create-storage-pool/create-storage-pool.module';
import { StoragePoolDetailModule } from './storage-pool-detail/storage-pool-detail.module';
import { StoragePoolListComponent } from './storage-pool-list.component';

@NgModule({
  declarations: [StoragePoolListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    StoragePoolDetailModule,
    CreateStoragePoolModule
  ],
  exports: [StoragePoolListComponent]
})
export class StoragePoolListModule {}
