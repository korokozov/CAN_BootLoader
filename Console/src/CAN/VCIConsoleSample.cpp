//////////////////////////////////////////////////////////////////////////
// HMS Technology Center Ravensburg GmbH
//////////////////////////////////////////////////////////////////////////
/**

  Demo application for the IXXAT interface based VCI-API.

  @note
	This demo demonstrates the following VCI features
	- controller selection
	- controller initialization
	- creation of a message channel
	- transmission / reception of CAN messages

*/
//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 
// HMS Technology Center Ravensburg GmbH, all rights reserved
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////////////////////////////
#include "vcisdk.h"

#include <process.h>
#include <stdio.h>
#include <conio.h>
#include "SocketSelectDlg.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////

static IBalObject* pBalObject = 0;     // bus access object

static LONG           lSocketNo = 0;     // socket number
static LONG           lBusCtlNo = 0;     // controller number

static ICanControl* pCanControl = 0;    // control interface
static ICanChannel* pCanChn = 0;        // channel interface

static LONG           lMustQuit = 0;      // quit flag for the receive thread

static HANDLE         hEventReader = 0;
static PFIFOREADER    pReader = 0;

static PFIFOWRITER    pWriter = 0;

static UINT8 Message[8];
static UINT32 MsgLength;
static UINT32 MsgId;
static UINT32 state;

#define STATE_INIT_BOOT_LOADER          0x0001
#define STATE_BOOT_LOADER_STARTED       0x0002
#define STATE_INIT_ERASE                0x0004
#define STATE_INIT_ERASE_OK             0x0008
#define STATE_ERASE_COMPLETE            0x0010
#define STATE_WRITE_START               0x0020
#define STATE_WRITE_START_COMLETE       0x0040
#define STATE_WRITE_DATA_BLOCK          0x0080
#define STATE_WRITE_DATA_BLOCK_COMPLETE 0x0100

#define MAX_COMAND_TIME                 100
#define MAX_ERASE_TIME                  (30000/1000)


#define MAX_REC_DATA_LENGTH     16

typedef struct {
	UINT8 RecLen;
	UINT16 MemOffset;
	UINT8 RecType;
	UINT8 Data_Or_Info[MAX_REC_DATA_LENGTH];
	UINT8 crc8;
}HexRecord;

int GetRecordFromString(HexRecord& hexrec, std::string hexstr) {
	UINT16 Len = hexstr.length();
	if (Len < 11)
		return 0;
	if (hexstr.substr(0, 1) != ":")
		return 0;
	UINT8 reclen = std::stoi(hexstr.substr(1, 2), nullptr, 16);
	if (Len < reclen * 2 + 11)
		return 0;
	hexrec.RecLen = reclen;
	hexrec.MemOffset = std::stoi(hexstr.substr(3, 4), nullptr, 16);
	hexrec.RecType = std::stoi(hexstr.substr(7, 2), nullptr, 16);
	for (UINT16 i = 0; i < hexrec.RecLen; i++)
		hexrec.Data_Or_Info[i] = std::stoi(hexstr.substr(9 + 2 * i, 2), nullptr, 16);
	hexrec.crc8 = std::stoi(hexstr.substr(9 + 2 * hexrec.RecLen, 2), nullptr, 16);
	return 1;
}

typedef struct {
	std::vector<HexRecord> records;
	UINT32 HexDataLen = 0;
	UINT32 StartAdres = 0;
	UINT32 LinStartAdres = 0;
	std::vector<UINT8> Data;
}HexData;

static HexData HData;

