import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table';
import { EnvironmentInfoComponent } from './environment-info.component';

@NgModule({
  declarations: [EnvironmentInfoComponent],
  imports: [CommonModule, BaseModule, CustomModalOperateModule, ProTableModule],
  exports: [EnvironmentInfoComponent]
})
export class EnvironmentInfoModule {}
