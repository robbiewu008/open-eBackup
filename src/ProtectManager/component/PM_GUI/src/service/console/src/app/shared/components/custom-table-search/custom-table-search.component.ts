import {
  Component,
  ElementRef,
  EventEmitter,
  Input,
  Output,
  ViewChild
} from '@angular/core';
import { trim } from 'lodash';

@Component({
  selector: 'aui-custom-table-search',
  templateUrl: './custom-table-search.component.html',
  styleUrls: ['./custom-table-search.component.less']
})
export class CustomTableSearchComponent {
  @Input() filterTitle: string;
  @Output() search = new EventEmitter<any>();

  nameFilterValue: string;
  nameFilterTrueValue: string;
  nameSearchPanelVisible = false;
  @ViewChild('primaryButton', { read: ElementRef }) primaryButton: ElementRef<
    HTMLElement
  >;
  @ViewChild('nameFilterInput') nameFilterInput: ElementRef<HTMLElement>;

  popoverBeforeOpen = () => {
    this.nameFilterValue = this.nameFilterTrueValue;
  };

  openNameSearchPanel() {
    this.nameSearchPanelVisible = !this.nameSearchPanelVisible;
    setTimeout(() => {
      this.nameFilterInput?.nativeElement?.focus();
    }, 300);
  }

  confirmName(e: KeyboardEvent) {
    if (e.code === 'NumpadEnter') {
      this.primaryButton.nativeElement.click();
    }
  }

  reset() {
    this.nameFilterValue = '';
    this.filterByName(this.nameFilterValue);
    this.nameSearchPanelVisible = false;
  }

  filterByName(value: string) {
    this.nameFilterTrueValue = value;
    if (value === null || value === undefined) {
      return;
    }
    this.search.emit(trim(value));
    this.nameSearchPanelVisible = false;
  }
}
