#pragma once
#pragma warning(disable:4786) 
#pragma warning(disable:4996) 
#include "stdafx.h"
#include <iostream>
#include <string>
#import  ".\\dll\\ClearImage.x64.dev.dll" exclude("LONG_PTR") exclude("ULONG_PTR") no_namespace  named_guids
void dump_com_error( _com_error &e );
bool ReadBarcodeBasic (ICiServerPtr &Ci, char *sFileName, long nPage);
bool ReadBarcodePro (ICiServerPtr &Ci, char *sFileName, long nPage);
bool ReadBarcodePdf417 (ICiServerPtr &Ci, ICiImagePtr &Image);
bool ReadBarcodeDataMatrix (ICiServerPtr &Ci, ICiImagePtr &Image);
bool ReadBarcodeQR (ICiServerPtr &Ci, ICiImagePtr &Image);
bool ReadBarcodePro_my(ICiBarcodeProPtr &BcIter, char *sFileName, long nPage,std::string &res);