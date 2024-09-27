import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AvailabiltyGroupComponent } from './availabilty-group.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { RegisterGroupModule } from './register-group/register-group.module';
import { DatabaseTemplateModule } from '../../database-template/database-template.module';
import { ExchangeDetailsModule } from './details/details.module';

@NgModule({
  declarations: [AvailabiltyGroupComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    RegisterGroupModule,
    DatabaseTemplateModule,
    ExchangeDetailsModule
  ],
  exports: [AvailabiltyGroupComponent]
})
export class AvailabiltyGroupModule {}
