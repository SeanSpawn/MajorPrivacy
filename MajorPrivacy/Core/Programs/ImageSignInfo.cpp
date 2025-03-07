#include "pch.h"
#include "ImageSignInfo.h"
#include "../../Library/API/DriverAPI.h"

CImageSignInfo::CImageSignInfo()
{
}

XVariant CImageSignInfo::ToVariant(const SVarWriteOpt& Opts) const
{
	XVariant Data;
	if (Opts.Format == SVarWriteOpt::eIndex) {
		Data.BeginIMap();
		WriteIVariant(Data, Opts);
	} else {  
		Data.BeginMap();
		WriteMVariant(Data, Opts);
	}
	Data.Finish();
	return Data;
}

NTSTATUS CImageSignInfo::FromVariant(const class XVariant& Data)
{
	if (Data.GetType() == VAR_TYPE_MAP)         Data.ReadRawMap([&](const SVarName& Name, const CVariant& Data) { ReadMValue(Name, Data); });
	else if (Data.GetType() == VAR_TYPE_INDEX)  Data.ReadRawIMap([&](uint32 Index, const CVariant& Data)        { ReadIValue(Index, Data); });
	else
		return STATUS_UNKNOWN_REVISION;
	return STATUS_SUCCESS;
}