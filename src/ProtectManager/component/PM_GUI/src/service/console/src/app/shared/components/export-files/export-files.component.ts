import { Component, Injectable, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { Router } from '@angular/router';
import { MessageService, ModalService } from '@iux/live';
import {
  ApiExportFilesApiService as ExportFileApiService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  GlobalService,
  I18NService,
  MODAL_COMMON
} from 'app/shared';
import { each, isFunction, now, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

interface Options {
  content?: string;
  data?: Data;
  width?: number;
  height?: number;
  onCancel?: (modal: any) => void;
  onOK?: (modal: any) => void;
}

interface Data {
  params: any;
  type: string;
  tip?: string;
}

@Component({
  selector: 'aui-export-files',
  templateUrl: './export-files.component.html',
  styleUrls: ['./export-files.component.less']
})
export class ExportFilesComponent implements OnInit {
  data = {};
  content;
  formGroup: FormGroup;

  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [251])
  };

  constructor(
    public router: Router,
    private fb: FormBuilder,
    private i18n: I18NService,
    private message: MessageService,
    private globalService: GlobalService,
    public baseUtilService: BaseUtilService,
    private exportFilesApi: ExportFileApiService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    const time = new Date();
    const year = time.getFullYear();
    const month =
      time.getMonth() < 9 ? `0${time.getMonth() + 1}` : time.getMonth() + 1;
    const date = time.getDate() < 10 ? `0${time.getDate()}` : time.getDate();

    this.formGroup = this.fb.group({
      name: new FormControl(`file_${year}${month}${date}_${now()}`, {
        validators: [
          this.baseUtilService.VALID.name(
            this.i18n.isEn
              ? CommonConsts.REGEX.dataBaseName
              : CommonConsts.REGEX.nameCombination,
            true,
            'invalidNameCombination'
          ),
          this.baseUtilService.VALID.maxLength(251)
        ]
      })
    });
  }

  onOK(): Observable<void> {
    return new Observable<any>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const request = {
        ...this.data,
        name: trim(this.formGroup.value.name) || `${now()}`
      } as any;

      let memberEsn = '';
      if (
        request.type === DataMap.Export_Query_Type.log.value &&
        request.params.clusterNodeName
      ) {
        memberEsn = request.params.clusterNodeName;
        delete request.params.clusterNodeName;
      } else if (
        request.type === DataMap.Export_Query_Type.copy.value &&
        request.params.memberEsn
      ) {
        memberEsn = request.params.memberEsn;
        delete request.params.memberEsn;
      }
      this.exportFilesApi
        .CreateExportFile({
          request,
          akOperationTips: false,
          memberEsn: memberEsn
        })
        .subscribe(
          res => {
            this.message.success(
              this.i18n.get('common_export_files_result_label'),
              {
                lvShowCloseButton: true,
                lvDuration: this.isHyperdetect ? 10 * 1e3 : 60 * 1e3,
                lvOnShow: () => {
                  const exportFilesResult = document.getElementsByClassName(
                    'export-files-result'
                  );
                  if (exportFilesResult) {
                    each(exportFilesResult, item => {
                      item.addEventListener('click', () => {
                        this.router.navigate(['system/export-query']);
                      });
                    });
                  }
                }
              }
            );
            observer.next(res);
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
}

@Injectable({
  providedIn: 'root'
})
export class ExportFilesService {
  private exportFilesComponent = ExportFilesComponent;

  constructor(
    private drawModalService: ModalService,
    private i18n: I18NService
  ) {}

  create(options: Options) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvModalKey: 'exportFilesModal',
      ...{
        lvType: 'dialog',
        lvHeader: this.i18n.get('common_export_label'),
        lvContent: this.exportFilesComponent,
        lvComponentParams: {
          content: options.content,
          data: options.data
        },
        lvWidth: options.width || MODAL_COMMON.normalWidth,
        lvHeight: options.height || (options.data?.tip ? 350 : 300),
        lvOkType: 'primary',
        lvCancelType: 'default',
        lvOkDisabled: false,
        lvFocusButtonId: 'cancel',
        lvCloseButtonDisplay: true,
        lvAfterOpen: modal => {
          const component = modal.getContentComponent() as ExportFilesComponent;
          component.formGroup.statusChanges.subscribe(res => {
            modal.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const component = modal.getContentComponent() as ExportFilesComponent;
            component.onOK().subscribe(
              () => {
                if (isFunction(options.onOK)) {
                  options.onOK(modal);
                }
                resolve(true);
              },
              () => {
                resolve(false);
              }
            );
          });
        },
        lvCancel: modal => {
          if (isFunction(options.onCancel)) {
            options.onCancel(modal);
          }
        }
      }
    });
  }
}
