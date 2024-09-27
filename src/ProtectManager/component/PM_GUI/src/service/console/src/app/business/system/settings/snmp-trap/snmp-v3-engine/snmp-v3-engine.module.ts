import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SnmpV3EngineComponent } from './snmp-v3-engine.component';

@NgModule({
  declarations: [SnmpV3EngineComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule]
})
export class SnmpV3EngineModule {}