int GetHexRecordsFromFile(std::string Filename, HexData& hData) {
	UINT16 i, j, RecCount;
	std::vector<std::string> lines;
	std::string line;
	std::ifstream in(Filename);
	if (!in.is_open())
	{
		in.close();
		return 0;
	}
	else
	{
		while (std::getline(in, line))
		{
			std::cout << line << std::endl;
			lines.push_back(line);
		}
		in.close();
	}
	RecCount = lines.size();
	HexRecord tmp_record;
	hData.HexDataLen = 0;
	for (i = 0; i < RecCount; i++) {
		if (GetRecordFromString(tmp_record, lines.at(i))) {
			hData.records.push_back(tmp_record);
			if (tmp_record.RecType == 0x00)
			{
				hData.HexDataLen += tmp_record.RecLen;
				for (j = 0; j < tmp_record.RecLen; j++)
					hData.Data.push_back(tmp_record.Data_Or_Info[j]);
			}

			if (tmp_record.RecType == 0x04)
				hData.StartAdres = (tmp_record.Data_Or_Info[0] << 24) + (tmp_record.Data_Or_Info[1] << 16);
			if (tmp_record.RecType == 0x05)
				hData.LinStartAdres = (tmp_record.Data_Or_Info[0] << 24) + (tmp_record.Data_Or_Info[1] << 16) + (tmp_record.Data_Or_Info[2] << 8) + (tmp_record.Data_Or_Info[3]);
		}
		else {
			hData.records.clear();
			hData.HexDataLen = 0;
			hData.StartAdres = 0;
			hData.LinStartAdres = 0;
			hData.Data.clear();
			break;
		}
	}
	lines.clear();
	if (i == RecCount)
		return 1;
	return 0;
}



//////////////////////////////////////////////////////////////////////////
// function prototypes
//////////////////////////////////////////////////////////////////////////

HRESULT SelectDevice(BOOL fUserSelect);

HRESULT CheckBalFeatures(LONG lCtrlNo);
HRESULT InitSocket(LONG lCtrlNo);

void    FinalizeApp(void);

void    TransmitViaPutDataEntry(UINT32 MsgId, UINT payloadLen, UINT8* Msg);
void    TransmitViaWriter();

void    ReceiveThread(void* Param);

//////////////////////////////////////////////////////////////////////////
/**
  Main entry point of the application.
*/
//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	HRESULT hResult;
	state = 0;
	if (argc > 1) {
		if (GetHexRecordsFromFile(argv[1], HData))
		{
			printf("\n Load hexfile.......OK");
			printf("\n Initializes the CAN with 125 kBaud");
			hResult = SelectDevice(FALSE);
			if (VCI_OK == hResult)
			{
				printf("\n Select Adapter.......... OK !");
				// This step is not necessary but shows how to get/check BAL features
				hResult = CheckBalFeatures(lBusCtlNo);
				if (VCI_OK == hResult)
				{
					printf("\n CheckBalFeatures......... OK !");
				}
				else
				{
					printf("\n CheckBalFeatures......... 0x%08lX !", hResult);
				}

				printf("\n Initialize CAN...");
				hResult = InitSocket(lBusCtlNo);
				if (VCI_OK == hResult)
				{
					printf("\n Initialize CAN............ OK !");

					//
					// start the receive thread
					//
					_beginthread(ReceiveThread, 0, NULL);



					//-------- init Boot_Loader ----------
					MsgId = 0x79;
					MsgLength = 0;
					state = STATE_INIT_BOOT_LOADER;
					TransmitViaPutDataEntry(MsgId, MsgLength, Message);
					Sleep(100);
					if (!(state & STATE_BOOT_LOADER_STARTED))
					{
						MsgId = 0x01;
						TransmitViaPutDataEntry(MsgId, MsgLength, Message);
						Sleep(100);
					}
					if (state & STATE_BOOT_LOADER_STARTED)
					{
						printf("\n BootLoader started........OK");
						//----------- erase -------------
						printf("\n Erase all memory start.....please wait\n");
						MsgId = 0x43;
						MsgLength = 1;
						Message[0] = 0xFF;
						state = STATE_INIT_ERASE;
						TransmitViaPutDataEntry(MsgId, MsgLength, Message);
						Sleep(100);
						if (state & STATE_INIT_ERASE_OK)
						{
							for (UINT16 i = 0; i < MAX_ERASE_TIME; i++)
							{
								if (state & STATE_ERASE_COMPLETE)
								{
									break;
								}
								Sleep(1000);
								printf(" %d", i);
							}
						}
						if (state & STATE_ERASE_COMPLETE)
						{
							printf("\n Erase memory complete\n");
							//---------------- write hex--------------
							UINT16 count = (HData.HexDataLen >> 8);
							UINT16 NBytes = 256;
							UINT16 countlocal = NBytes >> 3;
							UINT16 NByteslocal = 8;
							for (UINT16 k = 0; k <= count; k++)
							{
								printf("\n Write memory %d block  ", k + 1);
								state = STATE_WRITE_START;
								MsgId = 0x31;
								MsgLength = 5;
								UINT32 Adres = HData.StartAdres + (k << 8);
								Message[0] = Adres >> 24;
								Message[1] = Adres >> 16;
								Message[2] = Adres >> 8;
								Message[3] = Adres;
								if (k != count)
								{
									NBytes = 256;
									Message[4] = NBytes - 1;
									countlocal = NBytes >> 3;
								}

								else
								{
									NBytes = (HData.HexDataLen - (count << 8));
									Message[4] = NBytes - 1;
									countlocal = NBytes >> 3;
									if (NBytes - (countlocal << 3) > 0)
										countlocal++;
								}

								TransmitViaPutDataEntry(MsgId, MsgLength, Message);
								for (UINT16 i = 0; i < MAX_COMAND_TIME; i++)
								{
									if (state & STATE_WRITE_START_COMLETE)
									{
										break;
									}
									Sleep(1);
								}
								if (state & STATE_WRITE_START_COMLETE)
								{
									printf(" Started\n");
									MsgId = 0x04;
									MsgLength = 8;
									for (UINT16 n = 0; n < countlocal; n++) {
										NByteslocal = 8;
										if (n == countlocal - 1)
											NByteslocal = (NBytes - ((countlocal - 1) << 3));
										printf("\n%d ", n + 1);
										for (UINT16 m = 0; m < NByteslocal; m++)
											Message[m] = HData.Data[m + (n << 3) + (k << 8)];
										state = STATE_WRITE_DATA_BLOCK;
										TransmitViaPutDataEntry(MsgId, MsgLength, Message);
										for (UINT16 i = 0; i < MAX_COMAND_TIME; i++)
										{
											if (state & STATE_WRITE_DATA_BLOCK_COMPLETE)
											{
												break;
											}
											Sleep(1);
										}
										if (!(state & STATE_WRITE_DATA_BLOCK_COMPLETE))
										{
											printf("Write error");
											FinalizeApp();
											return 6;
										}
									}
								}
								else
								{
									printf("error");
									FinalizeApp();
									return 6;
								}
							}
							printf("\n Write memory complete");
							FinalizeApp();
							return 0;
						}
						else
						{
							printf("\n Erase memory error\n");
							FinalizeApp();
							return 5;
						}
					}
					else
					{
						printf("\n Error BootLoader notstarted");
						FinalizeApp();
						return 4;
					}





				}
				else
				{
					printf("\n No CAN To USB Devices found");
					FinalizeApp();
					return 3;
				}
			}
			else {
				printf("\n Error initialize CAN");
				FinalizeApp();
				return 2;
			}
		}
		else
		{
			printf("\n Invalid hexfile parametrs \n");
			return 1;
		}
	}
	else
	{
		printf("\n Error input parametrs");
		return 1;
	}

	FinalizeApp();
	printf("\n exit");
	return 0;
}

