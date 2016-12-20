/*******************************************************************************
* File Name		: timer.cpp
* Author		: zjw
* Email			: emp3XzA3MjJAMTYzLmNvbQo= (base64 encode)
* Create Time	: 2015年07月21日 星期二 16时45分16秒
*******************************************************************************/
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
	struct timeval tv;

	while (1)
	{
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		select(0, NULL, NULL, NULL, &tv);
		cout << "Five second have passed." << endl;
	}

	return 0;
}
