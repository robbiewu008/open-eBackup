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
import json
import re
import uuid
from inspect import signature
from typing import List, Dict, Type, TypeVar, Union, Iterable, Callable, Sized

from fastapi import Query, APIRouter, Header
from pydantic import BaseModel
from sqlalchemy.dialects.postgresql import ARRAY, UUID
from sqlalchemy.ext.declarative import DeclarativeMeta
from sqlalchemy.orm import RelationshipProperty, Mapper
from sqlalchemy.orm.attributes import InstrumentedAttribute

from app.common.constants.constant import RBACConstants
from app.common.database import Database
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.exter_attack import exter_attack
from app.common.schemas.common_schemas import BasePage
from app.common.security.jwt_utils import get_user_info_from_token
from app.common.security.right_control import right_control
from app.common.toolkit import combine_multiple_method_as_chain_method
from app.common.concurrency import async_depend
from app.resource.service.common import resource_service

T = TypeVar('T')


def is_instrumented_attribute(data):
    if not data:
        return False
    if not isinstance(data, InstrumentedAttribute):
        return False
    attribute: InstrumentedAttribute = data
    return attribute.is_attribute


def order_resolver(model: DeclarativeMeta, *fields: str, auto_load: bool = True, extra_fields: List[str] = None,
                   description=None):
    fields = get_valid_order_fields(model, fields, auto_load, extra_fields)
    pattern = re.compile("^[+-]")
    description = description if description else "排序字段"

    def validate(order: str):
        return order is not None and pattern.match(order) and order[1:] in fields

    def resolver(orders: List[str] = Query(None, description=description)) -> List[str]:
        if not orders:
            return []
        orders = [order for order in orders if order is not None and len(order) > 1]
        orders = [order if order[0] != " " else f"+{order[1:]}" for order in orders]
        return [order for order in orders if validate(order)] if orders is not None else []

    return resolver


def get_fields_config(model: DeclarativeMeta, name: str):
    value = getattr(model, name, None)
    if value is None:
        return []
    if isinstance(value, Iterable):
        return list(value)
    if not isinstance(value, list):
        return [value]
    return [field for field in value if field is not None]


def get_valid_order_fields(model: DeclarativeMeta, fields, auto_load: bool = True, extra_fields: List[str] = None):
    extra_fields = extra_fields if extra_fields else []
    if (fields is None or len(fields) == 0) and auto_load:
        fields = get_fields_config(model, '__order_fields__')
        for child in model.__subclasses__():
            if child is DeclarativeMeta:
                fields = fields + get_valid_order_fields(child, None, True, extra_fields)
    return [field for field in fields if is_order_field(model, field, extra_fields)]


def is_order_field(model: DeclarativeMeta, field: str, extra_fields: List[str]):
    if extra_fields and field in extra_fields:
        return True
    return is_instrumented_attribute(getattr(model, field))


def get_field_from_alias_config(alias):
    if not alias:
        return []
    reg = re.compile(r'^%|%$')
    return [(reg.sub('', key), key) for key in alias.keys()]


def filter_resolver(model: DeclarativeMeta, *fields: str, auto_load: bool = True, extras: List[str] = None,
                    relations=None, description=None):
    mapping = get_filter_field_config(model, fields, auto_load)
    extras = extras if extras else []
    relation_field_list = get_relation_field_list(relations, "alias", get_field_from_alias_config)
    relation_alias_patterns = dict([relation_field for relation_field in relation_field_list if relation_field])
    extras = extras + list(relation_alias_patterns.keys())
    mapping.update(relation_alias_patterns)
    description = description if description else "条件参数"

    def resolver(conditions: str = Query(None, description=description)) -> Dict[str, any]:
        result = {}
        if conditions is None or len(conditions) == 0:
            return result
        data = json.loads(conditions)
        for field in data:
            if f"!{field}" in fields:
                continue
            pattern = mapping[field] if field in mapping else field
            attr = getattr(model, field, None)
            valid = is_instrumented_attribute(attr) or field in extras
            if not valid:
                continue
            value = data[field]
            result[pattern] = value
        return result

    return resolver


