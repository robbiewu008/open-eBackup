import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateSchemaComponent } from './create-schema.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';

@NgModule({
  declarations: [CreateSchemaComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ]
})
export class CreateSchemaModule {}
