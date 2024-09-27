import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { VirtualizationGroupComponent } from './virtualization-group.component';
import { GroupSummaryModule } from './group-summary/group-summary.module';
import { BaseTableModule } from '../virtualization-base/base-table/base-table.module';

@NgModule({
  declarations: [VirtualizationGroupComponent],
  imports: [CommonModule, GroupSummaryModule, BaseTableModule],
  exports: [VirtualizationGroupComponent]
})
export class VirtualizationGroupModule {}
