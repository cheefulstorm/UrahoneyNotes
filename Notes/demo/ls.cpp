#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <functional>
#include <unordered_map>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: <path>" << endl;
        exit(EXIT_FAILURE);
    }
    int fd = open(argv[1], O_RDONLY);
    if (-1 == fd) {
        perror("open error");
        exit(EXIT_FAILURE);
    }
    struct stat st{};
    int ret = fstat(fd, &st);
    if (-1 == ret) {
        perror("fstat error");
        exit(EXIT_FAILURE);
    }
    if (!S_ISDIR(st.st_mode)) {
        cout << argv[1] << "(";
        stat(argv[1], &st);
        switch (st.st_mode & S_IFMT) {
            case S_IFBLK:  cout << 'b'; break;
            case S_IFCHR:  cout << 'c'; break;
            case S_IFIFO:  cout << 'f'; break;
            case S_IFLNK:  cout << 'l'; break;
            case S_IFREG:  cout << 'r'; break;
            case S_IFSOCK: cout << 's'; break;
            default:       cout << '?'; break;
        }
        cout << ")" << endl;
        exit(EXIT_SUCCESS);
    }
    DIR *dp = fdopendir(fd);
    if (nullptr == dp) {
        perror("fdopendir error");
        exit(EXIT_FAILURE);
    }
    unordered_map<unsigned char, char> typeMap{{0, '?'},
                                               {1, 'f'},
                                               {2, 'c'},
                                               {4, 'd'},
                                               {6, 'b'},
                                               {8, 'r'},
                                               {10, 'l'},
                                               {12, 's'},
                                               {14, '?'}};
    string dirPath(argv[1]);
    function<void(DIR*, int)> lsDir = [&](DIR *dp, int depth) {
        dirent *dentP = nullptr;
        while (nullptr != (dentP = readdir(dp))) {
            if (dentP->d_name[0] == '.') {
                continue;
            }
            cout << string(depth, '\t');
            cout << dentP->d_name << "(" << typeMap[dentP->d_type] << ")" << endl;
            if (dentP->d_type == DT_DIR) {
                dirPath.push_back('/');
                dirPath += dentP->d_name;
                DIR *nextDp = opendir(dirPath.c_str());
                if (nullptr == nextDp) {
                    perror("opendir error");
                    exit(EXIT_FAILURE);
                }
                lsDir(nextDp, depth + 1);
                dirPath.resize(dirPath.size() - 1 - strlen(dentP->d_name));
            }
        }
        if (0 != errno) {
            perror("readdir error");
            exit(EXIT_FAILURE);
        }
        closedir(dp);
    };
    lsDir(dp, 0);
    exit(EXIT_SUCCESS);
}