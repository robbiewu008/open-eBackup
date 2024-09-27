import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { StoragePoolDetailComponent } from './storage-pool-detail.component';

@NgModule({
  declarations: [StoragePoolDetailComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [StoragePoolDetailComponent]
})
export class StoragePoolDetailModule {}
