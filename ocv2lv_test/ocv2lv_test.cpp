// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ocv2lv_dll.h"
#include <iostream>
#include <windows.h>

#pragma comment(lib, "ocv2lv.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	cv::Size* testSize     = new cv::Size(640, 480);
	cv::Mat* testArray     = new cv::Mat(*testSize, CV_8UC1, cv::Scalar::all(0));
	__int32 _cameraNumber  = 0;
	__int32 _queryTimeout  = 2000;
	__int32 _answerTimeout = 1000;
	bool _exitThread = false;
	cv::namedWindow("test");

	for (int i = 0; i < 10; i++) {
		__int32 result = ocv2lvManager(testSize, testArray->data, _cameraNumber, _answerTimeout, _queryTimeout, &_exitThread);
		if (!result) {
			cv::imshow("test", *testArray);
			cv::waitKey(1000);
		}
		std::cout << result << std::endl;
	}
	cv::destroyAllWindows();
	delete(testArray);
	delete(testSize);
	if (!_exitThread)
		Sleep(_queryTimeout);
	system("pause");
	return 0;
}

