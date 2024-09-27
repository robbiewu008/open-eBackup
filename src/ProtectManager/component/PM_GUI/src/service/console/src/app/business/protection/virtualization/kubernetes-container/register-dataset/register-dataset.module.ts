import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterDatasetComponent } from './register-dataset.component';
import { BaseModule } from 'app/shared';
import { MultiAutocompleteModule } from '@iux/live';

@NgModule({
  declarations: [RegisterDatasetComponent],
  imports: [CommonModule, BaseModule, MultiAutocompleteModule],
  exports: [RegisterDatasetComponent]
})
export class RegisterDatasetModule {}
