#include "doc.h"
#include <time.h>
#include <iostream>
#include <string>

const string map[3][2] = {
	"id", "123456",
/*	"user2", "654321",
	"user3", "password"
*/
};

int main(int argc, char* argv[]){


	string name, pw;
	int i;
	while (cin >> name >> pw)
	{
		for (i = 0; i < 3; i++)
		{
			if (map[i][0] == name)			
				{
					clock_t start = clock();
					//if(argc < 3)
					//{
					//	cout<<"at least 3 argument needed"<<endl;
					//	cout<<"./xxx path keyword"<<endl;
					//	//return -1;
					//}
					char *path = "demo.doc";//argv[1]; //"./0.doc";
					FILE *fp = fopen(path, "rb");
					if (fp == NULL)
					{
						printf("open file fail\n");
						return -1;
					}
					fseek(fp, 0, SEEK_END);
					int length = ftell(fp);
					char *file = (char *)malloc(length);
					fseek(fp, 0, SEEK_SET);
					fread(file, sizeof(char), length, fp);

					vector<string> keyword;
					/*for(int i = 2; i < argc; i++)
					keyword.push_back(argv[i]);*/
					keyword.push_back("课程");

					/**			doc脱敏	                   */
					if (docmasking((byte*)file, keyword) == 1)
						cout << "success" << endl;
					/***************************************/

					FILE *out = fopen("out.doc", "wb");
					fwrite(file, 1, length, out);
					fclose(out);
					fclose(fp);

					clock_t finish = clock();
					double Milltime = (double)(finish - start) / CLOCKS_PER_SEC;
					//cout<<"\n此程序的运行时间为"<<Milltime<<"s！"<<endl;
			}break;
		}
		if (i == 3) cout << "no such user name\n";
		else if (map[i][1] != pw)
			cout << "password error\n";
		else break;
	}
	cout << "Login ok, welcome " << name << endl;
	cout << "no function, logout and exit\n";
	return 0;
}