def get_filter_field_config(model: DeclarativeMeta, fields, auto_load: bool):
    fields = fields if fields and len(fields) > 0 else get_filter_fields(model, auto_load)
    mapping = {}
    for field in fields:
        if field.startswith('!'):
            continue
        name = re.compile(r'^%|%$').sub('', field)
        if getattr(model, name, None) is None:
            continue
        mapping[name] = field
    return mapping


def get_filter_fields(model: DeclarativeMeta, auto_load: bool):
    fields = get_fields_config(model, "__filter_fields__")
    if auto_load:
        for child in model.__subclasses__():
            fields = fields + get_filter_fields(child, True)
    return fields


def to_orm(schema: Union[BaseModel, dict], orm_type: Type[T]) -> T:
    data: dict = schema.dict() if isinstance(schema, BaseModel) else schema
    relations = stripping_relations(data, orm_type)
    entity = orm_type(**data)
    table = orm_type.__table__
    columns = table.primary_key.columns
    if columns:
        columns = [column for column in columns]
        if len(columns) == 1:
            name = columns[0].name
        else:
            name = getattr(orm_type, "__primary__", None)
        if getattr(entity, name) is None:
            primary = str(uuid.uuid4())
            setattr(entity, name, primary)
    cast_relations_to_orm(entity, relations, orm_type)
    return entity


def stripping_relations(data: Dict[str, any], orm_type: type):
    relations = {}
    for key in data:
        value = data[key]
        field = getattr(orm_type, key)
        if is_relationship_property(field):
            relations[key] = value
    for key in relations:
        del data[key]
    return relations


BUILTIN_TYPE_MAPPINGS = {ARRAY: List, UUID: str}


def is_relationship_property(attribute: InstrumentedAttribute):
    return isinstance(attribute.property, RelationshipProperty)


def get_instrumented_attribute_type(attribute: InstrumentedAttribute):
    attr_type = attribute.property.columns[0].type
    module_name: str = attr_type.__class__.__module__
    builtin: bool = module_name.startswith("sqlalchemy.dialects.")
    if builtin:
        attr_type = getattr(attr_type, "python_type")
    if isinstance(attr_type, DeclarativeMeta):
        return getattr(attr_type, '__schema__')
    return BUILTIN_TYPE_MAPPINGS[attr_type] if attr_type in BUILTIN_TYPE_MAPPINGS else attr_type


def cast_relations_to_orm(entity: T, relations: Dict[str, any], orm_type: Type[T]):
    for key in relations:
        value = relations[key]
        attr: InstrumentedAttribute = getattr(orm_type, key)
        prop: RelationshipProperty = attr.property
        mapper: Mapper = prop.entity
        entity_type = mapper.entity
        if isinstance(value, Iterable):
            data = [to_orm(item, entity_type) for item in value]
            setattr(entity, key, data)
        elif value is not None:
            data = to_orm(value, entity_type)
            setattr(entity, key, data)


def get_all_model_types(base: type, with_self: bool = False):
    abstract = getattr(base, "__abstract__", False)
    collection = [] if abstract or not with_self else [base]
    children = base.__subclasses__()
    if len(children) == 0:
        return collection
    for child in children:
        collection = collection + [child] + get_all_model_types(child)
    return collection