//////////////////////////////////////////////////////////////////////////
/**
  Selects the first CAN adapter.

  @param fUserSelect
	If this parameter is set to TRUE the functions display a dialog box which
	allows the user to select the device.

  @return
	VCI_OK on success, otherwise an Error code
*/
//////////////////////////////////////////////////////////////////////////
HRESULT SelectDevice(BOOL fUserSelect)
{
	HRESULT hResult; // error code

	if (fUserSelect == FALSE)
	{
		IVciDeviceManager* pDevMgr = 0;    // device manager
		IVciEnumDevice* pEnum = 0;    // enumerator handle
		VCIDEVICEINFO       sInfo;          // device info

		hResult = VciGetDeviceManager(&pDevMgr);
		if (hResult == VCI_OK)
		{
			hResult = pDevMgr->EnumDevices(&pEnum);
		}

		//
		// retrieve information about the first
		// device within the device list
		//
		if (hResult == VCI_OK)
		{
			hResult = pEnum->Next(1, &sInfo, NULL);

			//
			// close the device list (no longer needed)
			//
			pEnum->Release();
			pEnum = NULL;
		}

		//
		// open the first device via device manager and get the bal object
		//
		if (hResult == VCI_OK)
		{
			IVciDevice* pDevice;
			hResult = pDevMgr->OpenDevice(sInfo.VciObjectId, &pDevice);

			if (hResult == VCI_OK)
			{
				hResult = pDevice->OpenComponent(CLSID_VCIBAL, IID_IBalObject, (void**)&pBalObject);

				pDevice->Release();
			}
		}

		//
		// always select controller 0
		//
		lBusCtlNo = 0;

		//
		// close device manager
		//
		if (pDevMgr)
		{
			pDevMgr->Release();
			pDevMgr = NULL;
		}
	}
	else
	{
		//
		// open a device selected by the user
		//
		hResult = SocketSelectDlg(NULL, VCI_BUS_CAN, &pBalObject, &lSocketNo, &lBusCtlNo);
	}

	//DisplayError(NULL, hResult);
	return hResult;
}


