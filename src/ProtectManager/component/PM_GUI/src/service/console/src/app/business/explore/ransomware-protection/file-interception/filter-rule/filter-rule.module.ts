import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FilterRuleComponent } from './filter-rule.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { CreateFilterRuleModule } from './create-filter-rule/create-filter-rule.module';
import { AssociateFileSystemModule } from './associate-file-system/associate-file-system.module';
import { FileSystemNumModule } from './file-system-num/file-system-num.module';

@NgModule({
  declarations: [FilterRuleComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    CreateFilterRuleModule,
    AssociateFileSystemModule,
    FileSystemNumModule
  ],
  exports: [FilterRuleComponent]
})
export class FilterRuleModule {}
