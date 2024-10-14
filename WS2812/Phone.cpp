//---------------------------------------------------------------------------

#pragma hdrstop

#define _EXPORTING
#include "..\..\tSIP\tSIP\phone\Phone.h"
#include "..\..\tSIP\tSIP\phone\PhoneSettings.h"
#include "..\..\tSIP\tSIP\phone\PhoneCapabilities.h"
#include "PhoneLocal.h"
#include "Log.h"
#include <assert.h>
#include <algorithm>	// needed by Utils::in_group
#include "Utils.h"
#include "Settings.h"
#include "ComPort.h"
#include "WS2812Write.h"
#include <string>

#pragma link "jsoncpp.lib"


//---------------------------------------------------------------------------

static const struct S_PHONE_DLL_INTERFACE dll_interface =
{DLL_INTERFACE_MAJOR_VERSION, DLL_INTERFACE_MINOR_VERSION};

// callback ptrs
CALLBACK_LOG lpLogFn = NULL;
CALLBACK_CONNECT lpConnectFn = NULL;
CALLBACK_KEY lpKeyFn = NULL;
CALLBACK_PAGING_TX lpPagingTxFn = NULL;
CALLBACK_CLEAR_DIAL lpClearDialFn = NULL;
CALLBACK_SET_VARIABLE lpSetVariableFn = NULL;
CALLBACK_CLEAR_VARIABLE lpClearVariableFn = NULL;

void *callbackCookie;	///< used by upper class to distinguish library instances when receiving callbacks


namespace {
	std::string configPath = Utils::ReplaceFileExtension(Utils::GetDllPath(), ".cfg");
}

void __stdcall GetPhoneInterfaceDescription(struct S_PHONE_DLL_INTERFACE* interf) {
    interf->majorVersion = dll_interface.majorVersion;
    interf->minorVersion = dll_interface.minorVersion;
}

void Log(char* txt) {
    if (lpLogFn)
        lpLogFn(callbackCookie, txt);
}

void Connect(int state, char *szMsgText) {
    if (lpConnectFn)
        lpConnectFn(callbackCookie, state, szMsgText);
}

void Key(int keyCode, int state) {
    if (lpKeyFn)
        lpKeyFn(callbackCookie, keyCode, state);
}

int PagingTx(const char* target, const char* filename, const char* codecname) {
	if (lpPagingTxFn) {
		return lpPagingTxFn(callbackCookie, target, filename, codecname);
	}
	return -1;
}

int ClearDial(void) {
	if (lpClearDialFn) {
		lpClearDialFn(callbackCookie);
		return 0;
	}
    return -1;
}

int SetVariable(const char* name, const char* value) {
	if (lpSetVariableFn) {
		return lpSetVariableFn(callbackCookie, name, value);
	}
	return -1;
}

int ClearVariable(const char* name) {
	if (lpClearVariableFn) {
		return lpClearVariableFn(callbackCookie, name);
	}
	return -1;
}

void __stdcall SetCallbacks(void *cookie, CALLBACK_LOG lpLog, CALLBACK_CONNECT lpConnect, CALLBACK_KEY lpKey) {
    assert(cookie && lpLog && lpConnect && lpKey);
    lpLogFn = lpLog;
    lpConnectFn = lpConnect;
    lpKeyFn = lpKey;
    callbackCookie = cookie;
    lpLogFn(callbackCookie, "Plugin DLL for WS2812 loaded\n");
}

void __stdcall GetPhoneCapabilities(struct S_PHONE_CAPABILITIES **caps) {
    static struct S_PHONE_CAPABILITIES capabilities = {
        0
    };
    *caps = &capabilities;
}

void __stdcall ShowSettings(HANDLE parent) {
    MessageBox((HWND)parent, "No settings window - edit JSON config file manually.", "Device DLL", MB_ICONINFORMATION);
}

int __stdcall Connect(void) {
	appSettings.Read(configPath.c_str());
	LOG("Settings loaded");

	return 0; //device.Start();
}

