import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateBackupSetComponent } from './create-backup-set.component';
import { BaseModule } from 'app/shared';
import { TablesModule } from '../tables/tables.module';

@NgModule({
  declarations: [CreateBackupSetComponent],
  imports: [CommonModule, BaseModule, TablesModule],
  exports: [CreateBackupSetComponent]
})
export class CreateBackupSetModule {}
