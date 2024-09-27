import { NgModule } from '@angular/core';
import { LoginComponent } from './login.component';
import { LoginRouterModule } from './login-router.module';
import { LogoTitleModule } from 'app/shared/components/logo-title/logo-title.module';
import { BaseModule } from 'app/shared';
import { AlertModule } from '@iux/live';

@NgModule({
  imports: [LoginRouterModule, BaseModule, LogoTitleModule, AlertModule],
  declarations: [LoginComponent]
})
export class LoginModule {}
