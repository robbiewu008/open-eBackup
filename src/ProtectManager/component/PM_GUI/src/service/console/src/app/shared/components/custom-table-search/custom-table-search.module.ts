import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CustomTableSearchComponent } from './custom-table-search.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [CustomTableSearchComponent],
  imports: [CommonModule, BaseModule],
  exports: [CustomTableSearchComponent]
})
export class CustomTableSearchModule {}
