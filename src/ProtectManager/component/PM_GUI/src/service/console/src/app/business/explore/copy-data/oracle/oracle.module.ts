import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { OracleRoutingModule } from './oracle-routing.module';
import { OracleComponent } from './oracle.component';

@NgModule({
  declarations: [OracleComponent],
  imports: [
    CommonModule,
    OracleRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class OracleModule {}
