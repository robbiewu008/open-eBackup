import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { InstanceComponent } from './instance.component';
import { BaseModule } from 'app/shared';
import { BaseTemplateModule } from '../base-template/base-template.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [InstanceComponent],
  imports: [
    CommonModule,
    BaseModule,
    BaseTemplateModule,
    ProButtonModule,
    ProTableModule
  ],
  exports: [InstanceComponent]
})
export class InstanceModule {}
