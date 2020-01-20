#include "stdafx.h"
#include "filesystem.h"


//�������ļ���
void listfiles(std::string dir, std::string namefolder, _folder &dbfolder)
{
	intptr_t handle;
	_finddata_t findData;
	string dirfilter = dir + "/" + namefolder + "/" + "*.*";
	handle = _findfirst(dirfilter.c_str(), &findData);    // ����Ŀ¼�еĵ�һ���ļ�
	if (handle == -1)
	{
		cout << "Failed to find first file!\n";
		return;
	}
	dbfolder.Folder_Name = namefolder;
	do
	{
		if (!(findData.attrib & _A_SUBDIR))
		{
			String filename = dir + "/" + namefolder + "/" + findData.name;
			dbfolder.files.push_back(filename);
		}
	} while (_findnext(handle, &findData) == 0);    // ����Ŀ¼�е���һ���ļ�
	_findclose(handle);    // �ر��������
}

//��dirĿ¼����Ѱ�����е�Ŀ¼��ÿһ��Ŀ¼subdir����һ���ˣ������˵����˵Ķ�����Ƭ
void listfolder(std::string dir, _folder_list &dbfolders)
{
	intptr_t handle;
	_finddata_t findData;
	string dirfilter = dir + "/" + "*.*";
	handle = _findfirst(dirfilter.c_str(), &findData);    // ����Ŀ¼�еĵ�һ���ļ�
	if (handle == -1)
	{
		cout << "Failed to find first file!\n";
		return;
	}
	do
	{
		if (findData.attrib & _A_SUBDIR)
			//&& strcmp(findData.name, ".") != 0
			//&& strcmp(findData.name, "..") != 0
			//)    // �Ƿ�����Ŀ¼���Ҳ�Ϊ"."��".."
		{
			if (findData.name[0] != '.')
			{
				//��һ����׼���ļ���
				_folder folder;
				listfiles(dir, findData.name, folder);
				dbfolders.folders.push_back(folder);
			}
		}
		else //�������ļ�
		{
			//���ļ�
			//face_desc_vec.files.push_back(findData.name);
		}
	} while (_findnext(handle, &findData) == 0);    // ����Ŀ¼�е���һ���ļ�
	_findclose(handle);    // �ر��������
};