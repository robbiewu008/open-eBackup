import { NgModule } from '@angular/core';
import { AddTelnetComponent } from './add-telnet.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { AddUserModule } from './add-user/add-user.module';

@NgModule({
  declarations: [AddTelnetComponent],
  imports: [
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    AddUserModule
  ]
})
export class AddTelnetComputeModule {}
