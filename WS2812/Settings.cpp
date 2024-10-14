//---------------------------------------------------------------------------


#pragma hdrstop

#include "Settings.h"
#include <algorithm>
#include <fstream>
#include <json/json.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

Settings appSettings;

inline void strncpyz(char* dst, const char* src, int dstsize) {
	strncpy(dst, src, dstsize);
	dst[dstsize-1] = '\0';
}

int Settings::Read(AnsiString asFileName)
{
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;

	try
	{
		std::ifstream ifs(asFileName.c_str());
		std::string strConfig((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		ifs.close();
		bool parsingSuccessful = reader.parse( strConfig, root );
		if ( !parsingSuccessful )
		{
			return 2;
		}
	}
	catch(...)
	{
		return 1;
	}

	{
		const Json::Value &jv = root["serialPort"];
		jv.getAString("name", serialPort.name);
		jv.getInt("baud", serialPort.baud);
		jv.getAString("parity", serialPort.parity);
		jv.getInt("data_bit", serialPort.data_bit);
		jv.getInt("stop_bit", serialPort.stop_bit);
		jv.getBool("openAtStartup", serialPort.openAtStartup);
		jv.getBool("autoReinit", serialPort.autoReinit);
	}

	return 0;
}

int Settings::Write(AnsiString asFileName)
{
	Json::Value root;
	Json::StyledWriter writer;

	{
		Json::Value &jv = root["serialPort"];
		jv["name"] = serialPort.name;
		jv["baud"] = serialPort.baud;
		jv["parity"] = serialPort.parity;
		jv["data_bit"] = serialPort.data_bit;
		jv["stop_bit"] = serialPort.stop_bit;
		jv["openAtStartup"] = serialPort.openAtStartup;
		jv["autoReinit"] = serialPort.autoReinit;
	}

	std::string outputConfig = writer.write( root );

	try
	{
		std::ofstream ofs(asFileName.c_str());
		ofs << outputConfig;
		ofs.close();
	}
	catch(...)
	{
    	return 1;
	}

	return 0;
}


