package openbackup.data.access.framework.core.dao;

import openbackup.data.access.framework.core.entity.CopiesAntiRansomware;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.springframework.stereotype.Repository;

/**
 * COPIES_ANTI_RANSOMWARE表Mapper
 *
 * @author f00809938
 * @since 2023-05-30
 * @version OceanCyber 300 1.2.0
 **/
@Repository
public interface CopiesAntiRansomwareDao extends BaseMapper<CopiesAntiRansomware> {
    /**
     * COPY_ID
     */
    String COPY_ID = "COPY_ID";

    /**
     * IO_DETECT
     */
    String IO_DETECT_TYPE = "IO_DETECT";

    /**
     * 根据快照id判断是否为实时侦测快照
     *
     * @param copyId copyId
     * @return boolean
     */
    default boolean isIoDetectCopy(String copyId) {
        CopiesAntiRansomware copiesAntiRansomware = selectById(copyId);
        if (copiesAntiRansomware == null) {
            return false;
        }
        return IO_DETECT_TYPE.equals(copiesAntiRansomware.getGenerateType());
    }

    /**
     * 根据快照id判断是否进入侦测
     *
     * @param copyId copyId
     * @return boolean
     */
    default boolean isDetectStart(String copyId) {
        CopiesAntiRansomware copiesAntiRansomware = selectById(copyId);
        return copiesAntiRansomware != null;
    }

    /**
     * 根据快照id判断是否检测完成
     *
     * @param copyId copyId
     * @return boolean
     */
    default boolean isDetectComplete(String copyId) {
        CopiesAntiRansomware copiesAntiRansomware = selectById(copyId);
        if (copiesAntiRansomware == null) {
            return false;
        }
        return copiesAntiRansomware.getStatus() != 0 && copiesAntiRansomware.getStatus() != 1;
    }
}

