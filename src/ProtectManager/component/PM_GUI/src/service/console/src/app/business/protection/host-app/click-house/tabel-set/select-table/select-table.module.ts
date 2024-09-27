import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SelectTableComponent } from './select-table.component';
import { TransferModule } from '@iux/live';

@NgModule({
  imports: [CommonModule, BaseModule, ProTableModule, TransferModule],
  declarations: [SelectTableComponent],
  exports: [SelectTableComponent]
})
export class SelectTableModule {}
