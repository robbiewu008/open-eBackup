import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { TableRestoreComponent } from './restore-table.component';

@NgModule({
  declarations: [TableRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [TableRestoreComponent]
})
export class TableRestoreModule {}
