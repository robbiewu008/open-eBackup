import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DoradoFileSystemRoutingModule } from './dorado-file-system-routing.module';
import { DoradoFileSystemComponent } from './dorado-file-system.component';
import { LiveMountsListModule } from '../live-mounts-list/live-mounts-list.module';

@NgModule({
  declarations: [DoradoFileSystemComponent],
  imports: [
    CommonModule,
    DoradoFileSystemRoutingModule,
    BaseModule,
    LiveMountsListModule
  ]
})
export class DoradoFileSystemModule {}
