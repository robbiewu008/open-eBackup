import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LocalResourceComponent } from './local-resource.component';
import { BaseModule } from 'app/shared';
import { LocalFileSystemModule } from '../local-file-system/local-file-system.module';
import { LocalLunModule } from '../local-lun/local-lun.module';
import { LocalResourceRoutingModule } from './local-resource-routing.module';

@NgModule({
  declarations: [LocalResourceComponent],
  imports: [
    CommonModule,
    BaseModule,
    LocalResourceRoutingModule,
    LocalFileSystemModule,
    LocalLunModule
  ]
})
export class LocalResourceModule {}