int __stdcall Disconnect(void) {
	appSettings.Write(configPath.c_str());

	comPort.Close();	
    return 0;
}

static bool bSettingsReaded = false;

static int GetDefaultSettings(struct S_PHONE_SETTINGS* settings) {
    settings->ring = 1;

    bSettingsReaded = true;
    return 0;
}

int __stdcall GetPhoneSettings(struct S_PHONE_SETTINGS* settings) {
    //settings->iTriggerSrcChannel = 0;

    std::string path = Utils::GetDllPath();
    path = Utils::ReplaceFileExtension(path, ".cfg");
    if (path == "")
        return GetDefaultSettings(settings);


    GetDefaultSettings(settings);


    //int mode = root.get("TriggerMode", TRIGGER_MODE_AUTO).asInt();
    settings->ring = true;//root.get("ring", settings->ring).asInt();


    bSettingsReaded = true;
    return 0;
}

int __stdcall SavePhoneSettings(struct S_PHONE_SETTINGS* settings) {

    return 0;
}

int __stdcall SetRegistrationState(int state) {
//	phoneState.registered = state;
	return 0;
}

int __stdcall SetCallState(int state, const char* display) {
//	phoneState.callState = state;
//	phoneState.display = display;
	return 0;
}


int __stdcall Ring(int state) {
    return 0;
}

void __stdcall SetPagingTxCallback(CALLBACK_PAGING_TX lpPagingTx) {
	lpPagingTxFn = lpPagingTx;
}

int __stdcall SetPagingTxState(int state) {
//	phoneState.paging = state;
	return 0;
}

void __stdcall SetClearDialCallback(CALLBACK_CLEAR_DIAL lpClearDial) {
	lpClearDialFn = lpClearDial;
}

void __stdcall SetSetVariableCallback(CALLBACK_SET_VARIABLE lpFn) {
	lpSetVariableFn = lpFn;
}

void __stdcall SetClearVariableCallback(CALLBACK_CLEAR_VARIABLE lpFn) {
	lpClearVariableFn = lpFn;
}

inline bool StartsWith(const char* &a, const char *b)
{
	if(strlen(a) < strlen(b)) {
		return false;
	}
	else {
		int len = strlen(b);
		bool result = !strnicmp(a,b,len); //case insensitive
		if(result)
			a += len;
		return result;
	}
}

int __stdcall SendMessageText(const char* text) {
	//LOG("received message: %s", text);

	if (StartsWith(text, "WRITE ")) {
		if (!comPort.isOpened())
		{
			const AnsiString namePrefix = "\\\\.\\";
			AnsiString name = appSettings.serialPort.name;
			if (name.Pos(namePrefix) != 1)
			{
				name = namePrefix + name;
			}

			int status = comPort.Open(name.c_str(), appSettings.serialPort.baud, false);
			AnsiString text;
			if (status == 0)
				LOG("%s opened, %u bps\n", appSettings.serialPort.name.c_str(), appSettings.serialPort.baud);
			else
				LOG("Failed to open %s, check config file\n", appSettings.serialPort.name.c_str());
		}
		if (!comPort.isOpened())
		{
			return -1;
		}

		std::vector<struct Ws2812Color> colors;

		const char* p = text;
		const char* textEnd = text + strlen(text);
		char colorId = 'r';
		struct Ws2812Color c;

		while(p < textEnd ) {
			char* end;
			long int value = strtol( p , &end , 10 );
			if( value == 0L && end == p )  //docs also suggest checking errno value
				break;

			uint8_t value8 = static_cast<uint8_t>(value);
			if (colorId == 'r')
			{
				colorId = 'g';
				c.r = value8;
			}
			else if (colorId == 'g')
			{
				colorId = 'b';
				c.g = value8;
			}
			else
			{
				colorId = 'r';
				c.b = value8;
				colors.push_back(c);
				// reset 
				c.r = 0;
				c.g = 0;
				c.b = 0;
			}

			p = end ;
		}

		if (!colors.empty())
		{
        	Ws2812Write(colors);
		}
	}

	return 0;
}

