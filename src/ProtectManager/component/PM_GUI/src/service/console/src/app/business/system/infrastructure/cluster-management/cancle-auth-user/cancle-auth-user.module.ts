import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CancleAuthUserComponent } from './cancle-auth-user.component';
import { CancleWarnComponent } from './cancle-warn/cancle-warn.component';

@NgModule({
  declarations: [CancleAuthUserComponent, CancleWarnComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProStatusModule],
  exports: [CancleAuthUserComponent]
})
export class CancleAuthUserModule {}
