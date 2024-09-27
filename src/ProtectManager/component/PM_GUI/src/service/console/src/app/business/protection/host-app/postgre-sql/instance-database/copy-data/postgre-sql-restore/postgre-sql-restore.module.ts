import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PostgreSqlRestoreComponent } from './postgre-sql-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [PostgreSqlRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class PostgreSqlRestoreModule {}
