import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { KerberosModule } from 'app/business/system/security/kerberos/kerberos.module';
import { BaseModule } from 'app/shared';
import { RegisterClusterComponent } from './register-cluster.component';

@NgModule({
  declarations: [RegisterClusterComponent],
  imports: [CommonModule, BaseModule, KerberosModule]
})
export class RegisterClusterModule {}