@exter_attack
def define_page_query_api(router: APIRouter, database: Database, table_type: type, path: str = None,
                          initiator: Callable[[Dict[str, any]], Callable[[Query], Query]] = None,
                          extra_conditions: List[str] = None,
                          roles=None, authenticate=None, excludes=None, converter_response=None):
    model_types = get_all_model_types(table_type)
    excludes = [] if excludes is None else excludes if isinstance(excludes, (tuple, list, set)) else [excludes]
    apis = []
    table_name_summary_map = {
        "ASMINFO": "查询Oracle ASM列表",
        "DATABASE_INSTANCE": "查询GaussDB实例列表",
        "DATABASES": "查询Oracle数据库列表",
        "FILESETS": "查询文件集列表",
        "HYPERV_RESOURCE": "查询Hyper-V资源列表",
        "REGISTER_DATABASES": "查询已注册的数据库列表",
        "VIRTUAL_RESOURCE": "查询VMware资源列表",
    }
    for model_type in model_types:
        if model_type in excludes:
            continue
        table_name = getattr(model_type, '__tablename__') if hasattr(model_type, '__tablename__') else None
        summary = table_name_summary_map.get(table_name, None)
        api = define_page_query_api_for_model(router, database, model_type, path=path, initiator=initiator,
                                              extra_conditions=extra_conditions, roles=roles, summary=summary,
                                              authenticate=authenticate, converter_response=converter_response)
        if api:
            apis.append(api)
    return apis


def get_all_relation_map_config(relations, name):
    result = {}
    if not relations:
        return result
    for relation in relations:
        config = relation.get(name)
        if config:
            result.update(config)
    return result


def get_model_paginate_initiator(relations, initiators: list = None):
    relations = relations if relations else []
    initiators = initiators if initiators else []
    initiators = initiators + [paginate.get("initiator", None) for paginate in relations]
    initiators = [initiator for initiator in initiators if initiator]
    return combine_initiators(initiators)


def combine_initiators(initiators):
    def wrapper(query: Query):
        for initiator in initiators:
            if initiator:
                query = initiator(query)
        return query

    return wrapper


def get_all_dict_keys(*data: dict):
    items = []
    for item in data:
        if item:
            items = items + list(item.keys())
    return items


def get_param_key(key):
    return key if isinstance(key, str) else None


def get_model_paginate_converter(relations, converters: list = None):
    all_fields = get_all_relation_map_config(relations, "fields")
    all_joins = get_all_relation_map_config(relations, "joins")
    all_keys = get_all_dict_keys(all_fields, all_joins)

    def define_converter(relation: dict):
        fields = relation.get("fields")
        joins = relation.get("joins")
        if not joins and not fields:
            return None
        fields = fields if fields else {}
        joins = joins if joins else {}
        keys = get_all_dict_keys(fields, joins)

        def converter(columns: list):
            params = [(get_param_key(k1), columns[n]) for k1 in keys for n, k2 in enumerate(all_keys) if k1 == k2]
            args = [param[1] for param in params if not param[0]]
            kwargs = dict([param for param in params if param[0]])
            func = relation.get("converter")
            data = func(*args, **kwargs)
            if isinstance(data, (list, tuple)):
                value = {}
                for item in data:
                    if item:
                        value.update(item)
                data = value
            return data

        return converter

    converters = converters if converters else []
    converters = [define_converter(paginate) for paginate in relations] + converters
    converters = [converter for converter in converters if converter]

    def wrapper(record):
        if not isinstance(record, Sized):
            record = [record] if record else []
        size = len(record)
        valid = len(all_keys) == size if all_fields else size == len(all_keys) + 1
        if not valid:
            raise Exception("relation config may be incorrect.")
        columns = record
        if all_fields:
            entity = {}
        else:
            entity = orm_to_dict(columns[0])
            columns = columns[1:]
        for converter in converters:
            data = converter(columns)
            if data:
                entity.update(data)
        return entity

    return wrapper


def orm_to_dict(entity, cache: dict = None):
    if entity is None:
        return None
    manager = getattr(entity, '_sa_class_manager', None)
    if manager is None:
        return entity
    cache = cache if cache else {}
    if entity in cache:
        return cache[entity]
    result = {}
    cache[entity] = result
    fields = getattr(manager, "_all_key_set")
    for field in fields:
        value = getattr(entity, field)
        orm = getattr(value, "__table__", None)
        if isinstance(value, list) or isinstance(value, tuple):
            result[field] = [orm_to_dict(item, cache=cache) for item in value]
        elif orm is None:
            result[field] = value
        else:
            result[field] = orm_to_dict(value, cache=cache)
    return result


