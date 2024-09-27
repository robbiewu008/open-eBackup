import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { TransferModule } from '@iux/live';
import { RestoreComponent } from './restore.component';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [RestoreComponent],
  imports: [
    CommonModule,
    BaseModule,
    ResourceFilterModule,
    TransferModule,
    SelectTagModule
  ],
  exports: [RestoreComponent]
})
export class RestoreModule {}
