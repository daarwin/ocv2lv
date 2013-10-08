// All files within this DLL are compiled with the OCV2LV_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OCV2LV_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
//
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <windows.h>
#include <iostream>
#include <process.h>

#ifdef OCV2LV_EXPORTS
#define OCV2LV_API __declspec(dllexport)
#else
#define OCV2LV_API __declspec(dllimport)
#endif
//#define THREADEX

#ifdef _DEBUG
#pragma comment(lib, "opencv_core243d.lib")
#pragma comment(lib, "opencv_highgui243d.lib")
#pragma comment(lib, "opencv_imgproc243d.lib")
#else
#pragma comment(lib, "opencv_core243.lib")
#pragma comment(lib, "opencv_highgui243.lib")
#pragma comment(lib, "opencv_imgproc243.lib")
#endif

#pragma region ReturnCodes
#define ALL_OK					0
#define BAD_ARRAY_SIZE			1
#define BAD_ARRAY_POINTER		2
#define BAD_CAMERA_NUMBER		3
#define BAD_TIMEOUT				4
#define UNSUPPORTED_RESOLUTION	10
#define CAP_OPEN_ERROR			11
#define CAP_GETPROPERTY_ERROR	12
#define CAP_SETPROPERTY_ERROR	13
#define CAP_READ_ERROR			14
#define WAIT_FOR_ANSWER_TIMEOUT	20
#define CREATE_THREAD_ERROR		30
#define CREATE_EVENT_ERROR		31
#pragma endregion Коды ошибок

extern "C" OCV2LV_API __int32 ocv2lvManager(cv::Size*, void*, __int32, __int32, __int32, bool*);
__int32 ocv2lvChecker(cv::Size*, void*, __int32, __int32, __int32);
__int32 ocv2lvInitializer(cv::Size*, void*, __int32, __int32, __int32);
__int32 ocv2lvCleaner();
#ifdef THREADEX
unsigned __stdcall ocv2lvWorker(void* pArguments);
#else
void ocv2lvWorker(void* pArguments);
#endif

class variablesBuffer {
public:
	static cv::Size LVarraySize;
	static DWORD LVarrayPointer;
	static __int32 cameraNumber;
	static __int32 queryTimeout;
	static __int32 returnCode;
	static bool workerExist;
	static bool saveData;
	static bool exitThread;
	static HANDLE returnDataEvent;
	static HANDLE workerThreadHandle;
};