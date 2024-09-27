import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddDiskModule } from './add-disk/add-disk.module';
import { SelectPoComponent } from './select-po.component';

@NgModule({
  declarations: [SelectPoComponent],
  imports: [CommonModule, BaseModule, ProTableModule, AddDiskModule],
  exports: [SelectPoComponent]
})
export class SelectPoModule {}
