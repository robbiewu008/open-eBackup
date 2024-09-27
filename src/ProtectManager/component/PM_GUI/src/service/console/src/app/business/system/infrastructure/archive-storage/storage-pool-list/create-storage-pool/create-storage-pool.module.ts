import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CreateStoragePoolComponent } from './create-storage-pool.component';

@NgModule({
  declarations: [CreateStoragePoolComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [CreateStoragePoolComponent]
})
export class CreateStoragePoolModule {}
