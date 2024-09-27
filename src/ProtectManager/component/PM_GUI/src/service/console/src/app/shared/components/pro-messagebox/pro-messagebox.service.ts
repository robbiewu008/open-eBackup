import { Injectable } from '@angular/core';
import { doFnResult, I18NService, LvConfig, ModalService } from '@iux/live';
import { Observable } from 'rxjs';
import { ProMessageboxOptions } from './interface';
import { ProMessageboxComponent } from './pro-messagebox.component';

@Injectable({ providedIn: 'root' })
export class ProMessageboxService {
  constructor(private modalService: ModalService, private i18n: I18NService) {}

  private iconMap = {
    question: 'lv-icon-popup-question-48',
    info: 'lv-icon-popup-info-48',
    success: 'lv-icon-popup-success-48',
    error: 'lv-icon-popup-error-48',
    warning: 'lv-icon-popup-warning-48',
    danger: 'lv-icon-popup-danger-48'
  };
  private titleMap = {
    confirm: this.i18n.get('msgBoxConfirm'),
    prompt: this.i18n.get('msgBoxInfo')
  };

  confirm(options: ProMessageboxOptions) {
    return this._create(options, 'confirm');
  }

  prompt(options: ProMessageboxOptions) {
    return this._create(options, 'prompt');
  }

  private _create(options: ProMessageboxOptions, type: string) {
    const buttonEvent = (modal, button, buttonOpts) => {
      const triggerFn = buttonOpts?.onClick;
      const fn = () => {
        modal.close({ trigger: type });
      };

      if (!triggerFn) {
        fn();
      } else {
        const call: any = triggerFn(this);
        if (call instanceof Promise || call instanceof Observable) {
          buttonOpts?.showLoading !== false && (button.loading = true);
        }
        doFnResult(call, result => {
          button.loading = false;
          if (result) {
            fn();
          }
        });
      }
    };
    let buttons = [
      {
        id: type === 'confirm' ? 'ok' : 'close',
        label:
          type === 'confirm' ? this.i18n.get('ok') : this.i18n.get('close'),
        type: 'default',
        disabled: options.okButton?.disabled || false,
        loadingText:
          options.okButton?.loadingText || this.i18n.get('loadingText'),
        onClick: (modal, button) => {
          buttonEvent(modal, button, options.okButton);
        }
      },
      {
        id: 'cancel',
        label: this.i18n.get('cancel'),
        type: 'primary',
        disabled: options.cancelButton?.disabled || false,
        loadingText:
          options.cancelButton?.loadingText || this.i18n.get('loadingText'),
        onClick: (modal, button) => {
          buttonEvent(modal, button, options.cancelButton);
        }
      }
    ];

    let footerButtons =
      options.customButtons || (type === 'confirm' ? buttons : [buttons[0]]);
    let _options = {
      lvType: 'dialog',
      lvVisible: false,
      lvModality: true,
      lvModalKey: options.modalKey,
      lvClass: options.panelClass,
      lvWidth: options.width || 400,
      lvHeight: options.height || 'auto',
      lvHeader: options.title ? options.title : this.titleMap[type],
      lvDialogIcon: options.icon
        ? this.iconMap[options.icon]
        : this.iconMap.info,
      lvCloseButtonDisplay: options.closeButtonDisplay || true,
      lvContent: ProMessageboxComponent,
      lvComponentParams: {
        data: options.content
      },
      lvBeforeOpen: options.beforeOpen,
      lvAfterOpen: options.afterOpen,
      lvBeforeClose: options.beforeClose,
      lvAfterClose: options.afterClose,
      lvFooter: footerButtons
    };

    _options = {
      ...{
        lvKeyboardEscape: false,
        lvOuterClosable: false,
        lvMaskTransparent: false
      },
      ...LvConfig.messageBoxOptions,
      ..._options
    };

    return this.modalService.create(_options);
  }
}
