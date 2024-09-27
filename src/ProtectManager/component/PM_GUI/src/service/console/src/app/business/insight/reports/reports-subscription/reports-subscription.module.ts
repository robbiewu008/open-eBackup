import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ReactiveFormsModule } from '@angular/forms';
import {
  ButtonModule,
  FormModule,
  GroupModule,
  IconModule,
  InputModule,
  OverflowModule,
  SearchModule
} from '@iux/live';
import { BaseModule } from 'app/shared';
import { StatusModule } from 'app/shared/components';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CreateSubscriptionModule } from './create-subscription/create-subscription.module';
import { ReportsSubscriptionComponent } from './reports-subscription.component';

@NgModule({
  declarations: [ReportsSubscriptionComponent],
  exports: [ReportsSubscriptionComponent],
  imports: [
    CommonModule,
    CreateSubscriptionModule,
    BaseModule,
    ButtonModule,
    FormModule,
    GroupModule,
    IconModule,
    InputModule,
    OverflowModule,
    ProButtonModule,
    ProTableModule,
    ReactiveFormsModule,
    SearchModule,
    StatusModule
  ]
})
export class ReportsSubscriptionModule {}
