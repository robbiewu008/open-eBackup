import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { InformixRoutingModule } from './informix-routing.module';
import { InformixComponent } from './informix.component';

@NgModule({
  declarations: [InformixComponent],
  imports: [
    CommonModule,
    InformixRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class InformixModule {}
