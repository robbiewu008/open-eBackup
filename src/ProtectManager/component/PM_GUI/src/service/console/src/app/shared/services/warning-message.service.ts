import { Injectable, TemplateRef } from '@angular/core';
import { ModalService } from '@iux/live';
import { WarningComponent } from '../components/warning.component';
import { MODAL_COMMON } from '../consts/live.const';
import { I18NService } from './i18n.service';

interface Options {
  content: string | TemplateRef<void>;
  okText?: string;
  onOK: () => void;
  width?: number;
  onCancel?: () => void;
  lvAfterClose?: (rseult: any) => void;
  header?: string;
}

@Injectable({
  providedIn: 'root'
})
export class WarningMessageService {
  private warningComponent = WarningComponent;

  constructor(
    private drawModalService: ModalService,
    private i18n: I18NService
  ) {}

  create(options: Options) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'warningMessage',
      ...{
        lvType: 'dialog',
        lvDialogIcon: 'lv-icon-popup-danger-48',
        lvHeader: options.header || this.i18n.get('common_danger_label'),
        lvContent: this.warningComponent,
        lvComponentParams: {
          content: options.content
        },
        lvWidth: options.width || MODAL_COMMON.normalWidth,
        lvOkType: 'primary',
        lvOkText: options.okText || this.i18n.get('common_ok_label'),
        lvCancelType: 'default',
        lvOkDisabled: true,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {
          const component = modal.getContentComponent() as WarningComponent;
          const modalIns = modal.getInstance();
          component.isChecked$.subscribe(e => {
            modalIns.lvOkDisabled = !e;
          });
        },
        lvCancel: modal => {
          if (options.onCancel) {
            options.onCancel();
          }
        },
        lvOk: modal => options.onOK(),
        lvAfterClose: result => {
          if (options.lvAfterClose) {
            options.lvAfterClose(result);
          }
        }
      }
    });
  }
}
