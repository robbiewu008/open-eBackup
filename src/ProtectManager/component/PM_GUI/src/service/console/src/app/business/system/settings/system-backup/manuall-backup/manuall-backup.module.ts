import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ManuallBackupComponent } from './manuall-backup.component';

@NgModule({
  declarations: [ManuallBackupComponent],
  imports: [CommonModule, BaseModule]
})
export class ManuallBackupModule {}
