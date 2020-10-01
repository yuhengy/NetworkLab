#ifndef __COMMON_H__
#define __COMMON_H__

#define REQMSG_LEN 1000
#define RESPMSG_LEN 1000

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include <pthread.h>
#include <sys/wait.h>
#include <ctime>
using namespace std;

vector<string> split(string s, string delimiter);

#endif
