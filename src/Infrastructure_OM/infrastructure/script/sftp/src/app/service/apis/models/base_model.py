# coding: utf-8
import typing
import six

from app.common.logger import log

T = typing.TypeVar('T')


class Model(object):
    # swaggerTypes: 类属性及其数据类型的映射关系
    # key是类的属性名（小写加下划线风格），value是属性数据类型（可以为内建的数据类型，如int、str、list、map等，也可以是自定义的数据类型）
    swagger_types = {}

    # attributeMap: 类属性和swagger接口定义字段的映射关系
    # key是类的属性名（小写加下划线风格），value是类属性对应的swagger接口定义中字段名（小驼峰风格）
    attribute_map = {}

    def to_dict(self):
        """
        将类对象的属性转换为字典

        :return: 字典格式的数据
        :rtype: dict
        """
        result = {}

        for attr, _ in six.iteritems(self.swagger_types):
            value = getattr(self, attr)
            if attr not in self.attribute_map:
                log.error(f'attr:{attr} is not defined')
                continue

            if isinstance(value, list):
                result[self.attribute_map[attr]] = list(map(
                    lambda x: x.to_dict() if hasattr(x, "to_dict") else x,
                    value
                ))
            elif hasattr(value, "to_dict"):
                result[self.attribute_map[attr]] = value.to_dict()
            elif isinstance(value, dict):
                result[self.attribute_map[attr]] = dict(map(
                    lambda item: (item[0], item[1].to_dict())
                    if hasattr(item[1], "to_dict") else item,
                    value.items()
                ))
            else:
                result[self.attribute_map[attr]] = value

        return result
