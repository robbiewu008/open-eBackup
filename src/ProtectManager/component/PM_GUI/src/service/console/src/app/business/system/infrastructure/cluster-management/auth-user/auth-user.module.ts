import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AuthUserComponent } from './auth-user.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [AuthUserComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ],
  exports: [AuthUserComponent]
})
export class AuthUserModule {}