def get_model_fields(model_type: type):
    fields = []
    for parent in model_type.__bases__:
        fields = fields + get_model_fields(parent)
    members = dir(model_type)
    for field in members:
        if isinstance(field, InstrumentedAttribute):
            fields.append(field.key)
    return fields


def get_valid_paginate_config_for_model(model_type: type, relation_config: str = None, limit=None) -> List[dict]:
    relation_config = relation_config if relation_config else "__relation__"
    configs = get_model_config_list(model_type, relation_config, limit)
    return get_valid_paginate_config(configs)


def get_valid_paginate_config(items: list) -> List[dict]:
    configs = []
    for item in items:
        model = item["model"]
        config = item["config"]
        if not config:
            continue
        fields = config.get("fields", None)
        if fields and not isinstance(fields, dict):
            raise Exception(f"value type of config 'fields' is incorrect: ${model}")
        initiator = config.get("initiator", None)
        if initiator and not hasattr(initiator, '__call__'):
            raise Exception(f"value type of config 'initiator' is incorrect: ${model}")
        converter = config.get("converter", None)
        if converter and not hasattr(converter, '__call__'):
            raise Exception(f"value type of config 'converter' is missing: ${model}")
        configs.append(config)
    return configs


def flat(items):
    items = [item if isinstance(item, Iterable) else [item] for item in items if item]
    return [item for each in items for item in each if item]


def cast_entity_as_dict(entity):
    return entity.dict()


def get_model_config_list(model_type: type, name: str, limit=None) -> List:
    if model_type is limit:
        return []
    config = getattr(model_type, name, None)
    if callable(config):
        config = config()
    if isinstance(config, dict):
        config = {"model": model_type, "config": config}
        results = [config]
    else:
        results = []
    base_name = config.get("base", name) if config else name
    for base in model_type.__bases__:
        items = get_model_config_list(base, base_name, limit)
        configs = [item["config"] for item in items]
        configs = [config for config in configs if config]
        results = items + [result for result in results if result["config"] not in configs]
    return results


def filter_tag_filter_criteria(condition, token):
    if 'labelName' in condition:
        user_info = get_user_info_from_token(token)
        condition['userInfoForLabel'] = user_info



