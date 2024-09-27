import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { MysqlRestoreComponent } from './mysql-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [MysqlRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class MysqlRestoreModule {}
