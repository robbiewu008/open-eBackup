import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table';
import { EnvironmentInfoApsaraStackComponent } from './environment-info-apsara-stack.component';

@NgModule({
  declarations: [EnvironmentInfoApsaraStackComponent],
  imports: [CommonModule, BaseModule, ProTableModule, CustomModalOperateModule],
  exports: [EnvironmentInfoApsaraStackComponent]
})
export class EnvironmentInfoApsaraStackModule {}
