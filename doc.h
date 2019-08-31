#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<vector>

using namespace std;

typedef unsigned char byte;

/*****************************************************************************
复合文档二进制文件头结构定义
*******************************************************************************/
struct Compound_Binary_File_Header
{
	byte compound_file_id[8]; //0 8 复合文档文件标识：D0H CFH 11H E0H A1H B1H 1AH E1H
	byte class_id[16]; //8 16 此文件的唯一标识(可全部为0)
	byte minor_version[2]; //24 2 文件格式修订号 (一般为003EH)
	byte major_version[2]; //26 2 文件格式版本号(一般为0003H)
	byte byte_order[2]; //28 2 字节顺序规则标识：FEH FFH = Little-Endian     FFH FEH = Big-Endian
	byte size_of_a_master_sector[2]; //30 2 复合文档中sector的大小(ssz)，以2的幂形式存储, sector实际大小为s_size= 2ssz 字节(一般为9即512字节, 最小值为7即128字节)
	byte size_of_a_short_sector[2]; //32 2 short-sector的大小，以2的幂形式存储, short-sector实际大小为s_s_size = 2sssz 字节(一般为6即64字节，最大为sector的大小)
	byte reserved[2]; //34 2 Not used
	byte reserved1[4]; //36 4 Not used
	byte reserved2[4]; //40 4 Not used
	byte number_of_sector_SAT[4]; //44 4 用于存放扇区配置表（SAT）的sector总数
	byte first_sector_of_directory_stream[4]; //48 4 用于存放目录流的第一个sector的SID
	byte signature[4]; //52 4 Not used
	byte min_size_of_a_standard_stream[4]; //56 4 标准流的最小大小(一般为4096 bytes), 小于此值的流即为短流
	byte first_sector_of_short_sector_SSAT[4]; //60 4 用于存放短扇区配置表（SSAT）的第一个sector的SID,或为–2 (End Of Chain SID)如不存在
	byte total_number_of_short_sector_SSAT[4]; //64 4 用于存放短扇区配置表（SSAT）的sector总数
	byte first_sector_of_master_sector_MSAT[4]; //68 4 用于存放主扇区配置表（MSAT）的第一个sector的SID,或为–2 (End Of Chain SID) 若无附加的sectors
	byte total_number_of_master_sector_MSAT[4]; //72 4 用于存放主扇区配置表（MSAT）的sector总数
	byte first_109_master_sector_MSAT[436]; //76 436 存放主扇区配置表（MSAT）的第一部分，包含109个SID
};

/*****************************************************************************
复合文档目录入口结构定义
*******************************************************************************/
struct Storage_Directory_Entry
{
	byte name_of_entry[64]; // [000H,64] 64字节. 此入口的名字（字符数组）, 一般为16位的Unicode字符,以0结束。(因此最大长度为31个字符)
	byte length_of_entry[2]; // [040H,02] 用于存放名字的区域的大小，包括结尾的0 (如：一个名字有6个字符则此值为(6+1)?2 = 14)
	byte type_of_entry[1]; // [042H,01] 入口类型: 00H = Empty  03H = LockBytes (unknown)   01H = User storage    04H = Property(unknown) 02H = User stream     05H = Root storage
	byte node_colour_of_entry[1]; // [043H,01] 此入口的节点颜色: 00H = Red    01H = Black
	byte did_of_left_sibling[4]; // [044H,04] 其左节点的DEID (若此入口为一个user storage or stream) ,若没有左节点就为-1
	byte did_of_right_sibling[4]; // [048H,04] 其右节点的DEID (若此入口为一个user storage or stream) ,若没有右节点就为-1
	byte did_of_child[4]; // [04CH,04] 其成员红黑树的根节点的DEID (若此入口为storage), 其他为-1
	byte class_id[16]; // [050H,16] 唯一标识符（若为storage）(不重要, 可能全为0)
	byte user_flag[4]; // [060H,04] 用户标记(不重要, 可能全为0)
	byte time_st[8]; // [064H,16] 创建此入口的时间标记。大多数情况都不写
	byte time_ed[8]; // [064H,16] 最后修改此入口的时间标记。大多数情况都不写
	byte sid_of_first_sector[4]; // [074H,04] 若此为流的入口，指定流的第一个sector或short-sector的SID,若此为根仓库入口，指定短流存放流的第一个sector的SID,其他情况，为0
	byte total_stream_size[4]; // [078H,04] 若此为流的入口，指定流的大小（字节）若此为根仓库入口，指定短流存放流的大小（字节）其他情况，为0
	byte reserved[4]; // [07CH,04] Reserved
};

