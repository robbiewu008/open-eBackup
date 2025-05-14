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
from typing import List, Any

from pydantic import BaseModel, Field


class ResourceCatalogSchema(BaseModel):
    catalog_id: str = Field(None, description="目录ID")
    catalog_name: str = Field(None, description="目录名称")
    display_order: int = Field(None, description="显示顺序")
    show: bool = Field(None, description="是否隐藏")
    parent_id: str = Field(None, description="父目录ID")
    children: List[Any] = Field(None, description="子目录列表")
    label: str = Field(None, description="标签")
    link: str = Field(None, description="资源目录对应的URL")

    class Config:
        extra = 'allow'
        arbitrary_types_allowed = True
        schema_extra = {
            'example': {
                'catalog_id': '50dc3d15-21dc-4fcb-97c4-35581f489afc',
                'catalog_name': 'BigData',
                'display_order': 0,
                'show': True,
                'parent_id': None,
                'label': 'resource.catalog.BigData.label',
                'link': '',
                'children': [{
                    'parent_id': '50dc3d15-21dc-4fcb-97c4-35581f489afc',
                    'catalog_id': '60dc3d15-31dc-5fcb-07c4-45581f489afc',
                    'show': True,
                    'catalog_name': 'Hadoop',
                    'display_order': 0,
                    'label': 'Hadoop',
                    'link': '/resource/bigdata.html',
                }]
            }
        }
