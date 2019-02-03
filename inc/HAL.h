// ================================================================================================
// HAL.h - USPlatform Hardware Abstraction Layer - header file.
// ------------------------------------------------------------------------------------------------
// Copyright (c) 2019 us4us Ltd. / MIT License
// ================================================================================================
#pragma once

#include <functional>
#include <Windows.h>

#ifdef _WIN32

#ifdef __cplusplus

#ifdef HAL_EXPORTS
#define HAL_API __declspec(dllexport)
#else
#define HAL_API __declspec(dllimport)
#endif

#pragma pack(1)
typedef struct {
	UINT32 totalTriggers;
	UINT32 totalChunks;
	UINT32 skippedChunks;
	UINT32 frameIdx;
} Metadata;
#pragma pack()

class IHALCallback
{
public:
	virtual void OnNewData(int idx) = 0;
};

class IHAL
{
public:
	typedef enum {
		HAL_OK = 0,
		HAL_NOT_CONFIGURED = -1,
		HAL_JSON_PARSER_ERROR = -2
	} HAL_ERRORS;
	virtual void Release(void) = 0;
	virtual int Start(void) = 0;
	virtual int SoftTrigger(void) = 0;
	virtual int Stop(void) = 0;
	virtual int GetLastError(void) = 0;
	virtual int RegisterCallback(IHALCallback * cb) = 0;
	virtual int UnregisterCallback(void) = 0;
	virtual void * GetBuffer(int idx) = 0;
	virtual void * GetData(int idx) = 0;
	virtual Metadata * GetMetadata(int idx) = 0;
	virtual int Sync(int idx) = 0;
	virtual int Configure(const char * jsonConfigString) = 0;
};

typedef IHAL* IHALHANDLE;

#endif

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C HAL_API IHALHANDLE WINAPI GetHALInstance(const char * name);
EXTERN_C HAL_API int WINAPI GetAvailableHALDevicesNames(char*** listOfNames);

// C Interface
EXTERN_C
{
	HAL_API void WINAPI Release(IHALHANDLE hal);
	HAL_API int WINAPI Start(IHALHANDLE hal);
	HAL_API int WINAPI SoftTrigger(IHALHANDLE hal);
	HAL_API int WINAPI Stop(IHALHANDLE hal);
	HAL_API int WINAPI GetHalLastError(IHALHANDLE hal);
	HAL_API int WINAPI RegisterCallback(IHALHANDLE hal, void(*callback)(int idx));
	HAL_API int WINAPI UnregisterCallback(IHALHANDLE hal);
	HAL_API void* WINAPI GetBuffer(IHALHANDLE hal, int idx);
	HAL_API void* WINAPI GetData(IHALHANDLE hal, int idx);
	HAL_API Metadata* WINAPI GetMetadata(IHALHANDLE hal, int idx);
	HAL_API int WINAPI Sync(IHALHANDLE hal, int idx);
	HAL_API int WINAPI Configure(IHALHANDLE hal, const char* jsonConfigString);
}

class CHALCallback : public IHALCallback
{
	private:
		std::function<void(int idx)> cCallback;

	public:
		CHALCallback(std::function<void(int idx)> callback);
		void OnNewData(int idx);
};

#else
#error "OS not supported"
#endif
