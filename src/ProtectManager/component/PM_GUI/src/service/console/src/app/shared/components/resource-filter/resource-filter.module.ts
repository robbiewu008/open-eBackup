import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AutocompleteModule } from '@iux/live';
import { BaseModule } from 'app/shared/base.module';
import { ResourceFilterComponent } from './resource-filter.component';

@NgModule({
  declarations: [ResourceFilterComponent],
  imports: [CommonModule, BaseModule, AutocompleteModule],
  exports: [ResourceFilterComponent]
})
export class ResourceFilterModule {}
