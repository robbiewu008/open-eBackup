import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProButtonComponent } from './pro-button.component';
import { ProButtonGroupComponent } from './pro-button-group.component';
import {
  ButtonModule,
  GroupModule,
  IconModule,
  TooltipModule,
  DropdownModule,
  PopoverModule
} from '@iux/live';
import { FormsModule } from '@angular/forms';

@NgModule({
  declarations: [ProButtonComponent, ProButtonGroupComponent],
  imports: [
    CommonModule,
    FormsModule,
    ButtonModule,
    GroupModule,
    IconModule,
    TooltipModule,
    DropdownModule,
    PopoverModule
  ],
  exports: [ProButtonComponent, ProButtonGroupComponent]
})
export class ProButtonModule {}
