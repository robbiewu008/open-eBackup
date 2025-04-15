import os
from fastapi import APIRouter, HTTPException, status
from server.common.logger.logger import logger
from server.services.appinstall import dataprotect
from server.schemas.request import (
    InstallSimbaOSRequest,
    InstallSmartkubeRequest,
    PreinstallDataProtectRequest,
    ResetSimbaOSRequest,
    InstallDataProtectRequest,
    GetSimbaosStatusResponse,
    GetDataProtectStatusResponse,
    ExpandDataBackupRequest,
    ExpandSimbaOSRequest,
    UpgradeDataProtectRequest,
    DeleteSimbaOSNodeRequest,
    SynchronizeCertRequest,
    DataBackupExpandDataBackupRequest,
    PreUpgradeSimbaOSRequest,
    UpgradeSimbaOSRequest,
    UpgradeStateCheck,
    UpgradeDatabackup,
)
from server.services.appinstall.dataprotect import (
    simbaos_reset,
    simbaos_preinstall,
    simbaos_install,
    dataprotect_preinstall,
    dataprotect_install,
    dataprotect_reset,
    get_simbaos_status,
    get_dataprotect_status,
    expand_dataprotect,
    expand_simbaos,
    upgrade_dataprotect,
    delete_simbaos_node,
    syn_cert,
    databackup_expand_dataprotect,
    pre_upgrade_simbaos,
    upgrade_simbaos,
    post_upgrade_simbaos,
)
from server.common import consts
from server.services.appinstall.dp_upgrade_check import (
    upgrade_state_check,
    upgrade_state_check_post,
    CheckFailures,
    upgrade_alarms_check,
    upgrade_config_check,
    upgrade_resources_check,
    upgrade_services_check,
    upgrade_jobs_check
)
from server.services.appinstall.dp_upgrade_databackup import upgrade_databackup

external_router = APIRouter(prefix="/app", tags=["application"])

databackup_router = APIRouter(prefix="/app", tags=["application"])


@external_router.post("/test")
def test(req: InstallSmartkubeRequest):
    return GetSimbaosStatusResponse(status=req.node_ip, version=req.package_name)


@databackup_router.post("/simbaos/preinstall", summary="软硬解耦预安装SimbaOS")
@external_router.post("/simbaos/preinstall", summary="预安装SimbaOS")
def request_install_smartkube(req: InstallSmartkubeRequest):
    script_path = os.path.join(consts.SCRIPTS_PATH, "simbaos", req.device_type.lower(), "preinstall.sh")
    has_issue, info = simbaos_preinstall(req, script_path)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to install smartkube, info:{info}")


@databackup_router.post("/simbaos/install", summary="安装SimbaOS")
@external_router.post("/simbaos/install", summary="安装SimbaOS")
def request_install_simbaos(req: InstallSimbaOSRequest):
    script_path = os.path.join(consts.SCRIPTS_PATH, "simbaos", req.device_type.lower(), "install.sh")
    has_issue, info = simbaos_install(req, script_path)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to install simbaos, info:{info}")


@databackup_router.post("/simbaos/reset", summary="卸载SimbaOS")
@external_router.post("/simbaos/reset", summary="卸载SimbaOS")
def request_reset_simbaos():
    has_issue, info = simbaos_reset()
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to reset simbaos, info: {info}")


@databackup_router.get("/simbaos/status", summary="检查SimbaOS状态")
@external_router.get("/simbaos/status", summary="检查SimbaOS状态")
def request_get_simbaos_status():
    has_issue, version, node_role_list = get_simbaos_status()
    if has_issue:
        return GetSimbaosStatusResponse(status="SimbaOS not found")
    return GetSimbaosStatusResponse(status="deployed", version=version, nodes=node_role_list)


@databackup_router.post("/simbaos/expand", summary="扩容SimbaOS")
@external_router.post("/simbaos/expand", summary="扩容SimbaOS")
def request_expand_simbaos(req: ExpandSimbaOSRequest):
    has_issue, info = expand_simbaos(req)
    if has_issue:
        HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to expand simbaos, info:{info}")


