import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { CreateCommonShareComponent } from './create-commonshare.component';

@NgModule({
  declarations: [CreateCommonShareComponent],
  imports: [CommonModule, BaseModule, AlertModule]
})
export class RegisterCommonShareModule {}
