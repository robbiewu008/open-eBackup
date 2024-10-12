from enum import Enum

from pydantic import BaseModel, Field


class BaseModelConf(BaseModel):
    class Config:
        # allow use field name as parameters of object and alias name
        allow_population_by_field_name = True


class RequestResult(BaseModelConf):
    error_code: int = Field(..., description="error code", alias='ErrorCode')
    error_desc: str = Field(..., description="error desc", alias='ErrorDesc')


class AccountType(str, Enum):
    WINDOWS_USER = 0
    AD_USER = 2
    AD_GROUP = 3


class ResUserInGroup(BaseModelConf):
    account_type: AccountType = Field(..., description="Account type", alias='ACCOUNTTYPE')
    name: str = Field(..., description="Name of the Windows user, AD domain user/user group", alias='NAME')
    RID: int = Field(None, description="User RID.", alias='RID')
    vstore_id: str = Field(None, description="vStore ID", alias='vstoreId')
    vstore_name: str = Field(None, description="vStore name", alias='vstoreName')
