import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { MultiSettingComponent } from './multi-setting.component';

@NgModule({
  declarations: [MultiSettingComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [MultiSettingComponent]
})
export class MultiSettingModule {}
