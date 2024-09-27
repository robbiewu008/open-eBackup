import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { OracleRoutingModule } from './oracle-routing.module';
import { OracleComponent } from '././oracle.component';
import { LiveMountsListModule } from '../live-mounts-list/live-mounts-list.module';

@NgModule({
  declarations: [OracleComponent],
  imports: [
    CommonModule,
    OracleRoutingModule,
    BaseModule,
    LiveMountsListModule
  ],
  exports: [OracleComponent]
})
export class OracleModule {}