@external_router.post("/simbaos/delete_node", summary="删除SimbaOS节点")
def request_expand_simbaos(req: DeleteSimbaOSNodeRequest):
    has_issue, info = delete_simbaos_node(req)
    if has_issue:
        HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to delete simbaos node, info:{info}")


@external_router.post("/simbaos/pre_upgrade", summary="预升级SimbaOS")
def request_expand_simbaos(req: PreUpgradeSimbaOSRequest):
    has_issue, info = pre_upgrade_simbaos(req)
    if has_issue:
        HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to pre_upgrade simbaos, info:{info}")


@external_router.post("/simbaos/upgrade", summary="升级SimbaOS")
def request_expand_simbaos(req: UpgradeSimbaOSRequest):
    has_issue, info = upgrade_simbaos(req)
    if has_issue:
        HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to upgrade simbaos, info:{info}")


@external_router.post("/simbaos/post_upgrade", summary="SimbaOS升级后处理")
def request_expand_simbaos():
    has_issue, info = post_upgrade_simbaos()
    if has_issue:
        HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to post_upgrade simbaos node, " f"info:{info}"
        )


@databackup_router.post("/dataprotect/preinstall", summary="加载镜像包")
@external_router.post("/dataprotect/preinstall", summary="加载镜像包")
def request_load_image(req: PreinstallDataProtectRequest):
    has_issue, info = dataprotect_preinstall(req)
    if has_issue:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to load dataprotect image, info: {info}"
        )


@databackup_router.post("/dataprotect/syn_cert", summary="同步证书相关文件")
def request_syn_cert(req: SynchronizeCertRequest):
    has_issue = syn_cert(req)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to syn cert")


@external_router.post("/dataprotect/install", summary="安装数据保护软件")
@databackup_router.post("/dataprotect/install", summary="安装数据保护软件")
def request_install_dataprotect(req: InstallDataProtectRequest):
    script_path = os.path.join(consts.SCRIPTS_PATH, "dataprotect", req.device_type.lower(), "install.sh")
    has_issue, info = dataprotect_install(req, script_path)
    if has_issue:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to install dataprotect, info:{info}"
        )


@databackup_router.delete("/dataprotect/reset", summary="卸载数据保护软件")
@external_router.delete("/dataprotect/reset", summary="卸载数据保护软件")
def request_reset_dataprotect():
    has_issue, info = dataprotect_reset()
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to reset dataprotect, info:{info}")


@databackup_router.get("/dataprotect/status", summary="查询数据保护软件安装情况")
@external_router.get("/dataprotect/status", summary="查询数据保护软件安装情况")
def request_get_dataprotect_status():
    has_issue, deploy_status, chart_name, info = get_dataprotect_status()
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to get dataprotect, info:{info}")
    return GetDataProtectStatusResponse(status=deploy_status, chart_name=chart_name)


@databackup_router.post("/dataprotect/expand", summary="扩容数据保护软件")
def request_install_dataprotect(req: DataBackupExpandDataBackupRequest):
    script_path = os.path.join(consts.SCRIPTS_PATH, "dataprotect", req.device_type.lower(), "expand.sh")
    has_issue, info = databackup_expand_dataprotect(req, script_path)
    if has_issue:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to install dataprotect, info:{info}"
        )


@external_router.post("/dataprotect/upgrade", summary="升级数据保护软件")
@databackup_router.post("/dataprotect/upgrade", summary="升级数据保护软件")
def request_upgrade_dataprotect(req: UpgradeDataProtectRequest):
    has_issue, info = upgrade_dataprotect(req)
    if has_issue:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to install dataprotect, info:{info}"
        )

@external_router.post("/dataprotect/upgrade", summary="升级数据保护软件")
@databackup_router.post("/dataprotect/upgrade", summary="升级数据保护软件")
def request_upgrade_dataprotect(req: UpgradeDataProtectRequest):
    has_issue, info = upgrade_dataprotect(req)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST,
                            detail=f"Failed to install dataprotect, info:{info}")

@external_router.post("/dataprotect/expand", summary="扩容数据保护软件")
def request_install_dataprotect(req: ExpandDataBackupRequest):
    script_path = os.path.join(consts.SCRIPTS_PATH, "dataprotect", req.device_type.lower(), "expand.sh")
    has_issue, info = expand_dataprotect(req, script_path)
    if has_issue:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to install dataprotect, info:{info}"
        )


