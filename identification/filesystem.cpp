#include "stdafx.h"
#include "filesystem.h"


//人名的文件夹
void listfiles(std::string dir, std::string namefolder, _folder &dbfolder)
{
	intptr_t handle;
	_finddata_t findData;
	string dirfilter = dir + "/" + namefolder + "/" + "*.*";
	handle = _findfirst(dirfilter.c_str(), &findData);    // 查找目录中的第一个文件
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
	} while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件
	_findclose(handle);    // 关闭搜索句柄
}

//从dir目录中中寻找其中的目录，每一个目录subdir代表一个人，包含了单个人的多张照片
void listfolder(std::string dir, _folder_list &dbfolders)
{
	intptr_t handle;
	_finddata_t findData;
	string dirfilter = dir + "/" + "*.*";
	handle = _findfirst(dirfilter.c_str(), &findData);    // 查找目录中的第一个文件
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
			//)    // 是否是子目录并且不为"."或".."
		{
			if (findData.name[0] != '.')
			{
				//是一个标准的文件夹
				_folder folder;
				listfiles(dir, findData.name, folder);
				dbfolders.folders.push_back(folder);
			}
		}
		else //否则是文件
		{
			//子文件
			//face_desc_vec.files.push_back(findData.name);
		}
	} while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件
	_findclose(handle);    // 关闭搜索句柄
};