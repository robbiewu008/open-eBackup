import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddDiskComponent } from './add-disk.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [AddDiskComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [AddDiskComponent]
})
export class AddDiskModule {}
