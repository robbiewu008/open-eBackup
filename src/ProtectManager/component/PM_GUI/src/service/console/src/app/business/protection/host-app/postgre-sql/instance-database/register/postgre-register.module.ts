import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PostgreRegisterComponent } from './postgre-register.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { PostgreAddHostModule } from './add-host/postgre-add-host.module';

@NgModule({
  declarations: [PostgreRegisterComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    PostgreAddHostModule
  ]
})
export class PostgreRegisterModule {}
