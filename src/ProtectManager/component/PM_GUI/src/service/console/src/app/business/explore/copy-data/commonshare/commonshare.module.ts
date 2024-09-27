import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CommonshareRoutingModule } from './commonshare-routing.module';
import { CommonshareComponent } from './commonshare.component';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [CommonshareComponent],
  imports: [
    BaseModule,
    CommonModule,
    CommonshareRoutingModule,
    CopyResourceListModule
  ]
})
export class CommonshareModule {}
