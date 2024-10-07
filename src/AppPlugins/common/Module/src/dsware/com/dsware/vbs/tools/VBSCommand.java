/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package com.dsware.vbs.tools;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * <Description>
 * 
 */
public class VBSCommand
{
    private static final Logger LOGGER = Logger.getLogger(VBSCommand.class);
    
    private DSwareApi api = null;
    private static final Logger logger = Logger.getLogger(CommandHandler.class);
    
    public int createVolume(Map<String, String> cmdArgs)
    {
        String dsaIps = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String[] dsaIpArray = dsaIps.split(",");
        String volName = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);
        
        int poolId;
        int volSize;
        int thinFlag;
        try {
            poolId = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_POOL_ID));
            volSize = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_VOL_SIZE));
            thinFlag = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_THIN_FLAG));
        }
        catch (NumberFormatException e) {
            LOGGER.error("numberformat exception", e);
            return ToolsErrors.TOOLS_ERROR;
        }
        
        try {
            api.createVolume(dsaIpArray, volName, poolId, volSize, thinFlag);
        }
        catch (DSwareException e) {
            printErrorString(new String[] {"Failed to create Volume, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }

        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int deleteVolume(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String volName = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);
        
        String classname = DSwareApiImpl.class.getName();
        String mestmp = "DSwareApiImpl.class.getName" + classname;
        logger.error(mestmp);
        @SuppressWarnings("rawtypes")
        Class cDswareApi;
        try {
            cDswareApi = Class.forName(DSwareApiImpl.class.getName());
        }
        catch (ClassNotFoundException e) {
            logger.error("Get DSwareApiImpl class command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }
        
        //add by wenzhibin 20140218 support multi-dsware begin
        String dswareFloatIp = cmdArgs.get(ToolsContances.CMD_ARG_DSWAREFLOAT_IP);
        DSwareApi delBitmapVolApi = new DSwareApiImpl(dswareFloatIp);
        
        @SuppressWarnings("unchecked")
        Method mDes;
        String cmdOp = "deleteVolume";
        try {
            mDes = cDswareApi.getMethod(cmdOp, String[].class, String.class, int.class);
        } catch (SecurityException | NoSuchMethodException e) {
            try {
                delBitmapVolApi.deleteVolume(dsaIp.split(","), volName);
            }
            catch (DSwareException ec) {
                printErrorString(new String[] {"Failed to delete Volume, cmdArgs:",
                        cmdArgs.toString()});
                printException(ec);
                return ec.getError().getErrorCode();
            }        
            return ToolsErrors.TOOLS_SUCCESS;
        }
        
        //add by wenzhibin end       
        int poolId = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_POOL_ID));
        try {
            mDes.invoke(delBitmapVolApi, dsaIp.split(","), volName, poolId);
        }
        catch (InvocationTargetException e) {
            DSwareException ec = (DSwareException) e.getTargetException();
            printErrorString(new String[] {"Failed to delete Volume, cmdArgs:",
                        cmdArgs.toString()});
                printException(ec);
                return ec.getError().getErrorCode();
        }
        catch (IllegalArgumentException | IllegalAccessException e) {
            logger.error("Command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }
        
        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int queryVolume(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String volName = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);
        
        try {
            DSwareVolumeInfo volInfo = api.queryVolume(dsaIp.split(","),
                    volName);
            Map<String, DSwareVolumeInfo> volInfos = new HashMap<String, DSwareVolumeInfo>();
            volInfos.put("", volInfo);
            printVolumeInfos(volInfos);
            
            return ToolsErrors.TOOLS_SUCCESS;
        }
        catch (DSwareException e) {
            printErrorString(new String[] {"Failed to query Volume, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
    }
    
    public int queryAllVolume(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        
        int poolId;
        try {
            poolId = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_POOL_ID));
        }
        catch (NumberFormatException e) {
            LOGGER.error("numberformat exception", e);
            return ToolsErrors.TOOLS_ERROR;
        }

        try {
            Map<String, DSwareVolumeInfo> volMap = api.queryAllVolume(dsaIp.split(","), poolId);
            printVolumeInfos(volMap);
            return ToolsErrors.TOOLS_SUCCESS;
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to query all Volume, cmdArgs:", cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
    }
    
    public int createSnapshot(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String volName = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);
        String snapName = cmdArgs.get(ToolsContances.CMD_ARG_SNAP_NAME);
        
        int smartFlag;
        try {
            smartFlag = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_SMART_FLAG));
        }
        catch (NumberFormatException e) {
            LOGGER.error("numberformat exception", e);
            return ToolsErrors.TOOLS_ERROR;
        }
        
        try {
            api.createSnapshot(dsaIp.split(","), snapName, volName, smartFlag);
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to create snapshot, cmdArgs:", cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int deleteSnapshot(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String snapName = cmdArgs.get(ToolsContances.CMD_ARG_SNAP_NAME);
        
        try {
            api.deleteSnapshot(dsaIp.split(","), snapName);
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to delete snapshot, cmdArgs:", cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
        
        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int querySnapshot(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String snapName = cmdArgs.get(ToolsContances.CMD_ARG_SNAP_NAME);
        
        try {
            DSwareSnapInfo snap = api.querySnapshot(dsaIp.split(","), snapName);
            Map<String, DSwareSnapInfo> snapMap = new HashMap<String, DSwareSnapInfo>();
            snapMap.put("", snap);
            printSnapshotInfos(snapMap);
            return ToolsErrors.TOOLS_SUCCESS;
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                "Failed to query snapshot, cmdArgs:", cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
        
    }
    
    public int queryAllSnapshot(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        
        int poolId;
        try {
            poolId = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_POOL_ID));
        }
        catch (NumberFormatException e) {
            printErrorString(new String[] {
                    "Failed to query all snapshot, cmdArgs:",
                    cmdArgs.toString()});
            LOGGER.error("numberformat exception", e);
            return ToolsErrors.TOOLS_ERR_COMMAND_FAILED;
        }
        try {
            String dswareFloatIp = cmdArgs.get(ToolsContances.CMD_ARG_DSWAREFLOAT_IP);
            DSwareApi queryBitmapVolApi = new DSwareApiImpl(dswareFloatIp);
            
            Map<String, DSwareSnapInfo> snapMap = queryBitmapVolApi.queryAllSnapshot(dsaIp.split(","),
                    poolId);
            printSnapshotInfos(snapMap);
            return ToolsErrors.TOOLS_SUCCESS;
        }
        catch (DSwareException e) {
            printException(e);
            return e.getError().getErrorCode();
        }
    }

    public int duplicateSnapshot(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String snapNameSrc = cmdArgs.get(ToolsContances.CMD_ARG_SNAP_NAME_SRC);
        String snapNameDst = cmdArgs.get(ToolsContances.CMD_ARG_SNAP_NAME_DEST);
        
        int poolId;
        int fullCopyFlag;
        int smartFlag;
        try {
            poolId = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_POOL_ID));
            fullCopyFlag = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_FULL_COPY_FLAG));
            smartFlag = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_SMART_FLAG));
        }
        catch (NumberFormatException e) {
            LOGGER.error("numberformat exception", e);
            return ToolsErrors.TOOLS_ERROR;
        }
        try {
            api.duplicateSnapshot(dsaIp.split(","),
                    snapNameSrc,
                    snapNameDst,
                    poolId,
                    fullCopyFlag,
                    smartFlag);
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to duplicate snapshot, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }

        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int createVolumeFromSnap(Map<String, String> cmdArgs)
    {
        String snapNameSrc = cmdArgs.get(ToolsContances.CMD_ARG_SNAP_NAME_SRC);
        String volName = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);
        
        int volSize;
        try {
            volSize = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_VOL_SIZE));
        }
        catch (NumberFormatException e) {
            LOGGER.error("numberformat exception", e);
            return ToolsErrors.TOOLS_ERROR;
        }

        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        try {
            api.createVolumeFromSnap(dsaIp.split(","),
                    volName,
                    volSize,
                    snapNameSrc);
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to create volume from snap, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }

        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int createVolumesFromSnap(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String snapNameSrc = cmdArgs.get(ToolsContances.CMD_ARG_SNAP_NAME_SRC);
        String volNames = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);

        String volName[] = volNames.split(" ");

        int volSize;

        try {
            volSize = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_VOL_SIZE));
        }
        catch (NumberFormatException e) {
            LOGGER.error("numberformat exception", e);
            return ToolsErrors.TOOLS_ERROR;
        }
        
        try {
            api.createVolumesFromSnap(dsaIp.split(","),
                    volName,
                    volSize,
                    snapNameSrc);
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to create volumes from snapshot, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
        
        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int querySnapOfVolume(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String volName = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);
        
        try {
            Map<String, DSwareSnapInfo> snapMap = api.querySnapOfVolume(dsaIp.split(","),
                    volName);
            printSnapshotInfos(snapMap);
            return ToolsErrors.TOOLS_SUCCESS;
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to query snapshot of volume, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
        
    }
    
    public int queryVolumeOfSnap(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String snapName = cmdArgs.get(ToolsContances.CMD_ARG_SNAP_NAME);
        
        try {
            Map<String, DSwareVolumeInfo> volMap = api.queryVolumeOfSnap(dsaIp.split(","),
                    snapName);
            printVolumeInfos(volMap);
            return ToolsErrors.TOOLS_SUCCESS;
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to query volume of snapshot, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
    }
    
    public int queryPoolInfo(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        int poolId;
        try {
            poolId = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_POOL_ID));
        }
        catch (NumberFormatException e) {
            LOGGER.error("numberformat exception", e);
            return ToolsErrors.TOOLS_ERROR;
        }
        
        try {
            DSwarePoolInfo pool = api.queryPoolInfo(dsaIp.split(","), poolId);
            Map<Integer, DSwarePoolInfo> poolMap = new HashMap<Integer, DSwarePoolInfo>();
            poolMap.put(poolId, pool);
            printPoolInfos(poolMap);
            return ToolsErrors.TOOLS_SUCCESS;
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to query pool info, cmdArgs:", cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
    }
    
    public int createFullVolumeFromSnap(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String volumeName = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);
        String snapName = cmdArgs.get(ToolsContances.CMD_ARG_SNAP_NAME);
        
        try {
            api.createFullVolumeFromSnap(dsaIp.split(","), snapName, volumeName);
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to create full volume, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
        
        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int queryAllPoolInfo(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);

        try {
            Map<Integer, DSwarePoolInfo> poolMap = api.queryAllPoolInfo(dsaIp.split(","));
            printPoolInfos(poolMap);
            return ToolsErrors.TOOLS_SUCCESS;
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to query all pool info, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
    }
    
    public int createBitmapVolume(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String snapNameFrom = cmdArgs.get(ToolsContances.CMD_ARG_SNAPNAME_FROM);
        String snapNameTo = cmdArgs.get(ToolsContances.CMD_ARG_SNAPNAME_TO);

        String classname = DSwareApiImpl.class.getName();
        String mestmp = "DSwareApiImpl.class.getName" + classname;
        logger.error(mestmp);
        @SuppressWarnings("rawtypes")
        Class cDswareApi;
        try {
            cDswareApi = Class.forName(DSwareApiImpl.class.getName());
        }
        catch (ClassNotFoundException e) {
            logger.error("Get DSwareApiImpl class command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }

        //add by wenzhibin 20140218 support multi-dsware begin
        String dswareFloatIp = cmdArgs.get(ToolsContances.CMD_ARG_DSWAREFLOAT_IP);
        DSwareApi createBitmapVolApi = new DSwareApiImpl(dswareFloatIp);
        
        @SuppressWarnings("unchecked")
        Method mDes;
        String cmdOp = "createBitmapVolume";
        DSwareBackupInfo dswareBackupInfo = null;
        String volNameForBitmap = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);
        try {
            mDes = cDswareApi.getMethod(cmdOp, String[].class, String.class, String.class, String.class, int.class);
        } catch (SecurityException | NoSuchMethodException e) {
            logger.error("Get function failed:", e);
            try {
                dswareBackupInfo = createBitmapVolApi.createBitmapVolume(dsaIp.split(","),
                    snapNameFrom,
                    snapNameTo,
                    volNameForBitmap);
            }
            catch (DSwareException ec) {
                printErrorString(new String[] {
                    "Failed to create bitmap volume, cmdArgs:",
                    cmdArgs.toString()});
                printException(ec);
                return ec.getError().getErrorCode();
            }
        
            printDSwareBackupInfo(dswareBackupInfo);
        
            return ToolsErrors.TOOLS_SUCCESS;
        }
        
        //add by wenzhibin end
        int poolId = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_POOL_ID));
        Object ret = null;
        try {
            ret = mDes.invoke(createBitmapVolApi, dsaIp.split(","), snapNameFrom, snapNameTo, volNameForBitmap, poolId);
            if (ret != null) {
                dswareBackupInfo = (DSwareBackupInfo) ret;
            }
        }
        catch (InvocationTargetException e) {
            DSwareException ec = (DSwareException) e.getTargetException();
            printErrorString(new String[] {
                    "Failed to create bitmap volume, cmdArgs:",
                    cmdArgs.toString()});
                printException(ec);
            return ec.getError().getErrorCode();
        }
        catch (IllegalArgumentException | IllegalAccessException e) {
            logger.error("Command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }

        printDSwareBackupInfo(dswareBackupInfo);
        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int queryBitmapVolume(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String classname = DSwareApiImpl.class.getName();
        String mestmp = "DSwareApiImpl.class.getName" + classname;
        logger.error(mestmp);
        @SuppressWarnings("rawtypes")
        Class cDswareApi;
        try {
            cDswareApi = Class.forName(DSwareApiImpl.class.getName());
        }
        catch (ClassNotFoundException e) {
            logger.error("Get DSwareApiImpl class command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }
        
        //add by wenzhibin 20140218 support multi-dsware begin
        String dswareFloatIp = cmdArgs.get(ToolsContances.CMD_ARG_DSWAREFLOAT_IP);
        DSwareApi queryBitmapVolApi = new DSwareApiImpl(dswareFloatIp);
        DSwareBitmapVolumeInfo dswareBitmapVolumeInfo = null;
        
        @SuppressWarnings("unchecked")
        Method mDes;
        String cmdOp = "queryBitmapVolume";
        String volNameForBitmap = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);
        try {
            mDes = cDswareApi.getMethod(cmdOp, String[].class, String.class, int.class);
        } catch (SecurityException | NoSuchMethodException e) {
            logger.error("Get function failed:", e);
            try {
                dswareBitmapVolumeInfo = queryBitmapVolApi.queryBitmapVolume(dsaIp.split(","),
                        volNameForBitmap);
            }
            catch (DSwareException ec) {
                printErrorString(new String[] {
                        "Failed to query specified bitmap volume, cmdArgs:",
                        cmdArgs.toString()});
                printException(ec);
                return ec.getError().getErrorCode();
            }
        
            printdswareBitmapVolumeInfo(dswareBitmapVolumeInfo);        
            return ToolsErrors.TOOLS_SUCCESS;
        }

        //add by wenzhibin end       
        int poolId = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_POOL_ID));
        Object ret = null;
        try {
            ret = mDes.invoke(queryBitmapVolApi, dsaIp.split(","), volNameForBitmap, poolId);
            if (ret != null) {
                dswareBitmapVolumeInfo = (DSwareBitmapVolumeInfo) ret;
            }
        }
        catch (InvocationTargetException e) {
            DSwareException ec = (DSwareException) e.getTargetException();
            printErrorString(new String[] {
                    "Failed to query specified bitmap volume, cmdArgs:",
                    cmdArgs.toString()});
            printException(ec);
            return ec.getError().getErrorCode();
        }
        catch (IllegalArgumentException | IllegalAccessException e) {
            logger.error("Command excute failed:", e);
            return ToolsErrors.TOOLS_ERR_INTERAL;
        }
        
        printdswareBitmapVolumeInfo(dswareBitmapVolumeInfo);        
        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int queryAllBitmapVolume(Map<String, String> cmdArgs)
    {
        String dsaIp = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        int poolID = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_POOL_ID));
        Map<String, DSwareBitmapVolumeInfo> dswareBitmapVolumeInfoMap;
        
        String dswareFloatIp = cmdArgs.get(ToolsContances.CMD_ARG_DSWAREFLOAT_IP);
        DSwareApi queryBitmapVolApi = new DSwareApiImpl(dswareFloatIp);
        try {
            dswareBitmapVolumeInfoMap = queryBitmapVolApi.queryAllBitmapVolume(dsaIp.split(","),
                    poolID);
        }
        catch (DSwareException e) {
            printErrorString(new String[] {
                    "Failed to query specified bitmap volume, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
        
        printdswareBitmapVolumeInfoMap(dswareBitmapVolumeInfoMap);
        
        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    public int expandVolume(Map<String, String> cmdArgs)
    {
        String dsaIps = cmdArgs.get(ToolsContances.CMD_ARG_DSAIP);
        String[] dsaIpArray = dsaIps.split(",");
        String volName = cmdArgs.get(ToolsContances.CMD_ARG_VOL_NAME);

        int newVolSize;
        try {
            newVolSize = Integer.parseInt(cmdArgs.get(ToolsContances.CMD_ARG_VOL_SIZE));
        }
        catch (NumberFormatException e) {
            LOGGER.error("numberformat exception", e);
            return ToolsErrors.TOOLS_ERROR;
        }
        
        try {
            api.expandVolume(dsaIpArray, volName, newVolSize);
        }
        catch (DSwareException e) {
            printErrorString(new String[] {"Failed to update Volume, cmdArgs:",
                    cmdArgs.toString()});
            printException(e);
            return e.getError().getErrorCode();
        }
        
        return ToolsErrors.TOOLS_SUCCESS;
    }
    
    private void printDSwareBackupInfo(DSwareBackupInfo dswareBackupInfo)
    {
        System.out.println("ResultInfo start");
        
        System.out.println("snapSize:" + dswareBackupInfo.getSnapSize());
        System.out.println("bitmapSize:" + dswareBackupInfo.getBitmapSize());
        System.out.println("blockSize:" + dswareBackupInfo.getBlockSize());
        System.out.println("");
        
        System.out.println("ResultInfo end");
    }
    
    private void printdswareBitmapVolumeInfo(
            DSwareBitmapVolumeInfo dswareBitmapVolumeInfo)
    {
        System.out.println("ResultInfo start");
        
        System.out.println("volName:" + dswareBitmapVolumeInfo.getVolName());
        System.out.println("status:" + dswareBitmapVolumeInfo.getStatus());
        System.out.println("volSize:" + dswareBitmapVolumeInfo.getVolSize());
        System.out.println("snapSize:" + dswareBitmapVolumeInfo.getSnapSize());
        System.out.println("snapNameFrom:"
                + dswareBitmapVolumeInfo.getSnapNameFrom());
        System.out.println("snapNameTo:"
                + dswareBitmapVolumeInfo.getSnapNameTo());
        System.out.println("blockSize:" + dswareBitmapVolumeInfo.getBlockSize());
        System.out.println("poolId:" + dswareBitmapVolumeInfo.getPoolId());
        System.out.println("createTime:"
                + dswareBitmapVolumeInfo.getCreateTime());
        System.out.println("");
        
        System.out.println("ResultInfo end");
    }
    
    private void printdswareBitmapVolumeInfoMap(
            Map<String, DSwareBitmapVolumeInfo> dswareBitmapVolumeInfoMap)
    {
        System.out.println("ResultInfo start");
        
        for (DSwareBitmapVolumeInfo dswareBitmapVolumeInfo : dswareBitmapVolumeInfoMap.values()) {
            System.out.println("volName:" + dswareBitmapVolumeInfo.getVolName());
            System.out.println("status:" + dswareBitmapVolumeInfo.getStatus());
            System.out.println("volSize:" + dswareBitmapVolumeInfo.getVolSize());
            System.out.println("snapSize:"
                    + dswareBitmapVolumeInfo.getSnapSize());
            System.out.println("snapNameFrom:"
                    + dswareBitmapVolumeInfo.getSnapNameFrom());
            System.out.println("snapNameTo:"
                    + dswareBitmapVolumeInfo.getSnapNameTo());
            System.out.println("blockSize:"
                    + dswareBitmapVolumeInfo.getBlockSize());
            System.out.println("poolId:" + dswareBitmapVolumeInfo.getPoolId());
            System.out.println("createTime:"
                    + dswareBitmapVolumeInfo.getCreateTime());
            System.out.println("");
        }
        
        System.out.println("ResultInfo end");
    }
    
    private void printVolumeInfos(Map<String, DSwareVolumeInfo> volMap)
    {
        System.out.println("ResultInfo start");
        for (DSwareVolumeInfo volume : volMap.values()) {
            System.out.println("volName:" + volume.getVolName());
            System.out.println("fatherName:" + volume.getFatherName());
            System.out.println("poolId:" + volume.getPoolId());
            System.out.println("realSize:" + volume.getRealSize());
            System.out.println("volSize:" + volume.getVolSize());
            System.out.println("status:" + volume.getStatus());
            System.out.println("createTime:" + volume.getCreateTime());
            System.out.println("");
        }
        System.out.println("ResultInfo end");
    }
    
    private void printSnapshotInfos(Map<String, DSwareSnapInfo> snapMap)
    {
        System.out.println("ResultInfo start");
        for (DSwareSnapInfo snap : snapMap.values()) {
            System.out.println("snapName:" + snap.getSnapName());
            System.out.println("fatherName:" + snap.getFatherName());
            System.out.println("poolId:" + snap.getPoolId());
            System.out.println("realSize:" + snap.getRealSize());
            System.out.println("snapSize:" + snap.getSnapSize());
            System.out.println("status:" + snap.getStatus());
            System.out.println("deletePrority:" + snap.getDeletePrority());
            System.out.println("createTime:" + snap.getCreateTime());
            System.out.println("");
        }
        System.out.println("ResultInfo end");
    }
    
    private void printPoolInfos(Map<Integer, DSwarePoolInfo> poolMap)
    {
        System.out.println("ResultInfo start");
        for (DSwarePoolInfo pool : poolMap.values()) {
            System.out.println("poolId:" + pool.getPoolId());
            System.out.println("totalCapacity:" + pool.getTotalCapacity());
            System.out.println("usedCapcity:" + pool.getUsedCapcity());
            System.out.println("allocCapacity:" + pool.getAllocCapacity());
            System.out.println("");
        }
        System.out.println("ResultInfo end");
    }

    private void printErrorString(String args[])
    {
        LOGGER.error(Arrays.toString(args));
    }
    
    private void printException(Exception e)
    {
        LOGGER.error("error:", e);
    }
}
