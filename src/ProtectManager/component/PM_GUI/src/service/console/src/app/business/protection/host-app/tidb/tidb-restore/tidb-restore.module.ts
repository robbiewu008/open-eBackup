import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';
import { TidbRestoreComponent } from './tidb-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [TidbRestoreComponent],
  imports: [CommonModule, BaseModule, ResourceFilterModule, SelectTagModule],
  exports: [TidbRestoreComponent]
})
export class TidbRestoreModule {}
