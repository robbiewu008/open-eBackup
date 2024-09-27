import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddDrillResourceComponent } from './add-drill-resource.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [AddDrillResourceComponent],
  imports: [CommonModule, BaseModule, ProTableModule]
})
export class AddDrillResourceModule {}
