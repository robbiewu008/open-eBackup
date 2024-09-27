import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddApsDiskComponent } from './add-aps-disk.component';

@NgModule({
  declarations: [AddApsDiskComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [AddApsDiskComponent]
})
export class AddApsDiskModule {}
