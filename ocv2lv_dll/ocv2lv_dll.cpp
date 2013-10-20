// ocv2lv.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ocv2lv_dll.h"

cv::Size variablesBuffer::LVarraySize;
DWORD variablesBuffer::LVarrayPointer;
__int32 variablesBuffer::cameraNumber;
__int32 variablesBuffer::queryTimeout;
__int32 variablesBuffer::returnCode;
bool variablesBuffer::workerExist = false;
bool variablesBuffer::saveData;
bool variablesBuffer::exitThread;
HANDLE variablesBuffer::returnDataEvent;
HANDLE variablesBuffer::workerThreadHandle;

__int32 ocv2lvManager(cv::Size* m_LVarraySize, void* m_LVarrayPointer, 
	__int32 m_cameraNumber, __int32 m_answerTimeout,__int32 m_queryTimeout, bool *m_exitThread) {
		
		__int32 m_returnCode;
		m_returnCode = ocv2lvChecker(m_LVarraySize, m_LVarrayPointer, m_cameraNumber, m_answerTimeout, m_queryTimeout);
		if (m_returnCode != ALL_OK) {
			if (variablesBuffer::workerExist)
				variablesBuffer::exitThread = true;
			return m_returnCode;
		}
		if (!variablesBuffer::workerExist) {
			m_returnCode = ocv2lvInitializer(m_LVarraySize, m_LVarrayPointer, m_cameraNumber, m_answerTimeout, m_queryTimeout);
			if (m_returnCode != ALL_OK)
				return m_returnCode;
		}
		else if (*m_LVarraySize != variablesBuffer::LVarraySize || (DWORD)m_LVarrayPointer != variablesBuffer::LVarrayPointer ||
			m_cameraNumber != variablesBuffer::cameraNumber || m_queryTimeout != variablesBuffer::queryTimeout)	{
				variablesBuffer::exitThread = true;
				m_returnCode = ocv2lvInitializer(m_LVarraySize, m_LVarrayPointer, m_cameraNumber, m_answerTimeout, m_queryTimeout);
				if (m_returnCode != ALL_OK)
					return m_returnCode;
		}
		variablesBuffer::saveData = true;
		if (*m_exitThread)
			variablesBuffer::exitThread = true;
		__int32 m_waitResult = WaitForSingleObject(variablesBuffer::returnDataEvent, m_answerTimeout);
		if (m_waitResult != WAIT_OBJECT_0) {
			variablesBuffer::saveData = false;
			if (variablesBuffer::returnCode != ALL_OK)
				return variablesBuffer::returnCode;
			else
				return WAIT_FOR_ANSWER_TIMEOUT;
		}
		return ALL_OK;
}

