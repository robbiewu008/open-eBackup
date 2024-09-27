import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SQLServerAlwaysOnComponent } from './alwayson-restore.component';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [SQLServerAlwaysOnComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class SQLServerAlwaysonModule {}
