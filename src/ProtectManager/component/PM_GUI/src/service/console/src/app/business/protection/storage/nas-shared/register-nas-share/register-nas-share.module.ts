import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';
import { RegisterNasShareComponent } from './register-nas-share.component';

@NgModule({
  declarations: [RegisterNasShareComponent],
  imports: [CommonModule, BaseModule, ResourceFilterModule]
})
export class RegisterNasShareModule {}
