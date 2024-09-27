import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DatabaseRestoreComponent } from './database-restore.component';
import { BaseModule } from 'app/shared';
import { SelectTagModule } from 'app/shared/components/select-tag/select-tag.module';

@NgModule({
  declarations: [DatabaseRestoreComponent],
  imports: [CommonModule, BaseModule, SelectTagModule]
})
export class DatabaseRestoreModule {}
