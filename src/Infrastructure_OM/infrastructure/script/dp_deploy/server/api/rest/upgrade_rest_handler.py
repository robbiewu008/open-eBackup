from fastapi import APIRouter
from server.schemas.request import GetVersion, RequestUpgradeDpserver
from server.services.upgrade.upgrade import get_dpserver_version, upgrade_dpserver
from server.common.logger.logger import logger

external_router = APIRouter(
    prefix="/dpserver",
    tags=["upgrade"])


@external_router.get("/version", summary="查询当前dpserver版本信息")
def request_get_dpserver_version():
    version = get_dpserver_version()
    logger.info(f"dpserver version is {version}")
    return GetVersion(version=version)


@external_router.post("/upgrade", summary="升级dpserver")
def request_upgrade_dpserver(req: RequestUpgradeDpserver):
    ret = upgrade_dpserver(req)
    if ret:
        logger.error(f"Dpserver upgrade failed")
    else:
        logger.info(f"Dpserver upgrade success.")
