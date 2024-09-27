#ifndef BITMAP_H
#define BITMAP_H

#include <math.h>
#include <vector>
#include "afs/ImgReader.h"

#define LVMNODATA (-2)

const uint32_t AFS_DEFAULT_BLOCK_SIZE = 512;        // Bitmap默认位代表Size
const uint32_t AFS_MX_BLOCK_SIZE = 1 * 1024 * 1024; // Bitmap位代表Size最大值(1M)
const uint8_t AFS_BITS_PER_CHAR = 8;                // Bitmap每个字节8位

/**
 * @brief BitMap类
 */
class BitMap {
public:
    BitMap();
    ~BitMap();

    int32_t initBitMap(uint64_t blocknum);
    void bitmapSetBlocksize(uint32_t blocksize);
    uint32_t bitmapGetBlocksize()
    {
        return m_blocksize;
    }

    int32_t bitmapSetMapAddr(imgReader *reader, uint64_t index);
    int32_t bitmapSet(uint64_t index);

    int32_t bitmapUnsetMapAddr(imgReader *reader, uint64_t index);

    int32_t bitmapGet(uint64_t index);

    int32_t bitmapSetRangeMapAddr(uint64_t start, uint64_t length, uint8_t flag);
    int32_t bitmapSetRangeMapAddr(imgReader *reader, uint64_t start, uint64_t length, vector<BitMap *> &bitmap_vect,
        uint8_t flag);

    int32_t bitmapSetRange(uint64_t start, uint64_t length, uint8_t flag);
    int32_t bitmapSetRangeSwapMapAddr(imgReader *reader, uint64_t start, uint64_t length, uint8_t flag,
        vector<BitMap *> &bitmap_vect);

    /*
     * del the index from 1 to 0
     */
    char *getbitmap()
    {
        return this->m_bitmap;
    }
    uint64_t getsize()
    {
        return m_bufsize;
    }

    void setImageSize(uint64_t imageSize)
    {
        m_imagesize = imageSize;
    }
    uint64_t getImageSize()
    {
        return m_imagesize;
    }

    int64_t bitmapSave(const char *filename);
    int64_t bitmapLoad(const char *filename);

    int32_t bitmapConvert(uint32_t bytes_per_bit, BitMap &bitmap);
    int32_t bitmapConvert(imgReader *reader, uint32_t bytes_per_bit, vector<BitMap *> &bitmap_vect);

    int32_t bitmapConvertFile(uint32_t bytes_per_bit, BitMap &newbitmap);

#ifdef CPPUNIT_MAIN
protected:
#else
private:
#endif
    char *m_bitmap;
    uint32_t m_blocksize;
    uint64_t m_bufsize;
    uint64_t m_blocknum;
    uint64_t m_imagesize;

    BitMap(const BitMap &bitmap);
    BitMap &operator = (const BitMap &bitmap);

    int32_t bitmapConvertFile_1(uint32_t bytes_per_bit, BitMap &newbitmap);

    int32_t bitmapSetMapAddr(uint64_t index, uint8_t value);
    int32_t bitmapSet(uint64_t index, uint8_t value);
    int32_t bitmapConvert_1(imgReader *img_reader, uint32_t bytes_per_bit, vector<BitMap *> &bitmap_vect);
    int32_t bitmapConvert_2(imgReader *img_reader, uint32_t bytes_per_bit, vector<BitMap *> &bitmap_vect);
    int32_t bitmapSetRangeByChunk(uint64_t bitmap_curr_pos, uint64_t real_start_pos, uint32_t split_bit,
        uint8_t bits_dispose, vector<BitMap *> &bitmap_vect, int32_t disk_id);
    uint32_t getBitCount(uint32_t max_storage_size, uint8_t is_free_disk);
    uint8_t isZeroBits(uint64_t current_index, uint8_t bits_count_per);
};

#endif // BITMAP_H_INCLUDED
