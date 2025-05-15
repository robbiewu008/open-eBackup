# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
from typing import TypeVar, Generic, List, Callable

from pydantic import Field, BaseModel
from pydantic.generics import GenericModel

T = TypeVar('T')
E = TypeVar('E')


class PageResponse(GenericModel, Generic[T]):
    """
    统一分页响应对象：新增接口统一使用此对象进行返回
    注意：不要在此类中增加任何函数，此类只做简单对象，不处理任何逻辑
    """
    total_count: int = Field(description="总的数据条数", alias="totalCount")
    records: List[T] = Field(description="数据列表")


class BasePage(GenericModel, Generic[T]):
    """
    统一分页响应对象：(已废弃)
    新增接口中不推荐使用此方法，新增接口中使用PageResponse进行分页
    """
    total: int = Field(description="总的数据条数")
    pages: int = Field(description="总页数")
    page_size: int = Field(description="每页数据量")
    page_no: int = Field(description="页面编号")
    items: List[T] = Field(description="数据列表")

    def map(self, mapper: Callable[[T], E]):
        return BasePage(total=self.total, pages=self.pages, page_size=self.page_size, page_no=self.page_no,
                        items=[mapper(item) for item in self.items])

    def one(self) -> T:
        items = self.items
        if not items:
            raise Exception("Not found object")
        if len(items) > 1:
            raise Exception("Multiple objects found")
        return items[0]


class PageRequest(BaseModel):
    page_no: int
    page_size: int


class BaseOrmModel(BaseModel):
    class Config:
        orm_mode = True


class UuidObject(BaseModel):
    uuid: str = Field(description="UUID")


class BatchOperationResult(BaseModel):
    all_ids: List = Field(description="操作成功id的列表", default=[])
    success_ids: List = Field(description="操作成功id的列表", default=[])
    failed_ids: List = Field(description="操作失败的id的列表", default=[])

    def append_success_id(self, success_id: str):
        self.success_ids.append(success_id)

    def append_success_id_list(self, success_ids: List):
        self.success_ids.extend(success_ids)

    def append_failed_id(self, failed_id: str):
        self.failed_ids.append(failed_id)

    def append_failed_id_list(self, failed_ids: List):
        self.failed_ids.extend(failed_ids)

    def merge_result(self, batch_result):
        self.success_ids.extend(batch_result.success_ids)
        self.failed_ids.extend(batch_result.failed_ids)

    def is_success(self) -> bool:
        return len(self.success_ids) > 0