#ifdef THREADEX
unsigned __stdcall ocv2lvWorker(void* pArguments) {
#else
void ocv2lvWorker(void* pArguments) {
#endif
	variablesBuffer::workerExist = true;
	variablesBuffer::returnCode = ALL_OK;
	cv::Size w_LVarraySize = variablesBuffer::LVarraySize;
	void* w_LVarrayPointer = (void*)variablesBuffer::LVarrayPointer;
	__int32 w_cameraNumber = variablesBuffer::cameraNumber;
	__int32 w_queryTimeout = variablesBuffer::queryTimeout;
	DWORD w_lastQueryTime  = GetTickCount();
	bool w_checkResult;

	cv::VideoCapture w_mainCapture(w_cameraNumber);
	w_checkResult = w_mainCapture.isOpened();
	if (!w_checkResult) {
		variablesBuffer::returnCode = CAP_OPEN_ERROR;
		ocv2lvCleaner();
#ifdef THREADEX
		_endthreadex(CAP_OPEN_ERROR);
#else
		_endthread();
#endif
	}
	w_checkResult = w_mainCapture.set(CV_CAP_PROP_FRAME_WIDTH, w_LVarraySize.width) &&  w_mainCapture.set(CV_CAP_PROP_FRAME_HEIGHT, w_LVarraySize.height);
	if (!w_checkResult) {
		w_mainCapture.release();
		variablesBuffer::returnCode = CAP_SETPROPERTY_ERROR;
		ocv2lvCleaner();
#ifdef THREADEX
		_endthreadex(CAP_SETPROPERTY_ERROR);
#else
		_endthread();
#endif
	}
	__int32 w_captureWidth  = (__int32)w_mainCapture.get(CV_CAP_PROP_FRAME_WIDTH);
	__int32 w_captureHeight = (__int32)w_mainCapture.get(CV_CAP_PROP_FRAME_HEIGHT);
	if (w_captureWidth == 0 || w_captureHeight == 0) {
		w_mainCapture.release();
		variablesBuffer::returnCode = CAP_GETPROPERTY_ERROR;
		ocv2lvCleaner();
#ifdef THREADEX
		_endthreadex(CAP_GETPROPERTY_ERROR);
#else
		_endthread();
#endif
	}
	if (w_captureWidth != w_LVarraySize.width || w_captureHeight != w_LVarraySize.height) {
		w_mainCapture.release();
		variablesBuffer::returnCode = UNSUPPORTED_RESOLUTION;
		ocv2lvCleaner();
#ifdef THREADEX
		_endthreadex(UNSUPPORTED_RESOLUTION);
#else
		_endthread();
#endif
	}
	cv::Mat w_tempMatForCapture;
	for (int i = 0; i < 2; i++) {
		w_checkResult = w_mainCapture.read(w_tempMatForCapture);
		if (!w_checkResult) {
			w_mainCapture.release();
			variablesBuffer::returnCode = CAP_READ_ERROR;
			ocv2lvCleaner();
#ifdef THREADEX
			_endthreadex(CAP_READ_ERROR);
#else
			_endthread();
#endif
		}
	}
	do {
		w_checkResult = w_mainCapture.read(w_tempMatForCapture);
		if (!w_checkResult) {
			w_mainCapture.release();
			variablesBuffer::returnCode = CAP_READ_ERROR;
			ocv2lvCleaner();
#ifdef THREADEX
			_endthreadex(CAP_READ_ERROR);
#else
			_endthread();
#endif
		}
		if (variablesBuffer::saveData) {
			variablesBuffer::saveData = false;
			w_lastQueryTime = GetTickCount();
			cv::Mat *w_matOverLVarray = new cv::Mat(w_LVarraySize, CV_8UC1, w_LVarrayPointer);
			cv::cvtColor(w_tempMatForCapture, *w_matOverLVarray, CV_BGR2GRAY);
			SetEvent(variablesBuffer::returnDataEvent);
			delete (w_matOverLVarray);
		}
	} while (!variablesBuffer::exitThread && GetTickCount() - w_lastQueryTime < (DWORD)w_queryTimeout);
	w_mainCapture.release();
	ocv2lvCleaner();
#ifdef THREADEX
	_endthreadex(ALL_OK);
	return 0;
#else
	_endthread();
#endif
}

__int32 ocv2lvChecker(cv::Size* ch_LVarraySize, void* ch_LVarrayPointer, __int32 ch_cameraNumber, __int32 ch_answerTimeout,
	__int32 ch_queryTimeout) {
		if (ch_LVarraySize->width <= 0 || ch_LVarraySize->height <= 0)
			return BAD_ARRAY_SIZE;
		else if ((DWORD)ch_LVarrayPointer == 0)
			return BAD_ARRAY_POINTER;
		else if (ch_cameraNumber < 0)
			return BAD_CAMERA_NUMBER;
		else if (ch_answerTimeout <= 0 || ch_queryTimeout <= 0)
			return BAD_TIMEOUT;
		else
			return ALL_OK;
}

__int32 ocv2lvInitializer(cv::Size* i_LVarraySize, void* i_LVarrayPointer, __int32 i_cameraNumber, __int32 i_answerTimeout, 
	__int32 i_queryTimeout) {
		if (variablesBuffer::workerExist) {
			__int32 i_waitResult = WaitForSingleObject(variablesBuffer::workerThreadHandle, i_answerTimeout);
			if (i_waitResult == WAIT_TIMEOUT)
				return WAIT_FOR_ANSWER_TIMEOUT;
		}
		variablesBuffer::LVarraySize     = *i_LVarraySize;
		variablesBuffer::LVarrayPointer  = (DWORD)i_LVarrayPointer;
		variablesBuffer::cameraNumber    = i_cameraNumber;
		variablesBuffer::queryTimeout    = i_queryTimeout;		
		variablesBuffer::returnDataEvent = CreateEvent(NULL, false, false, L"returnDataEvent");
		if (variablesBuffer::returnDataEvent != NULL) {
#ifdef THREADEX
			variablesBuffer::workerThreadHandle = (HANDLE)_beginthreadex(NULL, 0, &ocv2lvWorker, NULL, 0, NULL);
#else
			variablesBuffer::workerThreadHandle = (HANDLE)_beginthread(ocv2lvWorker, 0, NULL);
#endif
			if (variablesBuffer::workerThreadHandle != NULL)
				return ALL_OK;
			else {
				ocv2lvCleaner();
				return CREATE_THREAD_ERROR;
			}
		}
		ocv2lvCleaner();
		return CREATE_EVENT_ERROR;
}

__int32 ocv2lvCleaner() {
	if (variablesBuffer::returnDataEvent != NULL)
		CloseHandle(variablesBuffer::returnDataEvent);
#ifdef THREADEX
	if (variablesBuffer::workerThreadHandle != NULL)
		CloseHandle(variablesBuffer::workerThreadHandle);
#endif
	variablesBuffer::workerExist = false;
	return ALL_OK;
}