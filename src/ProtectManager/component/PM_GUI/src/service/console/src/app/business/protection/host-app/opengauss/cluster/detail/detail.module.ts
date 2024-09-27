import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetailComponent } from './detail.component';
import { BaseModule } from 'app/shared';
import { BasicInfoModule } from './basic-info/basic-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { InstanceTableModule } from './instance-table/instance-table.module';

@NgModule({
  declarations: [DetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    BasicInfoModule,
    ProTableModule,
    InstanceTableModule
  ]
})
export class DetailModule {}
