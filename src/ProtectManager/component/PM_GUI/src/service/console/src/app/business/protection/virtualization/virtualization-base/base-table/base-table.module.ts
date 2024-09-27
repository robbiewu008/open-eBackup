import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseTableComponent } from './base-table.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { CreateGroupModule } from '../../virtualization-group/create-group/create-group.module';

@NgModule({
  declarations: [BaseTableComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    CreateGroupModule
  ],
  exports: [BaseTableComponent]
})
export class BaseTableModule {}
