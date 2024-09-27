import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { FilesetModule } from '../fileset.module';
import { FilesetTemplateListRoutingModule } from './fileset-template-list-routing.module';
import { FilesetTemplateListComponent } from './fileset-template-list.component';
import { TemplateListModule } from './template-list/template-list.module';

@NgModule({
  declarations: [FilesetTemplateListComponent],
  imports: [
    CommonModule,
    FilesetTemplateListRoutingModule,
    BaseModule,
    FilesetModule,
    TemplateListModule,
    MultiClusterSwitchModule
  ]
})
export class FilesetTemplateListModule {}
