/*
版权所有 (c) 华为技术有限公司 2012-2018

公共函数实现
*/

#include "XML.h"

#ifdef __cplusplus
extern "C" {
#endif

// 十六进制字符串转换为字节流  
void HexStrToByte(const char* source, unsigned char* dest, int sourceLen)  
{  
    short i;  
    unsigned char highByte, lowByte;  

    for (i = 0; i < sourceLen; i += 2)  
    {  
        highByte = toupper(source[i]);  
        lowByte  = toupper(source[i + 1]);  

        if (highByte > 0x39)  
        {
            highByte -= 0x37;
        }
        else
        {
            highByte -= 0x30;
        }

        if (lowByte > 0x39)
        {
            lowByte -= 0x37;
        }
        else
        {
            lowByte -= 0x30;
        }

        dest[i / 2] = (highByte << 4) | lowByte;  
    }  
    return ;  
}  

int c2i(char ch)  
{  
    // 如果是数字，则用数字的ASCII码减去48, 如果ch = '2' ,则 '2' - 48 = 2  
    if (isdigit(ch))  
    {
        return ch - 48;  
    }

    // 如果是字母，但不是A~F,a~f则返回  
    if ( ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z' )  
    {
        return -1;
    }

    // 如果是大写字母，则用数字的ASCII码减去55, 如果ch = 'A' ,则 'A' - 55 = 10  
    // 如果是小写字母，则用数字的ASCII码减去87, 如果ch = 'a' ,则 'a' - 87 = 10  
    if (isalpha(ch))  
    {
        return isupper(ch) ? ch - 55 : ch - 87;  
    }

    return -1;  
}  

s64 Hex2Dec(u8 *hex)  
{  
    int len;  
    s64 num = 0;  
    s64 temp;  
    s64 bits;  
    int i;  

    // 此例中 hex = "1de" 长度为3, hex是main函数传递的  
    len = strlen((char *)hex);  

    for (i = 0, temp = 0; i < len; i++, temp = 0)
    {  
        //去掉空格
        if(*(hex + i) == ' ')
        {      
            continue;
        }


        // 第一次：i=0, *(hex + i) = *(hex + 0) = '1', 即temp = 1  
        // 第二次：i=1, *(hex + i) = *(hex + 1) = 'd', 即temp = 13  
        // 第三次：i=2, *(hex + i) = *(hex + 2) = 'd', 即temp = 14  
        temp = c2i( *(hex + i) );  
        // 总共3位，一个16进制位用 4 bit保存  
        // 第一次：'1'为最高位，所以temp左移 (len - i -1) * 4 = 2 * 4 = 8 位  
        // 第二次：'d'为次高位，所以temp左移 (len - i -1) * 4 = 1 * 4 = 4 位  
        // 第三次：'e'为最低位，所以temp左移 (len - i -1) * 4 = 0 * 4 = 0 位  
        bits = (len - i - 1) * 4;  
        temp = temp << bits;  

        // 此处也可以用 num += temp;进行累加  
        num = num | temp;  
    }  

    // 返回结果  
    return num;  
}  

u16 BigLittleSwap16(u16 A , int is_swap)      
{
    if (is_swap)
    {   
        return ((((u16)(A) & 0xff00) >> 8) | (((u16)(A) & 0x00ff) << 8));
    }

    return A;
}

u32 BigLittleSwap32(u32 A, int is_swap)           
{
    if (is_swap)
    {   
        return ((((u32)(A) & 0xff000000) >> 24) | (((u32)(A) & 0x00ff0000) >> 8) | (((u32)(A) & 0x0000ff00) << 8) | (((u32)(A) & 0x000000ff) << 24));
    }

    return A;
}

u64 BigLittleSwap64(u64 A, int is_swap)
{
    if (is_swap)
    {   
        return ((((u64)(A) & 0xff000000) >> 24) | (((u64)(A) & 0x00ff0000) >> 8) | (((u64)(A) & 0x0000ff00) << 8) | (((u64)(A) & 0x000000ff) << 24));
    }

    return A;
}

int GetLengthFromSize(int size)
{
    int length = 0;

    if (size <= 8)
    {   
        length = 1;
    }
    else if (size <= 16)
    {   
        length = 2;
    }
    else if (size <= 24)
    {   
        length = 3;
    }
    else if (size <= 32)
    {   
        length = 4;
    }
    else
    {   
     length = 8;
    }

    return length;
}

// 去掉空格
int TrimSpace(u8 *inbuf, u8 *outbuf)
{
    u8 *in = inbuf;
    u8 *out = outbuf;

    int ret = 0;
    int inLen = strlen((char *)in);

    if (!inbuf || !outbuf)
    {
        ret = -1;
        return ret;
    }
    else
    {
        int i = 0;
        ret = 1;
        for (i = 0; i < inLen; i++)
        {
            if (in[i] != ' ')
            {
                *out++ = in[i];
            }
        }
    }

    return ret;
}

u32 FromIpstrToUint(char* ip )
{
    char str_ip_index[4] = {'\0'};
    unsigned int ip_int,ip_add = 0;
    int j = 0, a = 3;	
    unsigned int i = 0;

    for (i = 0;i <= strlen(ip); i++) // 要用到'\0'
    {
        if (ip[i] == '\0' || ip[i] == '.')
        {			

            ip_int = atoi(str_ip_index);	

            if (ip_int > 255)
            {
                printf("IP address error\n");
                system("pause");
                return 0;				
            }

            ip_add += (ip_int * ((unsigned int)pow(256.0, a)));
            a--;

            Hw1Memset(str_ip_index, 0, sizeof(str_ip_index));
            j = 0;
            continue;
        }

        str_ip_index[j] = ip[i];
        j++;
    }
    return ip_add;
}

char* MyMemmem(char * a, int alen, char* b, int blen)
{
    int i, j;

    if (alen < blen)
    {
        return NULL;
    }

    for (i = 0; i <= alen - blen; ++i)
    {
        for (j = 0; j < blen; ++j)
        {
            if (a[i + j] != b[j])
            {
                break;
            }
        }
        if (j >= blen)
        {
            return a + i;
        }
    }
    return NULL;
}

unsigned short CheckSum(unsigned short *buf, int nword)
{
	 unsigned long sum;
	 for(sum = 0; nword > 0; nword--)
	 {
	  sum += htons(*buf);
	  buf++;
	 }
	 sum = (sum>>16) + (sum&0xffff);
	 sum += (sum>>16);
	 return ~sum;
}

static const unsigned int crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

unsigned int CheckSum32( const unsigned char *buf, unsigned int size)
{
    unsigned int i, crc;
    crc = 0xFFFFFFFF;

    for (i = 0; i < size; i++)
    {
        crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFF;
}

/**********************************************************
bit操作函数
**********************************************************/
s64 GetBufBitValue(int position, u8* buf)
{
    u8* temp_buf = buf;
    int ret = 0;

    if (temp_buf[position / 8] & ((u8)1 << (position % 8)))
    {   
        ret = 1;
    }

    return ret;
}

void SetBufBitValue(int position, u8* buf, u8 value)
{
    u8* temp_buf = buf;
    if (value)
    {   
        temp_buf[position / 8] = temp_buf[position / 8] | ((u8)1 << (position % 8));
    }
    else
    {   
        temp_buf[position / 8] = temp_buf[position / 8] & (~((u8)1 << (position % 8)));
    }

    return ;
}

s64 GetNumberBinValue(int position, int size, int tsigned, u8* buf)
{
    s64 temp = 0;
    u8* aaaa = (u8*)&temp;
    int i, k, l;

    // 按bit取值
    for (i = 0; i < size; i++)
    {
        // 大bit端
        if (g_isSwap)
        {
            // 复杂致死
            k = GetBufBitValue(position + i, buf);
            l = ((size - i - 1) / 8) * 8 + (((8 - (size - i) % 8) == 8) ? 0 : (8 - (size - i) % 8));
            SetBufBitValue(l, aaaa, k);
        }
        // small端
        else
        {
            // 复杂致死
            k = GetBufBitValue(i, buf);
            l = ((size-i-1 ) / 8) * 8 + (((8 - (size - i) % 8) == 8) ? 0 : (8 - (size - i) % 8));
            SetBufBitValue(i, aaaa, k);
        }
    }

    return temp;
}

void SetNumberBinValue(int position, int size, int tsigned, u8* buf, s64 bbbb)
{
    u8* aaaa = (u8*)&bbbb;
    int i, k, l;

    // 按bit取值
    for (i = 0; i < size; i++)
    {
        // 大bit端
        if (g_isSwap)
        {
            // 复杂致死
            k = GetBufBitValue(position + i, aaaa);
            l = ((size - i - 1) / 8) * 8 + (((8 - (size - i) % 8) == 8) ? 0 : (8 - (size - i) % 8));
            SetBufBitValue(l, buf, k);
        }
        // small端
        else
        {
            // 复杂致死
            k = GetBufBitValue(i, aaaa);
            l = ((size - i - 1) / 8) * 8 + (((8 - (size - i) % 8) == 8) ? 0 : (8 - (size - i) % 8));
            SetBufBitValue(i, buf, k);
        }
    }

    return ;
}

// posion 为0为最高位
s64 GetSomeBitValue(s64 temp, int posion, int size, int allsize)
{
    s64 value = 0;
    int i = 0;
    int temp_pos = allsize - posion - size;

    for(i = temp_pos; i < (temp_pos + size); i++)
    {
        s64 temp1 = temp & ((s64)1 << i);
        value |= temp1;
    }

    return (value >> temp_pos);
}

void SetSomeBitValue(s64 *value, s64 temp, int posion, int size, int allsize)
{
    int i = 0;

    for(i = 0; i < (size); i++)
    {
        s64 temp1 = temp & ((s64)1 << i);
        *value |= (temp1 << (allsize - posion - size));
    }
    return ;
}

/**********************************************************
element操作函数
**********************************************************/
char * GetConfigValueByKey(char* key) 
{
    // 优先使用函数配置的
    int i = 0;
    for (i = 0; i < g_configNum; i++)
    {
        if (strcmp(g_configKey[i], key) == 0)
        {
            return g_configValue[i];
        }
    }

    return NULL;
}

char* strip_config(char* value)
{
    char *stringValue = value;

    char* pos1 = NULL;
    char* pos2 = NULL;

    if ((stringValue != NULL) && (strlen(stringValue) > 4))
    {   
        pos1 = MyMemmem(stringValue, strlen(stringValue), "##", 2);
    }

    if (pos1 != NULL)
    {
        char *key = malloc(256);
        g_testcaseMemory[g_testcaseMemoryCount++] = key;
        pos2 = MyMemmem(pos1 + 2, strlen(pos1 + 2), "##", 2);

        memcpy(key, pos1 + 2, pos2 - (pos1 + 2));
        key[pos2 - (pos1 + 2)] = 0;

        char *key_value = GetConfigValueByKey(key);

        // 组装租后的值
        memcpy(key, stringValue, pos1 - stringValue);
        memcpy(key + (pos1 - stringValue), key_value, strlen(key_value));
        memcpy(key + (pos1 - stringValue) + strlen(key_value), pos2 + 2,strlen(pos2 + 2));

        key[pos1 - stringValue + strlen(key_value) + strlen(pos2 + 2) ] = 0;

        // 使用递归，基本能解决所有问题了
        return strip_config(key);
    }
    else
    {   
        return stringValue;
    }
}

char* XmlGetGetProp(hw_xmlNodePtr node, char* name)
{

    char *stringValue = (char *)HW1xmlGetProp(node, (hw_xmlChar *)name);
    g_testcaseMemory[g_testcaseMemoryCount++] = (char*)stringValue;

    return strip_config(stringValue);
}

char* XmlGetProperty(hw_xmlNodePtr node, char* property_name)
{
    char* tempValue = XmlGetGetProp(node, property_name);

    return tempValue;
}


hw_xmlNodePtr XmlGetTestModelByName(hw_xmlNodePtr par, char* name)
{
    hw_xmlNodePtr cur = NULL;
    cur = par->children;  
    while (cur != NULL) 
    {  
        if (strcmp((char*)cur->name,"Test") == 0)
        {
            if (strcmp(XmlGetGetProp(cur, (char *)"name"), name) == 0)
            {
                return cur;
            }
        }
        cur = cur->next;  
    }

    return cur;
}

hw_xmlNodePtr getIncludeNode(hw_xmlNodePtr par, char* ns_name)
{
    hw_xmlNodePtr child = NULL;
    child = par->children;  

    SXMLElement* temp;

    while (child != NULL) 
    {
        // 遍历所有DataModel，一次性解析完成，以后就不需要了
        if (strcmp((char*)child->name,"Include") == 0)
        {
            temp = child->_private;

            if (strcmp(temp->includeNs, ns_name) == 0)
            {
                return temp->includeRootDoc;
            }
        }
        child = child->next;  
    }
    return NULL;
}

hw_xmlNodePtr XmlGetStateModelNodeByName(hw_xmlNodePtr par, char* name)
{
    char temp_name[256];
    char* include_name;
    char* state_name;

    state_name = name;

    strcpy(temp_name, name);
    char* rel = MyMemmem(temp_name, strlen(temp_name), ":", 1); 

    if (rel != NULL)
    {
        temp_name[rel -temp_name] = 0;
        include_name = temp_name;

        state_name = rel+1;

        par = getIncludeNode(par, include_name);
    }

    hw_xmlNodePtr cur = NULL;
    cur = par->children;  
    while (cur != NULL) 
    {  
        if (strcmp((char*)cur->name,"StateModel") == 0)
        {
            char* aaaaa = XmlGetGetProp(cur, (char *)"name");

            if (strcmp(aaaaa,state_name) == 0)
            {
                return cur;
            }
        }
        cur = cur->next;  
    }

    return cur;
}

hw_xmlNodePtr XmlGetStateModelDocByName(hw_xmlNodePtr par, char* name)
{
    char temp_name[256];
    char* include_name;

    strcpy(temp_name, name);
    char* rel = MyMemmem(temp_name, strlen(temp_name), ":", 1); 

    if (rel != NULL)
    {
        temp_name[rel - temp_name] = 0;
        include_name = temp_name;

        par = getIncludeNode(par, include_name);
    }

    return par;
}

hw_xmlNodePtr XmlGetDataModelByName(hw_xmlNodePtr par, char* name)
{
    char temp_name[256];
    char* include_name;
    char* state_name;

    state_name = name;

    strcpy(temp_name, name);
    char *rel = MyMemmem(temp_name, strlen(temp_name), ":", 1); 

    if (rel != NULL)
    {
        temp_name[rel -temp_name] = 0;
        include_name = temp_name;

        state_name = rel + 1;

        par = getIncludeNode(par, include_name);
    }

    hw_xmlNodePtr cur = NULL;
    cur = par->children;  
    while (cur != NULL) 
    {  
        if (strcmp((char*)cur->name,"DataModel") == 0)
        {
            char *aaaaa = XmlGetGetProp(cur, (char *)"name");

            if(strcmp(aaaaa,state_name) == 0)
            {
                return cur;
            }
        }
        cur = cur->next;  
    }

    return cur;
}

SMutatorElement*  MutatorElementFoundByXpathName(char* name, SMutatorElement* parent)
{
    SMutatorElement* temp = parent;
    For_Tree_Start(temp)
    {
        if (strcmp(temp->xpathName, name) == 0)
        {      
            return temp;	
        }
    }
    For_Tree_End(temp)

    return NULL;
}

int  BinElementGetOffsetLength(SBinElement* parent, SBinElement* padding)
{
    int length = 0;
    SBinElement* temp = parent;

    For_Tree_Start(temp)
    {
        if (temp == padding)
        {      
            return length;
        }

        //要排除掉Flag，因为Flags算长度了
        if (strcmp(temp->mutatorElement->xmlElement->typeName, "Flag") != 0)
        {
            length = temp->mutaterLength + length;
        }
    }
    For_Tree_End(temp)

    return length;
}

/**********************************************************
tree查找函数
**********************************************************/
int BinElementIsSameElement(SBinElement* temp, char* name)
{
    if (temp->mutatorElement->xmlElement->name != NULL)
    {
        if (strcmp(temp->mutatorElement->xmlElement->name, name) == 0 )
        {
            return 1;
        }
    }

    if (temp->mutatorElement->xmlElement->isHasRef == 1)
    {
        SXMLElement* temp_xml = (SXMLElement *)temp->mutatorElement->xmlElement->xmlRef->_private;
        if (strcmp(temp_xml->name, name) == 0 )
        {
            return 1;
        }

        if (temp_xml->isHasRef == 1)
        {
            SXMLElement *temp_xml1 = temp_xml->xmlRef->_private;
            if (strcmp(temp_xml1->name, name) == 0 )
            {
                return 1;
            }

            if (temp_xml1->isHasRef == 1)
            {
                SXMLElement* temp_xml2 = temp_xml1->xmlRef->_private;
                if (strcmp(temp_xml2->name, name) == 0 )
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

int MutatorElementIsSameElement(SMutatorElement* temp, char* name)
{
    if (temp->xmlElement->name != NULL)
    {
        if (strcmp(temp->xmlElement->name, name) == 0 )
        {
            return 1;
        }
    }

    if (temp->xmlElement->isHasRef == 1)
    {
        SXMLElement *temp_xml = (SXMLElement *)temp->xmlElement->xmlRef->_private;
        if (strcmp(temp_xml->name, name) == 0 )
        {
            return 1;
        }

        if (temp_xml->isHasRef == 1)
        {
            SXMLElement* temp_xml1 = temp_xml->xmlRef->_private;
            if (strcmp(temp_xml1->name ,name) == 0 )
            {
                return 1;
            }

            if (temp_xml1->isHasRef == 1)
            {
                SXMLElement* temp_xml2 = temp_xml1->xmlRef->_private;
                if (strcmp(temp_xml2->name, name) == 0)
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

// 先查自己，然后兄弟:然后父亲，父亲的兄弟，爷爷，爷爷的兄弟... ...
SBinElement* BinElementFoundRelationofByName(SBinElement* temp1, char* name)
{
    SBinElement* temp = temp1;
    int ret = 0;

    //
    while (temp != NULL)
    {
        ret = BinElementIsSameElement(temp, name);

        if (ret == 1)
        {      
            return temp;
        }

        SBinElement* next = temp->next;

        while (next != NULL)
        {
            ret = BinElementIsSameElement(next, name);

            if (ret == 1)
            {         
                return next;
            }
            next = next->next;
        }

        temp = temp->parent;
    }


    // 都找不到，则找孙子之类的*************************
    SBinElement* parent = temp1->parent;

    while (parent != NULL)
    {
        ret = BinElementIsSameElement(parent, name);

        if (ret == 1)
        {      
            return parent;
        }

        SBinElement* temp_1 = parent;
        For_Tree_Start(temp_1)
        {
            ret = BinElementIsSameElement(temp_1, name);

            if (ret == 1)
            {         
                return temp_1;
            }
        }
        For_Tree_End(temp_1)

        parent = parent->parent;
    }

    return NULL;
}

// 先查自己，然后兄弟:然后父亲，父亲的兄弟，爷爷，爷爷的兄弟... ...
SMutatorElement* MutatorElementFoundRelationofByName(SMutatorElement* temp1, char* name)
{
    SMutatorElement* temp = temp1;
    int ret = 0;

    // 查找父亲祖父之类的有没有，不过ref
    while (temp != NULL)
    {
        ret = MutatorElementIsSameElement(temp, name);

        if (ret == 1)
        {      
            return temp;
        }


        SMutatorElement* next = temp->next;

        while (next != NULL)
        {
            ret = MutatorElementIsSameElement(next, name);

            if (ret == 1)
            {         
                return next;
            }

            next = next->next;
        }

        temp = temp->parent;
    }

    // 都找不到，则找孙子之类的*************************
    SMutatorElement * parent = temp1->parent;

    while (parent != NULL)
    {
        ret = MutatorElementIsSameElement(parent, name);

        if (ret == 1)
        {      
            return parent;
        }

        SMutatorElement* temp_1 = parent;
        For_Tree_Start(temp_1)
        {
            ret = MutatorElementIsSameElement(temp_1,name);

            if (ret == 1)
            {         
                return temp_1;
            }
        }
        For_Tree_End(temp_1)

        parent = parent->parent;
    }

    return NULL;
}

// 通过xpathName查找最近的元素，原则见下一行注释
// 先查找自己的父亲，然后是兄弟，爷爷，父亲的兄弟.... ....
SBinElement* BinElementFoundRecentlyByXpathName(SBinElement* temp, char* xpathName)
{
    SBinElement* parent = temp ;//没有用父亲

    while (parent != NULL)
    {
        if (parent->mutatorElement->xpathName != NULL)
        {
            if (strcmp(parent->mutatorElement->xpathName,xpathName) == 0 )
            {
                return parent;
            }
        }

        SBinElement* temp_1 = parent;
        For_Tree_Start(temp_1)
        {
            if (temp_1->mutatorElement->xpathName != NULL)
            {         
                if (strcmp(temp_1->mutatorElement->xpathName, xpathName) == 0)
                {
                    return temp_1;
                }
            }
        }
        For_Tree_End(temp_1)

        parent = parent->parent;
    }

    return parent;
}

// 找到最后一个元素
SMutatorElement* MutatorElementFoundLast(SMutatorElement* temp)
{
    while (temp->lastChildren != NULL)
    {
        temp = temp->lastChildren;
    }

    return temp;
}

// 判断ppp是否是temp的祖先
// 先查找自己的父亲，爷爷，祖父... ...
int MutatorElementIsParent(SMutatorElement* temp, SMutatorElement* ppp)
{
    SMutatorElement* parent = temp->parent;

    while (parent != NULL)
    {
        if (parent == ppp)
        {
            return 1;
        }

        parent = parent->parent;
    }

    return 0;
}

// 将一个mutator元素插入到父亲这里
void MutatorElementAddChildren(SMutatorElement* temp_parent, SMutatorElement* temp)
{
    //如果儿子为0，则添加第一个儿子
    if (temp_parent->children == NULL)
    {
        temp_parent->children = temp;
        temp_parent->lastChildren = temp;
        temp->parent = temp_parent;
    }
    else
    {
        //如果已经有xpath相同的，直接替换
        SMutatorElement * temp2 = MutatorElementFoundByXpathName(temp->xpathName, temp_parent);
        if (temp2 != NULL)
        {
            SMutatorElement* parent = temp2->parent;
            SMutatorElement* next = temp2->next;
            SMutatorElement* prev = temp2->prev;


            //除了不继承儿子
            temp->next = temp2->next;
            temp->prev= temp2->prev;
            temp->parent= temp2->parent;

            if (parent->children == temp2)
            {
                parent->children = temp;
            }

            if (parent->lastChildren == temp2)
            {
                parent->lastChildren = temp;
            }

            if (temp2->next != NULL)
            {
                next->prev = temp;
            }

            if (temp2->prev != NULL)
            {
                prev->next = temp;
            }

            //名字保留原来的名字,避免出现.Core.Core.Type的尴尬局面
            temp->xmlElement->name = temp2->xmlElement->name;

            //原来的需要释放:)
            return;	
        }


        //否则添加到结尾	
        SMutatorElement * temp1 = temp_parent->lastChildren;
        temp1->next = temp;
        temp->prev = temp1;

        temp_parent->lastChildren = temp;
        temp->parent = temp_parent;
    }
}


// 将一个bin元素插入到父亲这里，作为最后一个儿子
void BinElementAddChildren(SBinElement* tempParent, SBinElement* temp)
{
    //如果儿子为0，则添加第一个儿子
    if(tempParent->children == NULL)
    {
        tempParent->children = temp;
        tempParent->lastChildren = temp;
        temp->parent = tempParent;
    }
    else
    {
        SBinElement* temp1 = tempParent->lastChildren;
        temp1->next = temp;
        temp->prev = temp1;

        tempParent->lastChildren = temp;
        temp->parent = tempParent;
    }
}

// 从bin树中删除一个儿子
void BinElementDelChildren(SBinElement* tempParent, SBinElement* temp)
{
    SBinElement* parent =temp->parent;
    SBinElement* next = temp->next;
    SBinElement* prev = temp->prev;

    if (parent->children == temp)
    {
        parent->children = next;
    }

    if (parent->lastChildren == temp)
    {
        parent->lastChildren = prev;
    }

    if (next != NULL)
    {   
        next->prev = prev;
    }

    if (prev != NULL)
    {   
        prev->next = next;
    }
}

#ifdef __cplusplus
}
#endif