@exter_attack
def define_page_query_api_for_model(
        router: APIRouter,
        database: Database,
        model_type,
        *joins,
        initiator: Callable[[Dict[str, any]], Callable[[Query], Query]] = None,
        converter: Callable[[any], T] = None,
        extra_conditions: List[str] = None,
        path: str = None,
        relation_config: str = None,
        default_conditions: Dict[str, any] = None,
        roles=None,
        tags=(),
        summary: str = None,
        authenticate=None,
        converter_response=None,
):
    relations = get_valid_paginate_config_for_model(model_type, relation_config)
    resource_type_schema = get_relation_schema(model_type, relations)
    if resource_type_schema is None:
        return None
    paginate = paginator(database, model_type, *joins, converter=converter, relations=relations,
                         default_conditions=default_conditions, relation_config=relation_config)
    extra_conditions = extra_conditions if extra_conditions else []
    table_name: str = getattr(model_type, '__tablename__')
    resource_type_name = table_name.replace('_', '-').lower()
    path = path.format(path=resource_type_name) if path else f"/{resource_type_name}"
    order_fields = get_relation_field_list(relations, "orders")
    filter_fields = get_relation_field_list(relations, "filters")
    extra_fields = get_relation_field_list(relations, "fields", lambda item: item.keys() if item else [])
    order_by_list = get_relation_field_list(relations, "order_by")
    auth_disabled = not roles
    order_fields_description = "排序字段：" + "，".join(order_fields)
    filter_fields_description = "条件参数：" + "，".join(filter_fields + extra_fields)

    if tags is not None:
        tags = tags if tags else [resource_type_name]

    # 会根据资源类型，去查对应的数据
    def query_resources(page_no: int, page_size: int, orders: List[str], condition: Dict[str, any], token=None):
        filter_tag_filter_criteria(condition, token)
        custom_initiator = initiator(condition) if initiator else None
        author_initiator = authenticate(token) if authenticate and not auth_disabled else None

        def call_initiator(method, query: Query):
            if method is None:
                return query
            count = len(signature(method).parameters)
            if count == 0:
                return query
            if count == 1:
                return method(query)
            with database.session() as session:
                return method(query, session)

        def query_initiator(query: Query):
            if author_initiator:
                if not token:
                    raise EmeiStorBizException(UserErrorCodes.ACCESS_DENIED)
                query = call_initiator(author_initiator, query)
            query = call_initiator(custom_initiator, query)
            return query

        for extra_condition in extra_conditions:
            if extra_condition in condition and extra_condition not in RBACConstants.IGNORE_FIELDS_LIST:
                del condition[extra_condition]
        response = paginate(
            page=page_no,
            size=page_size,
            orders=orders,
            conditions=condition,
            query_initiator=query_initiator,
        )
        if converter_response:
            response = converter_response(response, token)
        return response

    decorator = router.get(
        path,
        status_code=200,
        tags=tags,
        summary=summary if summary else f"查询{resource_type_name}信息列表",
        response_model=BasePage[resource_type_schema],
    )
    if auth_disabled:
        # 非鉴权接口场景
        def do_query(
                page_no: int = Query(..., description="分页页面编码", ge=0),
                page_size: int = Query(..., description="分页数据条数", ge=0, le=200),
                orders: List[str] = async_depend(
                    order_resolver(
                        model_type,
                        *order_fields,
                        extra_fields=extra_fields,
                        description=order_fields_description,
                    )
                ),
                conditions: Dict[str, any] = async_depend(
                    filter_resolver(
                        model_type, *filter_fields,
                        extras=extra_conditions,
                        relations=relations,
                        description=filter_fields_description,
                    )
                ),
        ):
            orders += order_by_list
            return query_resources(page_no, page_size, orders, conditions)
    else:
        # 鉴权接口场景
        @right_control(roles=roles)
        def do_query(
                page_no: int = Query(..., description="分页页面编码", ge=0),
                page_size: int = Query(..., description="分页数据条数", ge=0, le=200),
                orders: List[str] = async_depend(
                    order_resolver(
                        model_type,
                        *order_fields,
                        extra_fields=extra_fields,
                        description=order_fields_description,
                    )
                ),
                conditions: Dict[str, any] = async_depend(
                    filter_resolver(
                        model_type, *filter_fields,
                        extras=extra_conditions,
                        relations=relations,
                        description=filter_fields_description,
                    )
                ),
                token=Header(..., alias="X-Auth-Token", title="X-Auth-Token", description="访问令牌"),
        ):
            orders += order_by_list
            resource_data = query_resources(page_no, page_size, orders, conditions, token)
            if conditions.get("type") == "VM":
                for res_data in resource_data.items:
                    resource_id = res_data.get("uuid")
                    resource_group_member = resource_service.query_resource_group_member_info(resource_id)
                    res_data.update({"in_group": False})
                    if resource_group_member:
                        res_data.update({"in_group": True})
                        resource_group_id = resource_group_member.resource_group_id
                        resource_group = resource_service.query_resource_group_by_id(resource_group_id)
                        res_data.update({"resource_group_id": resource_group_id,
                                         "resource_group_name": resource_group.name})
                        continue
                    protected_resource = resource_service.query_protected_resource_by_id(resource_id)
                    if protected_resource and protected_resource.resource_group_id:
                        res_data.update({"in_group": True})
                        resource_group_id = protected_resource.resource_group_id
                        resource_group = resource_service.query_resource_group_by_id(resource_group_id)
                        res_data.update({"resource_group_id": resource_group_id,
                                         "resource_group_name": resource_group.name})
            return resource_data
    do_query.__name__ = f"query_resources"
    api = decorator(do_query)
    api.schema = resource_type_schema
    return api


