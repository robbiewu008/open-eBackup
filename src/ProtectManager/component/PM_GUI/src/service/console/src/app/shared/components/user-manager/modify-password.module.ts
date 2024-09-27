import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import {
  FormModule,
  IconModule,
  InputModule,
  OverlaysModule,
  TooltipModule
} from '@iux/live';
import { BaseModule } from '../../base.module';
import { ModifyPasswordComponent } from './modify-password.component';

@NgModule({
  declarations: [ModifyPasswordComponent],
  imports: [
    CommonModule,
    IconModule,
    OverlaysModule,
    BaseModule,
    FormModule,
    FormsModule,
    TooltipModule,
    InputModule
  ],

  exports: [ModifyPasswordComponent]
})
export class ModifyPasswordModule {}
