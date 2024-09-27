import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { InstanceTableComponent } from './instance-table.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProStatusModule } from 'app/shared/components/pro-status';

@NgModule({
  declarations: [InstanceTableComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProStatusModule],
  exports: [InstanceTableComponent]
})
export class InstanceTableModule {}
