import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FileInterceptionRoutingModule } from './file-interception-routing.module';
import { FileInterceptionComponent } from './file-interception.component';
import { BaseModule } from 'app/shared';
import { FileSystemModule } from './file-system/file-system.module';
import { FilterRuleModule } from './filter-rule/filter-rule.module';

@NgModule({
  declarations: [FileInterceptionComponent],
  imports: [
    CommonModule,
    FileInterceptionRoutingModule,
    BaseModule,
    FileSystemModule,
    FilterRuleModule
  ],
  exports: [FileInterceptionComponent]
})
export class FileInterceptionModule {}
