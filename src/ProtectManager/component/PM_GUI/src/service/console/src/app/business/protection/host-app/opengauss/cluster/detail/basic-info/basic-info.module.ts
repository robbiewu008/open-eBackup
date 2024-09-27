import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BasicInfoComponent } from './basic-info.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [BasicInfoComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [BasicInfoComponent]
})
export class BasicInfoModule {}
