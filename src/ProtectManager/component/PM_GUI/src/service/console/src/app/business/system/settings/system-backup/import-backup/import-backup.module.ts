import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ImportBackupComponent } from './import-backup.component';

@NgModule({
  declarations: [ImportBackupComponent],
  imports: [CommonModule, BaseModule]
})
export class ImportBackupModule {}
