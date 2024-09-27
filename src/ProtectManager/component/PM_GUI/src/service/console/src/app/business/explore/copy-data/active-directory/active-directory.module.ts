import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ActiveDirectoryComponent } from './active-directory.component';
import { ActiveDirectoryRoutingModule } from './active-directory-routing.module';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [ActiveDirectoryComponent],
  imports: [
    CommonModule,
    BaseModule,
    CopyResourceListModule,
    ActiveDirectoryRoutingModule
  ]
})
export class ActiveDirectoryModule {}
