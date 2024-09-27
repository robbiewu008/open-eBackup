import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { TagModule } from '@iux/live';
import { DatabaseConfigModule } from 'app/shared/components/database-config/database-config.module';
import { OracleSingleFileRestoreComponent } from './oracle-single-file-restore.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [OracleSingleFileRestoreComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    TagModule,
    DatabaseConfigModule
  ]
})
export class OracleSingleFileRestoreModule {}
