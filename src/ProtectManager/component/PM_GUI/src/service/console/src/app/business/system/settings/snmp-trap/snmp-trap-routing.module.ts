import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { SnmpTrapComponent } from './snmp-trap.component';

const routes: Routes = [{ path: '', component: SnmpTrapComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SnmpTrapRoutingModule {}
