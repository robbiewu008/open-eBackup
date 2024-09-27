import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnInit,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  DatatableComponent,
  ModalRef,
  TreeComponent,
  TreeNode
} from '@iux/live';
import {
  CommonConsts,
  CopyControllerService,
  I18NService,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import {
  FileSystemResponse,
  FileSystemResponseList
} from 'app/shared/api/models';
import { RestoreApiV2Service } from 'app/shared/api/services';
import { TableConfig } from 'app/shared/components/pro-table';
import {
  cloneDeep,
  find,
  isArray,
  isEmpty,
  isString,
  isUndefined,
  map,
  reject,
  remove,
  size,
  uniqueId
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-object-level-restore',
  templateUrl: './object-level-restore.component.html',
  styleUrls: ['./object-level-restore.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class ObjectLevelRestoreComponent implements OnInit {
  resourceData;
  properties;
  instanceOptions = [];
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  tableConfig: TableConfig;
  tableData = [];
  treeData = [];
  treeSelection = [];
  tableSelection = [];
  pageSizeOptions = [100, 200, 500];
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @ViewChild('lvTable', { static: false }) lvTable: DatatableComponent;
  @ViewChild('lvTree', { static: false }) lvTree: TreeComponent;
  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private restoreV2Service: RestoreApiV2Service,
    private copyControllerService: CopyControllerService
  ) {}

  ngOnInit() {
    this.formatCopyData();
    this.initForm();
    this.queryTreeData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      location: new FormControl({
        value: this.resourceData?.name,
        disabled: true
      })
    });
    this.modal.getInstance().lvOkDisabled = true;
  }

  formatCopyData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.properties = isString(this.rowCopy.properties)
      ? JSON.parse(this.rowCopy.properties)
      : {};
  }

  trackByUuid(index, item) {
    return item.uuid;
  }

  queryTreeData(node?, page?: number) {
    const targetPath = isUndefined(node)
      ? `/${this.properties.parent_path}`
      : `${node.parentPath}/${node.path}`;
    this.copyControllerService
      .ListCopyCatalogs({
        pageNo: page || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        copyId: this.rowCopy.uuid,
        parentPath: targetPath
      })
      .subscribe((res: FileSystemResponseList) => {
        const treeNodes: TreeNode[] = map(
          res.records,
          (item: FileSystemResponse) => {
            return {
              ...item,
              key: item['path'],
              label: item['path'],
              isLeaf: false, // 暂时改为都有子节点，后续修改
              uuid: uniqueId(),
              children: [],
              parentPath: targetPath
            };
          }
        );
        this.appendTreeNode(treeNodes, res.totalCount, node);
      });
  }

  expandTreeNode(node: TreeNode) {
    if (isEmpty(node.children)) {
      this.queryTreeData(node);
    }
  }

  treeSelectionChange(tree) {
    this.tableData = cloneDeep(this.treeSelection);
    this.setValid();
    this.cdr.detectChanges();
  }

  setValid() {
    this.modal.getInstance().lvOkDisabled = isEmpty(this.tableData);
  }

  deleteItem(e, data) {
    this.lvTree.deleteSelection([data]);
    this.cdr.detectChanges();
  }

  clearSelection() {
    this.lvTree.clearSelection();
    this.cdr.detectChanges();
  }

  appendTreeNode(nodes: TreeNode[], total, node?) {
    if (isUndefined(node)) {
      this.treeData = nodes;
      this.cdr.detectChanges();
      return;
    }
    if (isArray(node.children) && !isEmpty(node.children)) {
      node.children = [
        ...reject(node.children, n => {
          return n.isMoreBtn;
        }),
        ...nodes
      ];
    } else {
      node.children = [...nodes];
    }
    if (total > size(node.children)) {
      const moreClickNode = {
        label: `${this.i18n.get('common_more_label')}...`,
        uuid: uniqueId(),
        isMoreBtn: true,
        isLeaf: true,
        disabled: true,
        startPage: Math.floor(size(node.children) / CommonConsts.PAGE_SIZE_MAX)
      };
      node.children = [...node.children, moreClickNode];
    }
    this.treeData = [...this.treeData];
    if (find(this.treeSelection, node)) {
      this.treeSelection = [...this.treeSelection, ...nodes];
      this.tableData = cloneDeep(this.treeSelection);
    }
    this.treeSelection = [...this.treeSelection];
    this.cdr.detectChanges();
  }

  treeSearch(value) {
    this.lvTable.filter({ key: 'label', value, filterMode: 'contains' });
  }

  getParams() {
    const objectInfo = map(this.tableData, 'extendInfo');
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.resourceData?.environment_uuid || this.resourceData?.root_uuid,
      restoreType: RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject: this.resourceData?.uuid,
      extendInfo: {
        objectInfo: JSON.stringify(objectInfo)
      }
    };
    return params;
  }

  restore(): Observable<void> {
    if (isEmpty(this.tableData)) {
      return;
    }
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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
    });
  }
}
