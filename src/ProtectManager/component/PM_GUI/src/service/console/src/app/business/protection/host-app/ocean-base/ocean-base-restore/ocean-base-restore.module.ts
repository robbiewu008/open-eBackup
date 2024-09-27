import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';
import { OceanBaseRestoreComponent } from './ocean-base-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [OceanBaseRestoreComponent],
  imports: [CommonModule, BaseModule, ResourceFilterModule, SelectTagModule]
})
export class OceanBaseRestoreModule {}