//////////////////////////////////////////////////////////////////////////
/**

  Checks BAL features

  @param lCtrlNo
	controller number to check the features

  @return
	VCI_OK on success, otherwise an Error code

*/////////////////////////////////////////////////////////////////////////
HRESULT CheckBalFeatures(LONG lCtrlNo)
{
	HRESULT hResult = E_FAIL;

	if (pBalObject)
	{
		// check if controller supports CANFD
		BALFEATURES features = { 0 };
		hResult = pBalObject->GetFeatures(&features);
		if (VCI_OK == hResult)
		{
			// check if controller number is valid
			if (lCtrlNo >= features.BusSocketCount)
			{
				// As we select the controller via the selection dialog, we should never get here.
				printf("\n Invalid controller number. !");
				return VCI_E_UNEXPECTED;
			}

			// check for the expected controller type
			if (VCI_BUS_TYPE(features.BusSocketType[lCtrlNo]) != VCI_BUS_CAN)
			{
				// Invalid controller type selected
				printf("\n Invalid controller type selected !");
				return VCI_E_UNEXPECTED;
			}
		}
		else
		{
			printf("\n pBalObject->GetFeatures failed: 0x%08lX !", hResult);
		}
	}

	return hResult;
}


//////////////////////////////////////////////////////////////////////////
/**
  Opens the specified socket, creates a message channel, initializes
  and starts the CAN controller.

  @param dwCanNo
	Number of the CAN controller to open.

  @return
	VCI_OK on success, otherwise an Error code

  @note
	If <dwCanNo> is set to 0xFFFFFFFF, the function shows a dialog box
	which allows the user to select the VCI device and CAN controller.
*/
//////////////////////////////////////////////////////////////////////////
HRESULT InitSocket(LONG lCtrlNo)
{
	HRESULT hResult = E_FAIL;

	if (pBalObject != NULL)
	{
		//
		// check controller capabilities create a message channel
		//
		ICanSocket* pCanSocket = 0;
		hResult = pBalObject->OpenSocket(lCtrlNo, IID_ICanSocket, (void**)&pCanSocket);
		if (hResult == VCI_OK)
		{
			// check capabilities
			CANCAPABILITIES capabilities = { 0 };
			hResult = pCanSocket->GetCapabilities(&capabilities);
			if (VCI_OK == hResult)
			{
				//
				// This sample expects that standard and extended mode are
				// supported simultaneously. See use of
				// CAN_OPMODE_STANDARD | CAN_OPMODE_EXTENDED in InitLine() below
				//
				if (capabilities.dwFeatures & CAN_FEATURE_STDANDEXT)
				{
					// supports simultaneous standard and extended -> ok
				}
				else
				{
					printf("\n Simultaneous standard and extended mode feature not supported !");
					hResult = VCI_E_NOT_SUPPORTED;
				}
			}
			else
			{
				// should not occurr
				printf("\n pCanSocket->GetCapabilities failed: 0x%08lX !", hResult);
			}

			//
			// create a message channel
			//
			if (VCI_OK == hResult)
			{
				hResult = pCanSocket->CreateChannel(FALSE, &pCanChn);
			}

			pCanSocket->Release();
		}

		//
		// initialize the message channel
		//
		if (hResult == VCI_OK)
		{
			UINT16 wRxFifoSize = 1024;
			UINT16 wRxThreshold = 1;
			UINT16 wTxFifoSize = 128;
			UINT16 wTxThreshold = 1;

			hResult = pCanChn->Initialize(wRxFifoSize, wTxFifoSize);
			if (hResult == VCI_OK)
			{
				hResult = pCanChn->GetReader(&pReader);
				if (hResult == VCI_OK)
				{
					pReader->SetThreshold(wRxThreshold);

					hEventReader = CreateEvent(NULL, FALSE, FALSE, NULL);
					pReader->AssignEvent(hEventReader);
				}
			}

			if (hResult == VCI_OK)
			{
				hResult = pCanChn->GetWriter(&pWriter);
				if (hResult == VCI_OK)
				{
					pWriter->SetThreshold(wTxThreshold);
				}
			}
		}

		//
		// activate the CAN channel
		//
		if (hResult == VCI_OK)
		{
			hResult = pCanChn->Activate();
		}

		//
		// Open the CAN control interface
		//
		// During the programs lifetime we have multiple options:
		// 1) Open the control interface and keep it open
		//     -> No other programm is able to get the control interface and change the line settings
		// 2) Try to get the control interface and change the settings only when we get it
		//     -> Other programs can change the settings by getting the control interface
		//
		if (hResult == VCI_OK)
		{
			hResult = pBalObject->OpenSocket(lCtrlNo, IID_ICanControl, (void**)&pCanControl);

			//
			// initialize the CAN controller
			//
			if (hResult == VCI_OK)
			{
				CANINITLINE init = {
				  CAN_OPMODE_STANDARD |
				  CAN_OPMODE_EXTENDED | CAN_OPMODE_ERRFRAME,      // opmode
				  0,                                              // bReserved
				  CAN_BT0_125KB, CAN_BT1_125KB                    // bt0, bt1
				};

				hResult = pCanControl->InitLine(&init);
				if (hResult != VCI_OK)
				{
					printf("\n pCanControl->InitLine failed: 0x%08lX !", hResult);
				}

				//
				// set the acceptance filter
				//
				if (hResult == VCI_OK)
				{
					hResult = pCanControl->SetAccFilter(CAN_FILTER_STD, CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL);

					//
					// set the acceptance filter
					//
					if (hResult == VCI_OK)
					{
						hResult = pCanControl->SetAccFilter(CAN_FILTER_EXT, CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL);
					}

					if (VCI_OK != hResult)
					{
						printf("\n pCanControl->SetAccFilter failed: 0x%08lX !", hResult);
					}

					//
					// SetAccFilter() returns VCI_E_INVALID_STATE if already controller is started. 
					// We ignore this because the controller could already be started
					// by another application.
					//
					if (VCI_E_INVALID_STATE == hResult)
					{
						hResult = VCI_OK;
					}
				}

				//
				// start the CAN controller
				//
				if (hResult == VCI_OK)
				{
					hResult = pCanControl->StartLine();
					if (hResult != VCI_OK)
					{
						printf("\n pCanControl->StartLine failed: 0x%08lX !", hResult);
					}
				}

				printf("\n Got Control interface. Settings applied !");
			}
			else
			{
				//
				// If we can't get the control interface it is occupied by another application.
				// This means the application is in charge of the controller parameters.
				// We live with it and move on.
				// 
				printf("\n Control interface occupied. Settings not applied: 0x%08lX !", hResult);
				hResult = VCI_OK;
			}
		}
	}
	else
	{
		hResult = VCI_E_INVHANDLE;
	}

	DisplayError(NULL, hResult);
	return hResult;
}

