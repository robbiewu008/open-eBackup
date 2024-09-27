import os
from fastapi import APIRouter, File, UploadFile, status, HTTPException, Form
from server.common.logger.logger import get_logger
from server.common.consts import LOG_PATH
from server.services.package_manage.package import unpack, upload, delete, upload_databackup, unpack_package
from server.schemas.request import MoveFileRequest, UnpackPackageRequest, GetPackageStatusResponse

external_router = APIRouter(
    prefix="/package",
    tags=["package"]
)

databackup_router = APIRouter(
    prefix="/package",
    tags=["package"]
)


@external_router.post("/upload", summary="上传包")
def upload_file(package_size: str = Form(...), file: UploadFile = File(...)):
    has_issue = upload(package_size, file)
    if has_issue:
        delete(file.filename)
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Failed to upload file.")


@databackup_router.post("/upload", summary="上传包")
def upload_file(package_size: str = Form(...), file: UploadFile = File(...)):
    has_issue = upload_databackup(package_size, file)
    if has_issue:
        delete(file.filename)
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="Failed to upload file.")


@databackup_router.delete("/delete/{package_name}", summary="删除包")
@external_router.delete("/delete/{package_name}", summary="删除包")
def delete_package(package_name: str):
    has_issue = delete(package_name)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail="delete package failed.")


@external_router.post("/unpack_package", summary="解压包")
@databackup_router.post("/unpack_package", summary="解压包")
def get_package_status(req: UnpackPackageRequest):
    has_issue, package_status, info = unpack_package(req.hash_value, req.package_name, req.package_size)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"unpack package failed, {info}")
    return GetPackageStatusResponse(status=package_status)

