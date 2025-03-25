#include "afs/LVMVolumeGroupManager.h"
#include "afs/AfsError.h"
#include "afs/LogMsg.h"
#include "afs/LogicalVolume.h"

/* * @brief 添加卷组
 *
 * @param uuid 唯一标识符
 * @param name 卷组名字
 * @param seq  卷组序列号
 * @param size extent的大小
 *
 * @return 卷组指针
 */
volumeGroup *LVMVolumeGroupManager::addVolgroup(string uuid, string name, int32_t seq, int32_t size)
{
    volumeGroup *vol = new volumeGroup(uuid, name, seq, size);
    if (!vol) {
        return NULL;
    }

    groupList.push_back(vol);

    return vol;
}

/**
 * @brief 根据uuid查找卷组
 *
 * @param uuid 唯一标识符
 *
 * @return 返回卷组
 *
 */
volumeGroup *LVMVolumeGroupManager::findVolgroup(string &uuid)
{
    volumeGroup *grp = NULL;
    list<volumeGroup *>::iterator iter;

    for (iter = groupList.begin(); iter != groupList.end(); ++iter) {
        if (NULL == (*iter)) {
            AFS_TRACE_OUT_ERROR("The space is NULL");
            return NULL;
        }

        grp = (*iter);
        if (grp->m_uuid.compare(uuid) == 0) {
            return grp;
        }
    }

    return NULL;
}

/**
 * @brief 获取卷组元数据的BitMap
 *
 * @param bitmap 需要设置的BitMap
 *
 * @return AFS_SUCCESS
 *
 */
int32_t LVMVolumeGroupManager::getBitmap(vector<BitMap *> &bitmap_vect)
{
    list<volumeGroup *>::iterator vg_iter;
    volumeGroup *pVg = NULL;
    int32_t ret = AFS_SUCCESS;

    // 设置卷组中关于PV的元数据BitMap
    for (vg_iter = groupList.begin(); vg_iter != groupList.end(); ++vg_iter) {
        if (NULL == (*vg_iter)) {
            AFS_TRACE_OUT_ERROR("The space is NULL");
            return AFS_ERR_LVM_PART;
        }
        pVg = (*vg_iter);
        ret = pVg->getBitmap(bitmap_vect);
        if (ret < 0) {
            AFS_TRACE_OUT_ERROR("Can'nt support format");
            return ret;
        }
    }

    return ret;
}

/**
 * @brief 根据分区识别号，取得相应lv
 *
 * @param index 分区识别号
 *
 * @return 返回lv
 * NULL 未找到
 */
logicalVolume *LVMVolumeGroupManager::findPartIdMapLv(int32_t index)
{
    list<volumeGroup *>::iterator grp_iter = groupList.begin();
    vector<logicalVolume *>::iterator lv_iter;
    bool result = false;

    // 遍历卷组
    for (; grp_iter != groupList.end(); ++grp_iter) {
        if (NULL == (*grp_iter)) {
            AFS_TRACE_OUT_ERROR("The space is NULL");
            return NULL;
        }

        lv_iter = (*grp_iter)->m_lvolumes.begin();
        for (; lv_iter != (*grp_iter)->m_lvolumes.end(); ++lv_iter) {
            if (NULL == (*lv_iter)) {
                AFS_TRACE_OUT_ERROR("The space is NULL");
                return NULL;
            }

            result = (*lv_iter)->matchLvPartId(index);
            if (result) {
                AFS_TRACE_OUT_INFO("find the logical volume whose m_part_id = %d", index);
                return (*lv_iter);
            }
        }
    }

    return NULL;
}