def get_relation_schema(model_type: type, relations):
    for relation in relations[::-1]:
        schema = relation.get("schema")
        if schema is not None:
            return schema
    return getattr(model_type, "__schema__", None)


def get_relation_field_list(relations, name, converter=None):
    if not relations:
        return []
    collection = [relation.get(name) for relation in relations if relation]

    def convert(item):
        return converter(item) if converter else item

    return [item for each in collection if each for item in convert(each) if item]


def paginator(
        database: Database,
        model_type,
        *joins,
        default_orders: List[str] = None,
        default_conditions: Dict[str, any] = None,
        initiator: Callable[[Query], Query] = None,
        converter: Callable[[any], T] = None,
        relations=None,
        relation_config: str = None
):
    default_orders = default_orders if default_orders else []
    default_conditions = default_conditions if default_conditions else {}
    relations = relations if relations else get_valid_paginate_config_for_model(model_type, relation_config)
    joins = list(joins) if joins else []
    field_configs = get_all_relation_map_config(relations, "fields")
    join_configs = get_all_relation_map_config(relations, "joins")
    fields = field_configs.values()
    joins = list(join_configs.keys()) + joins
    extra_entities = list(fields) + list(joins)
    join_initiators = [create_join_initiator(model, condition) for model, condition in join_configs.items()]
    join_initiators.append(initiator)
    converter = combine_multiple_method_as_chain_method(
        get_model_paginate_converter(relations, converters=[converter]),
        get_paginate_reform_handler(relations)
    )
    extra_fields = get_all_relation_map_config(relations, "alias")

    def transform(record):
        if isinstance(record, (set, list, tuple)):
            entity = record[0]
        else:
            entity = record
        result = converter(record)
        entity_type = type(entity)
        if entity_type is not model_type and issubclass(entity_type, model_type):
            sub_relations = get_valid_paginate_config_for_model(entity_type, relation_config, limit=model_type)
            reform = get_paginate_reform_handler(sub_relations)
            result = reform(result)
        return result

    def paginate(
            page: int,
            size: int,
            orders: List[str] = None,
            conditions: Dict[str, any] = None,
            query_initiator: Callable[[Query], Query] = None
    ):
        if fields:
            entities = extra_entities
        else:
            entities = [model_type, *extra_entities] if extra_entities else [model_type]
        orders = orders if orders else []
        conditions = conditions if conditions else {}
        conditions.update(default_conditions)
        combined_initiator = get_model_paginate_initiator(relations, join_initiators + [query_initiator])
        result = database.paginate(
            *entities,
            page=page,
            size=size,
            orders=default_orders + orders,
            conditions=conditions,
            initiator=combined_initiator,
            converter=transform,
            extra_fields=extra_fields,
        )
        return result

    return paginate


def get_paginate_reform_handler(relations):
    relations = relations if relations else []
    handlers = [relation.get("reform") for relation in relations]
    handlers = [handler for handler in handlers if handler]

    def handler(result):
        for handler in handlers:
            result = handler(result)
        return result

    return handler


def get_mapper_arg(model_type: type, name: str):
    args = getattr(model_type, "__mapper_args__", None)
    if not args or name not in args:
        return None
    return args[name]


def create_join_initiator(model, condition, outer: bool = True):
    if isinstance(condition, dict):
        outer = "outer" == condition.get("mode", None)
        condition = condition["join"]

    def join_initiator(query: Query):
        if isinstance(condition, list) or isinstance(condition, tuple):
            query = query.join(*condition, isouter=outer)
        else:
            query = query.join(model, condition, isouter=outer)
        return query

    return join_initiator
