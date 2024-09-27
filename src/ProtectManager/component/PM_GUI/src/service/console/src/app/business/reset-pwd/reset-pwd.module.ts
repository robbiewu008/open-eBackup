import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ResetPwdRoutingModule } from './reset-pwd-routing.module';
import { ResetPwdComponent } from './reset-pwd.component';
import { LogoTitleModule } from 'app/shared/components/logo-title/logo-title.module';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ResetPwdComponent],
  imports: [CommonModule, BaseModule, LogoTitleModule, ResetPwdRoutingModule]
})
export class ResetPwdModule {}
