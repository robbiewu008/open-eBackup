import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { KingBaseRegisterComponent } from './king-base-register.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { KingBaseAddHostModule } from './add-host/king-base-add-host.module';

@NgModule({
  declarations: [KingBaseRegisterComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    KingBaseAddHostModule
  ]
})
export class KingBaseRegisterModule {}
