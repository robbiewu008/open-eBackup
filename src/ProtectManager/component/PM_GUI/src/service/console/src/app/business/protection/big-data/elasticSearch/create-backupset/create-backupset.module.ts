import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateBackupsetComponent } from './create-backupset.component';
import { BaseModule } from 'app/shared';
import { SelectTableModule } from './select-table/select-table.module';

@NgModule({
  declarations: [CreateBackupsetComponent],
  imports: [CommonModule, BaseModule, SelectTableModule],
  exports: [CreateBackupsetComponent]
})
export class CreateBackupsetModule {}
