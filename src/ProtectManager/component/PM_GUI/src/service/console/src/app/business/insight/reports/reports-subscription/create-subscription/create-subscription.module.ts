import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ReactiveFormsModule } from '@angular/forms';
import {
  DatePickerModule,
  FormModule,
  GroupModule,
  IconModule,
  InputModule,
  InputTagModule,
  SelectModule,
  TreeSelectModule
} from '@iux/live';
import { BaseModule } from 'app/shared';
import { CreateSubscriptionComponent } from './create-subscription.component';

@NgModule({
  declarations: [CreateSubscriptionComponent],
  imports: [
    CommonModule,
    BaseModule,
    DatePickerModule,
    FormModule,
    GroupModule,
    IconModule,
    InputModule,
    ReactiveFormsModule,
    SelectModule,
    TreeSelectModule,
    InputTagModule
  ]
})
export class CreateSubscriptionModule {}