/*****************************************************************************
//word文档FIB结构定义
******************************************************************************/
struct FIB
{
	//=========FibBase (32 bytes)===========//
	byte wIdent[2]; // [000H,2] 固定为0Xa5EC，表明文档为Word二进制文件
	byte nFib[2]; // [002H,2] Word格式的版本（nFib），但实际上这里一般为0Xc1，即Word97的格式
	byte unused[2]; // [004H,2] Not used
	byte lid[2]; // [006H,2] 指定生成文档的应用程序的安装语言
	byte pnNext[2]; // [008H,2] 指定包含所有自动图文集项目的文档在Word Document stream的FIB中的偏移量。如果值为0，表明没有自动图文集项目。否则FIB发现文件位置为pnNext×512
	byte Doctemplate[1]; // [00AH,1] 文档是否是.Dot文件（Word模板文件）;文档是否只包含自动图文集;文档是否是复杂格式（快速保存时生成的格式）;文档是否包含图片
	byte Encrypted[1]; // [00BH,1] 文档是否加密;为1时文字存储于1Table，为0时文字存储于0Table;是否“建议以只读方式打开文档”;是否有写保护密码
	byte nFibBack[2]; // [00CH,2] 固定的0x00BF或0x00C1
	byte lKey[4]; // [00EH,4] 当文档加密并且混淆，则为混淆的密钥；如果加密不混淆，则为加密头的长度；否则应置0
	byte envr[1]; // [012H,1]置0，并且忽略
	byte LoadOverridePage[1]; // [013H,1] 是否要用应用程序的默认值覆盖页面中的页面大小、页面方向、页边距等
	byte reserved[12]; // [014H,12]
	//=========csw (2 bytes)===========//
	byte csw[2]; //[020H,2]FibRgW97块中16位整数的个数，固定为0x000E
	//=========FibRgW97 (28 bytes)===========//
	byte reserved1[2]; //[022H,2]
	byte reserved2[2];//[024H, 2]
	byte reserved3[2];//[026H, 2]
	byte reserved4[2];// [028H, 2]
	byte reserved5[2]; //[02AH,2]
	byte reserved6[2];//[02CH, 2]
	byte reserved7[2];//[02EH, 2]
	byte reserved8[2];// [030H, 2]
	byte reserved9[2];// [032H, 2]
	byte reserved10[2]; //[034H,2]
	byte reserved11[2];//[036H, 2]
	byte reserved12[2];//[038H, 2]
	byte reserved13[2];// [03AH, 2]
	byte lidFE[2]; // [03CH,2] nFib与文件版本对应的情况
	//=========cslw (2 bytes)===========//
	byte cslw[2]; //[03EH,2]FibRgLw97块中32位整数的个数，固定为0x0016
	//=========FibRgLw97(88 bytes)===========//
	byte cbMac[4]; //[040H,4]Word Document中有意义的字节数（即Word Document之后的字节数都可以忽略）
	byte reserved14[4];//[044H, 4]
	byte reserved15[4];//[048H, 4]
	byte ccpText[4];// [04CH, 4]文档中正文（Main document）的总字数
	byte ccpFtn[4]; //[050H,4]文档中页脚（Footnote subdocument）的总字数
	byte ccpHdd[4];//[054H, 4]文档中页眉（Header subdocument）的总字数
	byte reserved16[4];//[058H, 4]
	byte ccpAtn[4];// [05CH, 4]文档中批注（Comment subdocument）的总字数
	byte ccpEdn[4];// [060H, 4]文档中尾注（Endnote subdocument）的总字数
	byte ccpTxbx[4]; //[064H,4]文档中文本框（Textbox subdocument）的总字数
	byte ccpHdrTxbx[4];//[068H, 4]文档中页眉文本框（Textbox Subdocument of the header）的总字数
	byte reserved17[4];//[06CH, 4]
	byte reserved18[4];// [070H, 4]
	byte reserved19[4];//[074H, 4]
	byte reserved20[4];// [078H, 4]
	byte reserved21[4];//[07CH, 4]
	byte reserved22[4];// [080H, 4]
	byte reserved23[4];//[084H, 4]
	byte reserved24[4];// [088H, 4]
	byte reserved25[4];//[08CH, 4]
	byte reserved26[4];// [090H, 4]
	byte reserved27[4];//[094H, 4]
	//=========cbRgFcLcb (2 bytes)===========//
	byte cbRgFcLcb[2]; //[09AH,2]FibRgFcLcb块中64位整数的个数（但FibRgFcLcb实际存储的是32位整数）。如果文档为Word97，该项为0x005D。
};

int docmasking(byte *file, vector<string> keyword);
int mask(byte *txt, int len, char *keyword, int mode);
int UTF8StrToUnicodeStr(unsigned char * utf8_str,
	unsigned short * unicode_str, int unicode_str_size);
int readPlcPcd(byte *file, byte **apcd, byte **acp, int &wordEntryOffset, int *difat1);
int findClxOffset(byte *file, int &wordEntryOffset, int *difat1);
int findstream(byte *file, char *name, int len, int &msector_len, int *difat1);
int readFIB(byte *file, int &fcClx, int &lcbClx, int *difat1, int &msector_len, int &ssector_len, int &WordDoc_offset);
int bin_to_hex(byte* addr, int len);
int readheader(byte *file);
char *KMP(char *A, char *B, int Alen, int Blen);
void InitP(char *B, int size);            //将B串(模式串)进行自我匹配
int getTrueOffset(byte* file, int curOffset, int requireOffset, int* difat1);

int summaryInfoReplace(char* name, byte* file, vector<string> word, int* difat1);
int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen,
	char *outbuf, size_t outlen);
int u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
int u8tou16(char *inbuf, size_t inlen, char *outbuf, size_t outlen);
