//===================================================================
// This file is provided as part of the ClearImage COM Development Kit
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT  
// WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED,  
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES  
// OF MERCHANTABILITY AND/OR FITNESS FOR A  PARTICULAR PURPOSE. 
// 
// Copyright (C) 2003-2010.  Inlite Research, Inc. All rights reserved 
//===================================================================

#include "stdafx.h"
#include <stdio.h>			// sprintf 
#include "ClearImageBarcode.h"

using namespace std;

void dump_com_error( _com_error &e ) 
{ 
	char sMsg[256];
	if (e.Description().length())
		sprintf (sMsg, "=====> COM error: Code: %08lx  Description: %s" , 
						e.Error(), (LPCSTR)e.Description()); 
	else
		sprintf (sMsg, "=====> COM error: Code: %08lx  Message: %s" , 
						e.Error(), (LPCSTR)e.ErrorMessage()); 
		
	cout << sMsg << endl;	
}

void DisplayBarcode (ICiBarcodePtr &Bc)
{
	cout <<  "  Type=" << Bc->Type;
	cout <<  "  Length=" << Bc->Length;
	cout <<  "  Text=" <<  Bc->Text;
	cout <<  "  Rect=" <<  Bc->Rect->left << ":"  << Bc->Rect->top << " - "  << Bc->Rect->right << ":"  << Bc->Rect->bottom;
	cout <<  endl;
}

//========================================================
void DisplayHexBarcodeData (ICiBarcodePtr &Bc)
{
	VARIANT &pData = Bc->Data;
	long UpperBounds, LowerBounds, i;
	unsigned char*	pByte; 
		// Check array structure
	if (pData.vt != (VT_ARRAY | VT_UI1)) 
		goto DoError;
	if (SafeArrayGetDim(pData.parray) != 1) 
		goto DoError;
	SafeArrayGetLBound(pData.parray, 1, &LowerBounds);
  SafeArrayGetUBound(pData.parray, 1, &UpperBounds);
	if (LowerBounds != 0) 
		goto DoError;
		//  Access bytes in safe array
	SafeArrayAccessData (pData.parray, reinterpret_cast <void **> (&pByte));
	cout << "HEX DATA   LBound=" << LowerBounds << "  UBound=" << UpperBounds << endl;
	for (i = 0; i <= UpperBounds; i++, pByte++)
		{
		if (!(i % 16) && i)
			cout << endl;	// 16 char in a row
		char buf[10];  sprintf(buf, "%02x", *pByte);
		cout << buf << " ";
		}
	cout << endl;
		// Terminate
	SafeArrayUnaccessData(pData.parray);
	return;

DoError:
	cout << " Incorrect Data format" << endl;
	return;
}
//########################################################################
bool ReadBarcodeBasic (ICiServerPtr &Ci, char *sFileName, long nPage)
	{
	try 
		{
		ICiBarcodeBasicPtr BcIter;	
		BcIter =  Ci->CreateBarcodeBasic ();
			//  Open image
		BcIter->Image->Open (_bstr_t(sFileName), nPage);

			// =========================================
		cout << "===========================" << endl <<
						 "CiBarcodeBasic: Find 3 of any 1D barcodes  on '"<< sFileName << "' Page:" << nPage << endl;
		BcIter->AutoDetect1D = ciTrue;
		BcIter->ValidateOptChecksum = ciFalse;
		ICiBarcodePtr Barcode;
		Barcode = BcIter->FirstBarcode();
		while (Barcode)
			{
			DisplayBarcode (Barcode);
			if (BcIter->Barcodes->Count == 3)
				break;
			Barcode = BcIter->NextBarcode();
			}
		cout << "  TOTAL " << BcIter->Barcodes->Count << " barcodes" << endl;
			// =========================================
		cout << "===========================" << endl <<
						 "CiBarcodeBasic: Find all Cod93 and Code128 barcodes on '"<< sFileName << "'   Page:" << nPage << endl;
		BcIter->AutoDetect1D = ciFalse;
		BcIter->ValidateOptChecksum = ciFalse;
		BcIter->Type = (FBarcodeType) (cibfCode128 | cibfCode93);
		BcIter->Find(0);
		for (int i = 1 ; i <= BcIter->Barcodes->Count ; i++)
			DisplayBarcode (BcIter->Barcodes->Item[i]);
		cout << "  TOTAL " << BcIter->Barcodes->Count << " barcodes" << endl;
		return true;
		}
	catch (_com_error &ex)
		{
		dump_com_error(ex);
		return false;
		}
	}

bool ReadBarcodePro_my (ICiBarcodeProPtr &BcIter, char *sFileName, long nPage,string &res)
	{
	try 
		{
		//BcIter =  Ci->CreateBarcodePro ();
			//  Open image
		BcIter->Image->Open (_bstr_t(sFileName), nPage);
			// =========================================
		//cout << "===========================" << endl <<
						 //"CiBarcodePro: All 1D Barcodes and Postnet on '"<< sFileName << "' Page:" << nPage << endl;
		BcIter->Find(0);
		//cout << "text = ";
		for (int i = 1 ; i <= BcIter->Barcodes->Count ; i++)
			res.append(BcIter->Barcodes->Item[i]->Text);
			//cout << BcIter->Barcodes->Item[i]->Text;
			//DisplayBarcode (BcIter->Barcodes->Item[i]);
		//cout << "  TOTAL " << BcIter->Barcodes->Count << " barcodes" << endl;
		return true;
		}
	catch (_com_error &ex)
		{
		dump_com_error(ex);
		return false;
		}
	}