@databackup_router.post("/dataprotect/upgrade_check", summary="数据保护软件升级前检查")
@external_router.post("/dataprotect/upgrade_check", summary="数据保护软件升级前检查")
def request_upgrade_status_check(req: UpgradeStateCheck):
    no_issue, info = upgrade_state_check(req)
    if not no_issue:
        for err in CheckFailures:
            if info.split(":", 1)[0] == err.value[:-1]:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"{err.value} Failed to pass the preupgrade status check, info:{info}",
                )

@databackup_router.post("/dataprotect/upgrade_check_alarm", summary="数据保护软件升级前检查")
@external_router.post("/dataprotect/upgrade_check_alarm", summary="数据保护软件升级前检查")
def request_upgrade_status_check_alarm(req: UpgradeStateCheck):
    no_issue, info = upgrade_alarms_check(req)
    if not no_issue:
        for err in CheckFailures:
            if info.split(":", 1)[0] == err.value[:-1]:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"{err.value} Failed to pass the preupgrade status check, info:{info}",
                )

@databackup_router.post("/dataprotect/upgrade_check_config", summary="数据保护软件升级前检查")
@external_router.post("/dataprotect/upgrade_check_config", summary="数据保护软件升级前检查")
def request_upgrade_status_check_config(req: UpgradeStateCheck):
    no_issue, info = upgrade_config_check(req)
    if not no_issue:
        for err in CheckFailures:
            if info.split(":", 1)[0] == err.value[:-1]:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"{err.value} Failed to pass the preupgrade status check, info:{info}",
                )

@databackup_router.post("/dataprotect/upgrade_check_resources", summary="数据保护软件升级前检查")
@external_router.post("/dataprotect/upgrade_check_resources", summary="数据保护软件升级前检查")
def request_upgrade_status_check_config(req: UpgradeStateCheck):
    no_issue, info = upgrade_resources_check(req)
    if not no_issue:
        for err in CheckFailures:
            if info.split(":", 1)[0] == err.value[:-1]:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"{err.value} Failed to pass the preupgrade status check, info:{info}",
                )

@databackup_router.post("/dataprotect/upgrade_services_services", summary="数据保护软件升级前检查")
@external_router.post("/dataprotect/upgrade_services_services", summary="数据保护软件升级前检查")
def request_upgrade_status_check_config(req: UpgradeStateCheck):
    no_issue, info = upgrade_services_check(req)
    if not no_issue:
        for err in CheckFailures:
            if info.split(":", 1)[0] == err.value[:-1]:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"{err.value} Failed to pass the preupgrade status check, info:{info}",
                )

@databackup_router.post("/dataprotect/upgrade_jobs_services", summary="数据保护软件升级前检查")
@external_router.post("/dataprotect/upgrade_jobs_services", summary="数据保护软件升级前检查")
def request_upgrade_status_check_jobs(req: UpgradeStateCheck):
    no_issue, info = upgrade_jobs_check(req)
    if not no_issue:
        for err in CheckFailures:
            if info.split(":", 1)[0] == err.value[:-1]:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"{err.value} Failed to pass the preupgrade status check, info:{info}",
                )

@databackup_router.post("/dataprotect/upgrade_post_check", summary="数据保护软件升级后检查")
@external_router.post("/dataprotect/upgrade_post_check", summary="数据保护软件升级后检查")
def request_upgrade_status_postcheck(req: UpgradeStateCheck):
    no_issue, info = upgrade_state_check_post(req)
    if not no_issue:
        for err in CheckFailures:
            if info.split(":", 1)[0] == err.value[:-1]:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"{err.value} Failed to pass the post upgrade status check, info:{info}",
                )


@databackup_router.post("/dataprotect/upgrade_databackup", summary="数据保护软件升级前备份")
@external_router.post("/dataprotect/upgrade_databackup", summary="升级数据保护软件升级前备份")
def request_upgrade_databackup(req: UpgradeDatabackup):
    no_issue, info = upgrade_databackup(req)
    if not no_issue:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to backup the info before the upgrade, info:{info}"
        )
