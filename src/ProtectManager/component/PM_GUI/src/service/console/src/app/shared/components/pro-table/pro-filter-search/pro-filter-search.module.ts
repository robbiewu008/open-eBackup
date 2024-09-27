import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { ProFilterSearchComponent } from './pro-filter-search.component';
import { PopoverModule, IconModule, SearchModule } from '@iux/live';

@NgModule({
  declarations: [ProFilterSearchComponent],
  imports: [CommonModule, FormsModule, PopoverModule, IconModule, SearchModule],
  exports: [ProFilterSearchComponent]
})
export class ProFilterSearchModule {}
