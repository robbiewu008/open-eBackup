import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { OracleRoutingModule } from './oracle-routing.module';
import { OracleComponent } from './oracle.component';
import { DataDesensitizationListModule } from '../data-desensitization-list/data-desensitization-list.module';

@NgModule({
  declarations: [OracleComponent],
  imports: [
    CommonModule,
    OracleRoutingModule,
    BaseModule,
    DataDesensitizationListModule
  ]
})
export class OracleModule {}
