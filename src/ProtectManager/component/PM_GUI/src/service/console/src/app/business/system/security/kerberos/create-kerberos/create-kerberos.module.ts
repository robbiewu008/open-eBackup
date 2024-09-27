import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CreateKerberosComponent } from './create-kerberos.component';

@NgModule({
  declarations: [CreateKerberosComponent],
  imports: [CommonModule, BaseModule]
})
export class CreateKerberosModule {}
