import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SelectBackupSetListComponent } from './select-backup-set-list.component';
import { BaseModule } from 'app/shared';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';

@NgModule({
  declarations: [SelectBackupSetListComponent],
  imports: [CommonModule, BaseModule, SelectProtectObjectsModule],
  exports: [SelectBackupSetListComponent]
})
export class SelectBackupSetListModule {}
