import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { IpConfigListComponent } from './ip-config-list.component';

@NgModule({
  declarations: [IpConfigListComponent],
  imports: [CommonModule, BaseModule],
  exports: [IpConfigListComponent]
})
export class IpConfigListModule {}
