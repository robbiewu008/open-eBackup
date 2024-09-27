import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { GoldendbRestoreComponent } from './goldendb-restore.component';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [GoldendbRestoreComponent],
  imports: [CommonModule, BaseModule, ResourceFilterModule, SelectTagModule],
  exports: [GoldendbRestoreComponent]
})
export class GoldendbRestoreModule {}
