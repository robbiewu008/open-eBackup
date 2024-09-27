import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { GaussdbRestoreComponent } from './gaussdb-restore.component';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [GaussdbRestoreComponent],
  imports: [CommonModule, BaseModule, ResourceFilterModule, SelectTagModule],
  exports: [GaussdbRestoreComponent]
})
export class GaussdbRestoreModule {}