//////////////////////////////////////////////////////////////////////////
/**

  Transmit message via PutDataEntry

*/////////////////////////////////////////////////////////////////////////
void TransmitViaPutDataEntry(UINT32 MsgId, UINT payloadLen, UINT8* Msg)
{
	CANMSG  sCanMsg = { 0 };

	// length of message payload
	//UINT payloadLen = 8;

	sCanMsg.dwTime = 0;
	sCanMsg.dwMsgId = MsgId;    // CAN message identifier

	sCanMsg.uMsgInfo.Bytes.bType = CAN_MSGTYPE_DATA;
	// Flags:
	// srr = 1
	sCanMsg.uMsgInfo.Bytes.bFlags = CAN_MAKE_MSGFLAGS(CAN_LEN_TO_SDLC(payloadLen), 0, 1, 0, 0);
	// Flags2:
	// Set bFlags2 to 0
	sCanMsg.uMsgInfo.Bytes.bFlags2 = CAN_MAKE_MSGFLAGS2(0, 0, 0, 0, 0);

	for (UINT i = 0; i < payloadLen; i++)
	{
		sCanMsg.abData[i] = Msg[i];
	}

	// write a single CAN message into the transmit FIFO
	while (VCI_E_TXQUEUE_FULL == pWriter->PutDataEntry(&sCanMsg))
	{
		Sleep(1);
	}
}

