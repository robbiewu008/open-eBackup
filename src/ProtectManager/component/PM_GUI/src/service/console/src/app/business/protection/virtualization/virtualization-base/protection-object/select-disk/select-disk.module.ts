import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SelectDiskComponent } from './select-disk.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SelectDiskComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [SelectDiskComponent]
})
export class SelectDiskModule {}
