import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CnwareRestoreComponent } from './cnware-restore.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [CnwareRestoreComponent],
  imports: [CommonModule, BaseModule, ProTableModule, AlertModule]
})
export class CnwareRestoreModule {}
