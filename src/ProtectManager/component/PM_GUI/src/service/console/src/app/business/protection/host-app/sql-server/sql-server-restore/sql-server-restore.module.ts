import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SQLServerRestoreComponent } from './sql-server-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [SQLServerRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class SQLServerRestoreModule {}
