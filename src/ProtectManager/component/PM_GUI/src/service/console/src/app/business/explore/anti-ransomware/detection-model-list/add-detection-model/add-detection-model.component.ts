import { Component, OnInit } from '@angular/core';
import { FormGroup, FormBuilder } from '@angular/forms';
import { Subject, Observable, Observer } from 'rxjs';
import {
  I18NService,
  DataMapService,
  WarningMessageService,
  BaseUtilService,
  ModelManagementService
} from 'app/shared';
import { MessageService, UploadFile } from '@iux/live';
import { InfoMessageService } from 'app/shared/services/info-message.service';

@Component({
  selector: 'aui-add-detection-model',
  templateUrl: './add-detection-model.component.html',
  styleUrls: ['./add-detection-model.component.less']
})
export class AddDetectionModelComponent implements OnInit {
  selectFile;
  filters = [];
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  modelFileLabel = this.i18n.get('explore_model_file_label');

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public message: MessageService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private infoMessageService: InfoMessageService,
    private warningMessageService: WarningMessageService,
    private modelManagementService: ModelManagementService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.filters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['zip', 'tgz'];
          let validFiles = files.filter(file => {
            let suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['zip/tgz']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.valid$.next(false);
            return;
          }
          if (files[0].size > 1024 * 1024 * 10) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['10MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.valid$.next(false);
            return;
          }
          this.valid$.next(true);
          return validFiles;
        }
      }
    ];
  }

  uploadChange(e) {
    if (e.activeFiles && e.activeFiles.length) {
      this.selectFile = e.activeFiles[0].originFile;
      this.valid$.next(true);
    } else {
      this.valid$.next(false);
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.i18n.get('explore_upload_anti_model_label'),
        onOK: () => {
          this.modelManagementService
            .addModelInfoUsingPOST({
              modelFile: this.selectFile
            })
            .subscribe(
              res => {
                observer.next();
                observer.complete();
              },
              err => {
                observer.error(err);
                observer.complete();
              }
            );
        },
        onCancel: () => {
          observer.error('');
          observer.complete();
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            observer.error(null);
            observer.complete();
          }
        }
      });
    });
  }
}