//////////////////////////////////////////////////////////////////////////
/**

  Transmit CAN via Writer API (AcquireWrite/ReleaseWrite)
   with ID 0x100.

*/////////////////////////////////////////////////////////////////////////
void TransmitViaWriter()
{
	// use the FIFO interface 
	// to write multiple messages
	UINT16  count = 0;
	PCANMSG pMsg;

	// length of message payload
	UINT payloadLen = 8;

	// aquire write access to FIFO
	HRESULT hr = pWriter->AcquireWrite((void**)&pMsg, &count);
	if (VCI_OK == hr)
	{
		// number of written messages needed for ReleaseWrite
		UINT16 written = 0;

		if (count > 0)
		{
			pMsg->dwTime = 0;
			pMsg->dwMsgId = 0x200;

			pMsg->uMsgInfo.Bytes.bType = CAN_MSGTYPE_DATA;
			// Flags:
			// srr = 1
			pMsg->uMsgInfo.Bytes.bFlags = CAN_MAKE_MSGFLAGS(CAN_LEN_TO_SDLC(payloadLen), 0, 1, 0, 0);
			// Flags2:
			// Set bFlags2 to 0 because FIFO memory will not be initialized by AquireWrite
			pMsg->uMsgInfo.Bytes.bFlags2 = CAN_MAKE_MSGFLAGS2(0, 0, 0, 0, 0);

			for (UINT i = 0; i < payloadLen; i++)
			{
				pMsg->abData[i] = i;
			}

			written = 1;
		}

		// release write access to FIFO
		hr = pWriter->ReleaseWrite(written);
		if (VCI_OK != hr)
		{
			printf("\nReleaseWrite failed: 0x%08lX", hr);
		}
	}
	else
	{
		printf("\nAcquireWrite failed: 0x%08lX", hr);
	}
}

