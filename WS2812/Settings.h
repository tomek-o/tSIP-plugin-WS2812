/** \file
*/
//---------------------------------------------------------------------------

#ifndef SettingsH
#define SettingsH
//---------------------------------------------------------------------------
#include <System.hpp>
#include <deque>

class Settings
{
public:
	int Read(AnsiString asFileName);
	int Write(AnsiString asFileName);
	struct SerialPort
	{
		AnsiString name;
		int baud;
		AnsiString parity;
		int data_bit;
		int stop_bit;
		bool openAtStartup;
		bool autoReinit;
		static const AnsiString namePrefix;
		SerialPort(void):
			name("COM10"),
			baud(2000000),
			parity("N"),
			data_bit(7),
			stop_bit(1),
			openAtStartup(true),
			autoReinit(true)
		{}
	} serialPort;
};

extern Settings appSettings;

#endif
