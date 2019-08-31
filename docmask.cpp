#include "doc.h"

int docmasking(byte *file, vector<string> word){
    int compoundCheck = readheader(file);
    if(compoundCheck == -2)
        return -1;

	//get sector_len
    Compound_Binary_File_Header *header = (Compound_Binary_File_Header *)file;
    int msector_len = pow(2, bin_to_hex(header->size_of_a_master_sector, 2));
    int ssector_len = pow(2, bin_to_hex(header->size_of_a_short_sector, 2));

    //build complete DiFat
    int *difat = (int *)((Compound_Binary_File_Header *)file)->first_109_master_sector_MSAT;
    int difatSect1 = bin_to_hex(header->first_sector_of_master_sector_MSAT, 4);
    int numDifatSects = bin_to_hex(header->total_number_of_master_sector_MSAT, 4);
    //int difat1[109 + numDifatSects * msector_len];
	int difat1[10000];

    for(int i = 0; i < 109; i++)
    {
        difat1[i] = difat[i];
    }
    int* nextDifat = (int*)(file + (512 + difatSect1 * msector_len));  //first difatSect offset
    for(int i = 0; i < numDifatSects; i++)
    {
        for(int j = 0; j < msector_len / 4 - 1; j++)
        {
            difat1[109 + j + (msector_len / 4 - 1)* i] = nextDifat[j];
        }

        difatSect1 = nextDifat[msector_len / 4 - 1];
        nextDifat = (int*)(file + (512 + difatSect1 * msector_len));
    }
	
	//summaryInfoReplace("\005SummaryInformation", (byte*)file, word, difat1);
	//summaryInfoReplace("\005DocumentSummaryInformation", (byte*)file, word, difat1);
	
	byte *apcd = NULL;
	byte *acp = NULL;
	int wordEntryOffset = 0; //word entry start
	int number_pcd = readPlcPcd((byte *)file, &apcd, &acp, wordEntryOffset, difat1);
	if (number_pcd < 0)
		return -1;

	int success = 0;

	for (int i = 0; i < number_pcd; i++){
        int fc = bin_to_hex(apcd + 8 * i + 2, 4) & 0x40000000;
        int cp_len = bin_to_hex(acp + 4 * (i + 1), 4) - bin_to_hex(acp + 4 * i, 4);
        vector<string>::iterator it;
		if (fc == 0){
			// Unicode
			int txt_offset = bin_to_hex((apcd + 8 * i + 2), 4) & 0x3FFFFFFF;
			int trueOffset = getTrueOffset(file, wordEntryOffset, txt_offset, difat1);
			
			byte *txt = file + trueOffset;
			for (it = word.begin(); it != word.end(); it++){
				char *keyword = (char *)(*it).c_str();
				success = mask(txt, cp_len, keyword, 2);
			}
		}
		else{
			// ANSI
			int txt_offset = (bin_to_hex(apcd + 8 * i + 2, 4) & 0x3FFFFFFF) / 2;
			int trueOffset = getTrueOffset(file, wordEntryOffset, txt_offset, difat1);
			byte * txt = file + trueOffset;
			for (it = word.begin(); it != word.end(); it++){
				char *keyword = (char*)(*it).c_str();
				success = mask(txt, cp_len, keyword, 1);
			}
		}
	}

	free(apcd);
	free(acp);
	return success;
}

// mode = 1: denote ANSI mode = 2: denote Unicode
// return 1: 有过滤关键词
// return 0: 没有过滤
int mask(byte *txt, int len, char *keyword, int mode){
	int flag = 0;
	int keyword_len = strlen(keyword);

	char *tmp = NULL;
	int size = 0;
	unsigned short *outputStr;
	if (mode == 2){
		outputStr = (unsigned short *)malloc(2 * keyword_len);
		memset(outputStr, 0, 2 * keyword_len);
		int str_len = UTF8StrToUnicodeStr((byte *)keyword, outputStr, 0);
		UTF8StrToUnicodeStr((byte *)keyword, outputStr, (str_len) / 2);
		keyword_len = str_len;
		keyword = (char *)outputStr;	
		//free(outputStr);
	}
	
	while (size < len*mode){
		tmp = KMP((char*)txt + size, keyword, len*mode - size, keyword_len);
		if (tmp != NULL){
			flag = 1;
			int posi = (int)((byte *)tmp - txt);
			for (int i = 0; i < keyword_len / mode; i++){
				*(txt + posi + i*mode) = 0x2a;
				if (mode == 2)
					*(txt + posi + i*mode + 1) = 0x00;
			}
			size = posi + keyword_len;
		}
		else
			break;
	}
	/*if (mode == 2)
		free(outputStr);*/
	return flag;
}

int getTrueOffset(byte* file, int curOffset, int requireOffset, int* difat1)
{
    //get sector_len
    Compound_Binary_File_Header *header = (Compound_Binary_File_Header *)file;
    int msector_len = pow(2, bin_to_hex(header->size_of_a_master_sector, 2));
    int ssector_len = pow(2, bin_to_hex(header->size_of_a_short_sector, 2));

    int stream_sid = bin_to_hex((byte*)((struct Storage_Directory_Entry *)(file + curOffset))->sid_of_first_sector, 4);
    int stream_size = bin_to_hex((byte*)((struct Storage_Directory_Entry *)(file + curOffset))->total_stream_size, 4);

    if(stream_size >= 4096)
            {
                //find the actual content
                for (int i = 0; i < requireOffset / msector_len; i++)
                {
                    stream_sid = bin_to_hex(file + sizeof(struct Compound_Binary_File_Header) + difat1[stream_sid / (msector_len / 4)] * msector_len + stream_sid % (msector_len / 4) * 4, 4);
                }
                return stream_sid*msector_len + sizeof(Compound_Binary_File_Header) + requireOffset%msector_len;
            }
            else
            {
                //use short sector to locate table stream start offset
                int s_stream_sid = stream_sid;
                //here table_stream_sid indicates where it belongs to the true msector-sid
                stream_sid = bin_to_hex(header->first_sector_of_short_sector_SSAT, 4) + bin_to_hex(header->total_number_of_short_sector_SSAT, 4);
                for (int i = 0; i < requireOffset / msector_len; i++)
                {
                    stream_sid = bin_to_hex(file + sizeof(struct Compound_Binary_File_Header) + difat1[stream_sid / (msector_len / 4)] * msector_len + stream_sid % (msector_len / 4) * 4, 4);
                }
                return stream_sid*msector_len + sizeof(Compound_Binary_File_Header) + requireOffset%msector_len + ssector_len * s_stream_sid;
            }

}