//########################################################################
bool ReadBarcodePro (ICiServerPtr &Ci, char *sFileName, long nPage)
	{
	try 
		{
		ICiBarcodeProPtr BcIter;	
		BcIter =  Ci->CreateBarcodePro ();
			//  Open image
		BcIter->Image->Open (_bstr_t(sFileName), nPage);
			// =========================================
		//cout << "===========================" << endl <<
						 //"CiBarcodePro: All 1D Barcodes and Postnet on '"<< sFileName << "' Page:" << nPage << endl;
		BcIter->AutoDetect1D = ciTrue;
		BcIter->ValidateOptChecksum = ciFalse;
		BcIter->Directions = (FBarcodeDirections)(cibHorz | cibVert | cibDiag);
		BcIter->Type = (FBarcodeType) (cibfPostnet);
		BcIter->Encodings = (EBarcodeEncoding)106;
		BcIter->Algorithm = cibBestRecognition;
		BcIter->Find(0);
		cout << "text = ";
		for (int i = 1 ; i <= BcIter->Barcodes->Count ; i++)
			cout << BcIter->Barcodes->Item[i]->Text;
			//DisplayBarcode (BcIter->Barcodes->Item[i]);
		//cout << "  TOTAL " << BcIter->Barcodes->Count << " barcodes" << endl;
		return true;
		}
	catch (_com_error &ex)
		{
		dump_com_error(ex);
		return false;
		}
	}

bool ReadBarcodePro (ICiServerPtr &Ci, ICiImagePtr &Image)
	{
	try 
		{
		ICiBarcodeProPtr BcIter;	
		BcIter =  Ci->CreateBarcodePro ();
			//  Attach image
		BcIter->Image = Image;
			// =========================================
		cout << "===========================" << endl <<
						 "CiBarcodePro: All 1D Barcodes and Postnet on '" << Image->FileName << "' Page:" << Image->PageNumber << endl;
		BcIter->ValidateOptChecksum = ciFalse;
		BcIter->Directions = (FBarcodeDirections)(cibHorz | cibVert | cibDiag);
		BcIter->Type = (FBarcodeType) (cibfPostnet);
		BcIter->Algorithm = cibBestRecognition;
		BcIter->Find(0);
		cout << " Text=";
		for (int i = 1 ; i <= BcIter->Barcodes->Count ; i++)
			//cout << BcIter->Barcodes->Item[i]->Text;
			DisplayBarcode (BcIter->Barcodes->Item[i]);
		cout << "  TOTAL " << BcIter->Barcodes->Count << " barcodes" << endl;
		return true;
		}
	catch (_com_error &ex)
		{
		dump_com_error(ex);
		return false;
		}
	}


//########################################################################
bool ReadBarcodePdf417 (ICiServerPtr &Ci, ICiImagePtr &Image)
	{
	try 
		{ 
		ICiPdf417Ptr BcIter;	
		BcIter =  Ci->CreatePdf417 ();
			//  Attach image
		BcIter->Image = Image;
			// =========================================
		cout << "===========================" << endl <<
						 "CiPdf417: Find all on '"<< Image->FileName << "' Page:" << Image->PageNumber << endl;
		BcIter->Directions = (FBarcodeDirections)(cibHorz | cibVert | cibDiag);
		BcIter->Algorithm = cibBestRecognition;
		BcIter->Find(0);
		for (int i = 1 ; i <= BcIter->Barcodes->Count ; i++)
			{
			if ((BcIter->Barcodes->Item[i])->IsChecksumVerified == ciTrue)
				{
				DisplayBarcode (BcIter->Barcodes->Item[i]);
				DisplayHexBarcodeData (BcIter->Barcodes->Item[i]);
				}
			}
		cout << "  TOTAL " << BcIter->Barcodes->Count << " barcodes" << endl;
		return true;
		}
	catch (_com_error &ex)
		{
		dump_com_error(ex);
		return false;
		}
	}

//########################################################################
bool ReadBarcodeDataMatrix (ICiServerPtr &Ci, ICiImagePtr &Image)
	{
	try 
		{
		ICiDataMatrixPtr BcIter;	
		BcIter =  Ci->CreateDataMatrix ();
			//  Attach image
		BcIter->Image = Image;
			// =========================================
		cout << "===========================" << endl <<
						 "CiDataMatrix: Find all on '"<< Image->FileName << "' Page:" << Image->PageNumber << endl;
		BcIter->Directions = (FBarcodeDirections)(cibHorz | cibVert | cibDiag);
		BcIter->Algorithm = cibBestRecognition;
		BcIter->Find(0);
		for (int i = 1 ; i <= BcIter->Barcodes->Count ; i++)
			DisplayBarcode (BcIter->Barcodes->Item[i]);
		cout << "  TOTAL " << BcIter->Barcodes->Count << " barcodes" << endl;
		return true;
		}
	catch (_com_error &ex)
		{
		dump_com_error(ex);
		return false;
		}
	}



//########################################################################
bool ReadBarcodeQR (ICiServerPtr &Ci, ICiImagePtr &Image)
	{
	try 
		{
		ICiQRPtr BcIter;	
		BcIter =  Ci->CreateQR ();
			//  Attach image
		BcIter->Image = Image;
			// =========================================
		cout << "===========================" << endl <<
						 "CiQR: Find all on '"<< Image->FileName << "' Page:" << Image->PageNumber << endl;
		BcIter->Directions = (FBarcodeDirections)(cibHorz | cibVert | cibDiag);
		BcIter->Algorithm = cibBestRecognition;
		BcIter->Find(0);
		for (int i = 1 ; i <= BcIter->Barcodes->Count ; i++)
			DisplayBarcode (BcIter->Barcodes->Item[i]);
		cout << "  TOTAL " << BcIter->Barcodes->Count << " barcodes" << endl;
		return true;
		}
	catch (_com_error &ex)
		{
		dump_com_error(ex);
		return false;
		}
	}


