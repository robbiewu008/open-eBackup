import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';
import { TdsqlRestoreComponent } from './tdsql-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [TdsqlRestoreComponent],
  imports: [CommonModule, BaseModule, ResourceFilterModule, AlertModule],
  exports: [TdsqlRestoreComponent]
})
export class TdsqlRestoreModule {}
