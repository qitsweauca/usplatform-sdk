// ================================================================================================
// HALTest.cpp - a simple console app for ultrasound data acquisition with the USPlatform.
// ------------------------------------------------------------------------------------------------
// Copyright (c) 2019 us4us Ltd. / MIT License
// ================================================================================================
#include "stdafx.h"
#include "inc/HAL.h"
#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <stdio.h>
#include <cuda_runtime_api.h>

#pragma pack(1)
typedef struct
{
	UINT16 data : 14;
	UINT16 id : 2;
} HALDEBUGDATA;
#pragma pack()

#define TXRX_JSON "LA-CPWI.json"	// define TX/RX JSON configuration file

#define NCH 192				// Total # of channels
#define NSAMPLES (4*1024)	// Samples in one line
#define NEVENTS 32			// Total Events
#define NFRAMES 125			// Total Frames

// Total Frame size [Bytes]
#define FRAMESIZE (NCH * NSAMPLES * sizeof(SHORT) * NEVENTS)

BYTE * data;
int frame;
Metadata metadata[NFRAMES];
int idx;
bool dataWritten = false;

class Callback : public IHALCallback
{
public:
	void OnNewData(int idx)
	{
		//::idx++;
		//::idx %= 2;
		//std::cout << std::dec << "NewData " << idx;

		//pHAL->Sync(pHAL->GetMetadata(::idx)->frameIdx);
		//short * data = (short*)pHAL->GetData(idx);
		//int frameIdx = pHAL->GetMetadata(idx)->frameIdx;
		//int skippedChunks = pHAL->GetMetadata(idx)->skippedChunks;

		//std::cout << " FrameIdx=" << frameIdx << " SkippedChunks=" << skippedChunks << std::endl;

		//pHAL->Sync(idx);

		if (frame < NFRAMES)
		{
			//std::cout << "HALCallback" << std::endl;
			BYTE * ptr = (BYTE*)pHAL->GetData(idx);
			memcpy(data + frame*FRAMESIZE, ptr, FRAMESIZE);
			memcpy((void*)&metadata[frame], (void*)pHAL->GetMetadata(idx), sizeof(Metadata));
			++frame;
			pHAL->Sync(idx);
		}
		else if (!dataWritten)
		{
			pHAL->Stop();

			for (int i = 0; i < NFRAMES; i++)
			{
				FILE * f = NULL;
				std::stringstream filename;
				filename << "rf_frame\\rf_frame_" << i << ".bin";
				fopen_s(&f, filename.str().c_str(), "wb");
				if (f)
				{
					std::cout << "Writing " << std::dec << FRAMESIZE << " bytes to file " << filename.str() << std::endl;
					fwrite(data + i*FRAMESIZE, sizeof(SHORT), FRAMESIZE / sizeof(SHORT), f);
					fclose(f);
				}
				else
				{
					std::cout << "File open error.\n";
				}
			}
			std::cout << "Finished! " << std::endl;

			FILE * f = NULL;
			std::stringstream filename1;
			filename1 << "metadata.bin";
			fopen_s(&f, filename1.str().c_str(), "wb");
			if (f)
			{
				fwrite(metadata, sizeof(Metadata), NFRAMES, f);
				fclose(f);
				std::cout << "Metadata written to file " << filename1.str() << std::endl;
			}
			else
			{
				std::cout << "File open error.\n";
			}

			dataWritten = true;
		}
	}
	IHALHANDLE pHAL;
};

// Read JSON config file
std::string getJson(const std::string& jsonFileName)
{
	std::ifstream jsonFile(jsonFileName);
	std::stringstream jsonFileBuffer;
	jsonFileBuffer << jsonFile.rdbuf();
	jsonFile.close();
	return jsonFileBuffer.str();
}

// Show list of available/detected HAL devices
void printDetectedDevices()
{
	char** listOfNames;
	int namesNumber = GetAvailableHALDevicesNames(&listOfNames);

	std::cout << "Detected devices:" << std::endl;
	for (int i = 0; i < namesNumber; ++i)
		std::cout << "--- " << std::string(listOfNames[i]) << std::endl;

	for (int i = 0; i < namesNumber; ++i)
		delete[] listOfNames[i];
	delete[] listOfNames;
}

// *** main() function ***
int _tmain(int argc, _TCHAR* argv[])
{
	// allocate a memory buffer for raw RF data
	data = (BYTE*)malloc(NCH * NSAMPLES * sizeof(SHORT) * NEVENTS * NFRAMES);

	// print all detected HAL devices
	printDetectedDevices();

	// open real USPlatform hardware
	IHALHANDLE pHAL = GetHALInstance("USPLATFORMHAL");

	// configure USPlatform with the JSON file
	pHAL->Configure(getJson( TXRX_JSON ).c_str());

	Callback cb;
	cb.pHAL = pHAL;

	// register an async acquisition callback
	pHAL->RegisterCallback(&cb);

	// start TX/RX
	pHAL->Start();

	int key = 0;
	while (1)
	{
		if (_kbhit())
		{
			key = _getch();
			if (key == 'q') break;
		}
		Sleep(100);
	}

	// stop TX/RX
	pHAL->Stop();
	pHAL->Release();

	system("PAUSE");
	cudaDeviceSynchronize();
	cudaDeviceReset();

	return 0;
}
