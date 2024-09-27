import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SelectDiskComponent } from './select-disk.component';

@NgModule({
  declarations: [SelectDiskComponent],
  imports: [CommonModule, BaseModule, ProTableModule, AlertModule],
  exports: [SelectDiskComponent]
})
export class SelectDiskModule {}
