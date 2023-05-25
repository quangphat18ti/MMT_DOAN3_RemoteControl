#include "FolderTask.h"

string FolderTask::convertWstringToConstChar(const std::wstring &wstr)
{
    std::string str(wstr.begin(), wstr.end());
    return str;
}

bool FolderTask::isFolder(string fn)
{
    struct stat path;

    stat(fn.c_str(), &path);

    return S_ISDIR(path.st_mode);
}

bool FolderTask::isExistFile(const char *path)
{
    ifstream file(path, ios::binary);
    if (!file)
        return false;
    else
    {
        file.close();
        return true;
    }
}

bool FolderTask::deleteFile(const char *path)
{
    bool result = remove(path);
    if (result != 0)
    {
        perror("Error deleting file");
        return false;
    }
    else
    {
        // printf("File deleted successfully\n");
        return true;
    }
    return true;
}

bool FolderTask::deleteFolder(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == nullptr)
    {
        cout << "Direction: " << path << "\n";
        cout << "Cannot open Folder\n";
        return false;
    }

    bool status = true;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char subpath[1000];
        sprintf(subpath, "%s/%s", path, entry->d_name);

        if (isFolder(subpath))
        {
            // Đệ quy xóa thư mục con
            status &= deleteFolder(subpath);
        }
        else
        {
            // Xóa tệp tin
            status &= deleteFile(subpath);
        }
    }
    closedir(dir);

    // Xóa thư mục gốc (rỗng)
    if (rmdir(path) != 0)
    {
        perror("Error deleting folder");
        return false;
    }

    return status;
}

// Hàm duyệt thư mục và lấy danh sách các file và thư mục
bool FolderTask::list_files(const char *path, vector<string> &files)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        cout << "path err: " << path << endl;
        cerr << "Error opening directory\n";
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            // Bỏ qua thư mục "." và ".."
            continue;
        }
        files.push_back(entry->d_name);
    }
    closedir(dir);
    return 1;
}

bool FolderTask::recursive_list_files(const char *path, vector<string> &files)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        cerr << "Error opening directory\n";
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            // Bỏ qua thư mục "." và ".."
            continue;
        }

        char subpath[1000];
        sprintf(subpath, "%s/%s", path, entry->d_name);

        // files.push_back(entry->d_name);
        files.push_back(subpath);

        if (isFolder(subpath))
        {
            vector<string> tmp;
            bool isOk = recursive_list_files(subpath, tmp);
            if (!isOk)
                return false;
            files.insert(files.end(), tmp.begin(), tmp.end());
        }
    }
    closedir(dir);
    return 1;
}

bool FolderTask::createFile(const char *path)
{
    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        perror("Error creating file");
        return false;
    }
    else
    {
        // printf("File created successfully");
        fclose(file);
    }
    return true;
}

bool FolderTask::createFolder(const char *path)
{
    cout << "Path: " << path << "\n";
    int result = mkdir(path);
    if (result == 0)
    {
        // printf("Folder created successfully\n");
        return true;
    }
    else
    {
        perror("Error creating folder");
        return false;
    }
}

bool FolderTask::renameFileFolder(const char *oldName, const char *newName)
{
    bool result = rename(oldName, newName);
    if (result == 0)
    {
        // printf("Rename successfully\n");
        return true;
    }
    else
    {
        perror("Error renaming");
        return false;
    }
}

bool FolderTask::copyFile(const char *sourcePath, const char *destPath)
{
    ifstream source(sourcePath, ios::binary);
    if (!source)
    {
        cout << "Cannot open source file\n";
        return false;
    }

    if (isExistFile(destPath))
    {
        cout << "Destination: " << destPath << "\n";
        cout << "Destination exist! Do you want to overwirte it? (Y = Yes)\n";
        char ch;
        cin >> ch;
        if (ch != 'Y')
            return false;
    }

    ofstream destination(destPath, ios::binary);
    if (!destination)
    {
        cout << "Cannot create copy file\n";
        return false;
    }

    // đọc nội dung tệp gốc
    destination << source.rdbuf();

    // đóng tệp
    source.close();
    destination.close();

    // cout << "File copied successfully\n";
    return true;
}

bool FolderTask::copyFolder(const char *sourcePath, const char *destPath)
{
    createFolder(destPath);
    DIR *dir = opendir(sourcePath);
    if (dir == NULL)
    {
        cerr << "Error opening source directory\n";
        return false;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            // Bỏ qua thư mục "." và ".."
            continue;
        }

        char subpath[1000];
        sprintf(subpath, "%s/%s", sourcePath, entry->d_name);

        char copypath[1000];
        sprintf(copypath, "%s/%s", destPath, entry->d_name);

        if (isFolder(subpath))
        {
            copyFolder(subpath, copypath);
        }
        else
            copyFile(subpath, copypath);
    }
    closedir(dir);
    return true;
}

string createFormatLine(string name, bool isFolder, string subpath, int cntTab)
{
    string ans = "";
    for (int i = 0; i < cntTab; i++)
        ans += '\t';

    ans = ans + name + "?" + (isFolder ? "Folder" : "File") + "?" + subpath + "\n";
    return ans;
}

string FolderTask::tab_recursive_list_files(const char *path, int cntTab)
{
    // cout << path << "\n";
    string files = "";
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        cerr << "Error opening directory\n";
        return files;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            // Bỏ qua thư mục "." và ".."
            continue;
        }

        char subpath[1000];
        sprintf(subpath, "%s/%s", path, entry->d_name);

        // files.push_back(entry->d_name);
        string addLine = createFormatLine(entry->d_name, isFolder(subpath), subpath, cntTab);
        files += addLine;

        if (isFolder(subpath))
        {
            auto tmp = tab_recursive_list_files(subpath, cntTab + 1);
            files += tmp;
        }
    }
    closedir(dir);
    return files;
}

// void FolderTask::getFileSystem(const char* path) {
//     // create a recursive_directory_iterator object for a given path
//     fs::recursive_directory_iterator iter(path);

//    // loop through all the entries in the directory
//    for (const auto& entry : iter)
//    {
//        // get the path of the entry
//        // fs::path path = entry.path();

//        // print the path
//        cout << entry.path() << "\t";
//        // ktra co phai folder hay khong
//        cout << entry.is_directory() << "\n";
//        // // kich thuoc
//        // cout << entry.file_size() << "\n";
//    }
//    Filesy
//}