//////////////////////////////////////////////////////////////////////////
/**

  Print a message

*/
//////////////////////////////////////////////////////////////////////////
void PrintMessage(PCANMSG pCanMsg)
{
	if (pCanMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_DATA)
	{
		//
		// show data frames
		//
		if (pCanMsg->uMsgInfo.Bits.rtr == 0)
		{
			UINT j;

			// number of bytes in message payload
			UINT payloadLen = CAN_SDLC_TO_LEN(pCanMsg->uMsgInfo.Bits.dlc);

			printf("\nTime: %10u  ID: %3X %s  Len: %1u  Data:",
				pCanMsg->dwTime,
				pCanMsg->dwMsgId,
				(pCanMsg->uMsgInfo.Bits.ext == 1) ? "Ext" : "Std",
				payloadLen);

			// print payload bytes
			if ((state & STATE_INIT_BOOT_LOADER) && (pCanMsg->abData[0] == 0x79))
			{
				state &= ~STATE_INIT_BOOT_LOADER;
				state |= STATE_BOOT_LOADER_STARTED;
			}
			else
			{
				if ((state & STATE_INIT_ERASE) && (pCanMsg->abData[0] == 0x79))
				{
					state &= ~STATE_INIT_ERASE;
					state |= STATE_INIT_ERASE_OK;
				}
				else
				{
					if ((state & STATE_INIT_ERASE_OK) && (pCanMsg->abData[0] == 0x79))
					{
						state &= ~STATE_INIT_ERASE_OK;
						state |= STATE_ERASE_COMPLETE;
					}
					else
					{
						if ((state & STATE_WRITE_START) && (pCanMsg->abData[0] == 0x79))
						{
							state &= ~STATE_WRITE_START;
							state |= STATE_WRITE_START_COMLETE;
						}
						else
						{
							if ((state & STATE_WRITE_DATA_BLOCK) && (pCanMsg->abData[0] == 0x79))
							{
								state &= ~STATE_WRITE_DATA_BLOCK;
								state |= STATE_WRITE_DATA_BLOCK_COMPLETE;
							}
						}
					}
				}
			}
			for (j = 0; j < payloadLen; j++)
			{
				printf(" %.2X", pCanMsg->abData[j]);
			}
		}
		else
		{
			printf("\nTime: %10u ID: %3X  DLC: %1u  Remote Frame",
				pCanMsg->dwTime,
				pCanMsg->dwMsgId,
				pCanMsg->uMsgInfo.Bits.dlc);
		}
	}
	else if (pCanMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_INFO)
	{
		//
		// show informational frames
		//
		switch (pCanMsg->abData[0])
		{
		case CAN_INFO_START: printf("\nCAN started..."); break;
		case CAN_INFO_STOP: printf("\nCAN stopped..."); break;
		case CAN_INFO_RESET: printf("\nCAN reseted..."); break;
		}
	}
	else if (pCanMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_ERROR)
	{
		//
		// show error frames
		//
		switch (pCanMsg->abData[0])
		{
		case CAN_ERROR_STUFF: printf("\nstuff error...");          break;
		case CAN_ERROR_FORM: printf("\nform error...");           break;
		case CAN_ERROR_ACK: printf("\nacknowledgment error..."); break;
		case CAN_ERROR_BIT: printf("\nbit error...");            break;
		case CAN_ERROR_CRC: printf("\nCRC error...");            break;
		case CAN_ERROR_OTHER:
		default: printf("\nother error...");          break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
/**

  Process messages

  @param wLimit  max number of messages to process

  @return VCI_OK if more messages (may be) available

*/
//////////////////////////////////////////////////////////////////////////
HRESULT ProcessMessages(WORD wLimit)
{
	// parameter checking
	if (!pReader) return E_UNEXPECTED;

	PCANMSG pCanMsg;

	// check if messages available
	UINT16  wCount = 0;
	HRESULT hr = pReader->AcquireRead((PVOID*)&pCanMsg, &wCount);
	if (VCI_OK == hr)
	{
		// limit number of messages to read
		if (wCount > wLimit)
		{
			wCount = wLimit;
		}

		UINT16 iter = wCount;
		while (iter)
		{
			PrintMessage(pCanMsg);

			// process next VCI message
			iter--;
			pCanMsg++;
		}
		pReader->ReleaseRead(wCount);
	}
	else if (VCI_E_RXQUEUE_EMPTY == hr)
	{
		// return error code
	}
	else
	{
		// ignore all other errors
		hr = VCI_OK;
	}

	return hr;
}


//////////////////////////////////////////////////////////////////////////
/**
  Receive thread.

  Note:
	Here console output in the receive thread is used for demonstration purposes.
	Using console outout in the receive thread involves Asynchronous
	Local Procedure Calls (ALPC) with the console host application (conhost.exe).
	So expect console output to be slow.
	Slow output can stall receive queue handling and finally lead
	to controller overruns on some CAN interfaces, even with moderate busloads
	(moderate = 1000 kBit/s, dlc=8, busload >= 30%).

  @param Param
	ptr on a user defined information
*/
//////////////////////////////////////////////////////////////////////////
void ReceiveThread(void* Param)
{
	UNREFERENCED_PARAMETER(Param);

	BOOL receiveSignaled = FALSE;
	BOOL moreMsgMayAvail = FALSE;

	while (lMustQuit == 0)
	{
		// if no more messages available wait 100msec for reader event
		if (!moreMsgMayAvail)
		{
			receiveSignaled = (WAIT_OBJECT_0 == WaitForSingleObject(hEventReader, 100));
		}

		// process messages while messages are available
		if (receiveSignaled || moreMsgMayAvail)
		{
			// try to process next chunk of messages (with max 100 msgs)
			moreMsgMayAvail = (VCI_OK == ProcessMessages(100));
		}
	}

	_endthread();
}

//////////////////////////////////////////////////////////////////////////
/**
  Finalizes the application
*/
//////////////////////////////////////////////////////////////////////////
void FinalizeApp()
{
	//
	// release reader
	//
	if (pReader)
	{
		pReader->Release();
		pReader = 0;
	}

	//
	// release writer
	//
	if (pWriter)
	{
		pWriter->Release();
		pWriter = 0;
	}

	//
	// release channel interface
	//
	if (pCanChn)
	{
		pCanChn->Release();
		pCanChn = 0;
	}

	//
	// release CAN control object
	//
	if (pCanControl)
	{
		HRESULT hResult = pCanControl->StopLine();
		if (hResult != VCI_OK)
		{
			printf("\n pCanControl->StopLine failed: 0x%08lX !", hResult);
		}

		hResult = pCanControl->ResetLine();
		if (hResult != VCI_OK)
		{
			printf("\n pCanControl->ResetLine failed: 0x%08lX !", hResult);
		}

		pCanControl->Release();
		pCanControl = NULL;
	}

	//
	// release bal object
	//
	if (pBalObject)
	{
		pBalObject->Release();
		pBalObject = NULL;
	}
}

