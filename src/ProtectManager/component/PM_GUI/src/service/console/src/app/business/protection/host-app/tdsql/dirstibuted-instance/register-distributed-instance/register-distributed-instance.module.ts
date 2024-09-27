import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterDistributedInstanceComponent } from 'app/business/protection/host-app/tdsql/dirstibuted-instance/register-distributed-instance/register-distributed-instance.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [RegisterDistributedInstanceComponent],
  imports: [CommonModule, BaseModule, ProButtonModule, ProTableModule],
  exports: [RegisterDistributedInstanceComponent]
})
export class RegisterDistributedInstanceModule {}
