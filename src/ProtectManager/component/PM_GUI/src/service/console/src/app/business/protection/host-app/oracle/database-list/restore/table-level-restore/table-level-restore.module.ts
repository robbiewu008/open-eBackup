import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { CascaderModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { TableLevelRestoreComponent } from './table-level-restore.component';

@NgModule({
  declarations: [TableLevelRestoreComponent],
  imports: [CommonModule, BaseModule, CascaderModule, ProTableModule]
})
export class OracleTableLevelRestoreModule {}
