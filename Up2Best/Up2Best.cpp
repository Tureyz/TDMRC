//===========================================================================
//===========================================================================
//===========================================================================
//==  UP2Best. Author: Costin-Anton BOIANGIU
//===========================================================================
//===========================================================================
//===========================================================================

#include "stdafx.h"
#include "Direct_Access_Image.h"
#include "Resample.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 3)
	{
		_tprintf(_T("Invalid program usage, correct syntax is: %s <Input Folder> <Output Folder> <CR>!\n"), argv[0]);
		getchar();
		return -1;
	}

	int intStatDataCount = 0;
	long double dblMSESum = 0.0;

	TCHAR szInputPath[_MAX_PATH], szOutputPath[_MAX_PATH], szFileMask[_MAX_PATH], szFileName[_MAX_PATH], szDumpFileName[_MAX_PATH];
	_tcscpy_s(szInputPath, argv[1]);
	_tcscpy_s(szOutputPath, argv[2]);
	_stprintf_s(szFileMask, _MAX_PATH, _T("%s\\*.TIF"), szInputPath);
	_tfinddata_t FindData;
	intptr_t handleI, handleJ;
	for (handleI = handleJ = _tfindfirst(szFileMask, &FindData); handleI != -1; handleI = _tfindnext(handleJ, &FindData))
	{
		if ((FindData.attrib & _A_SUBDIR) != 0)
			continue;

		_stprintf_s(szFileName, _MAX_PATH, _T("%s\\%s"), szInputPath, FindData.name);
		KImage *pImage = new KImage(szFileName);
		if (!pImage->IsValid() || pImage->GetBPP() != 8)
		{
			delete pImage;
			continue;
		}
		int intWidth = pImage->GetWidth();
		int intHeight = pImage->GetHeight();

		for (int intFilterType = 0; intFilterType < NUMBER_OF_FILTERS; intFilterType++)
		{
			for (double dblScale = 1.5; dblScale <= 5.0; dblScale += 0.5)
			{
				int intNewWidth = int(intWidth / dblScale + 0.5);
				int intNewHeight = int(intHeight / dblScale + 0.5);
				KImage downsampledImage(intNewWidth, intNewHeight, 8), dumpImage(intWidth, intHeight, 8);
				
				Resample(pImage, &downsampledImage, intFilterType);
				_stprintf_s(szDumpFileName, _MAX_PATH, _T("%s\\%s_FLT=%d_SCL=%.2lf_DOWNSAMPLED.TIF"), szOutputPath, FindData.name, intFilterType, dblScale);
				downsampledImage.SaveAs(szDumpFileName);

				Resample(&downsampledImage, &dumpImage, intFilterType);
				long double dblMSE = MSE(pImage, &dumpImage);
				_stprintf_s(szDumpFileName, _MAX_PATH, _T("%s\\%s_FLT=%d_SCL=%.2lf_MSE=%.3lf.TIF"), szOutputPath, FindData.name, intFilterType, dblScale, dblMSE);
				dumpImage.SaveAs(szDumpFileName);

				dblMSESum += dblMSE;
				intStatDataCount++;
			}
		}
		delete pImage;
	}
	_findclose(handleJ);

	long double dblMSEAvg = dblMSESum / intStatDataCount;
	long double dblPSNRAvg = PSNR(dblMSEAvg);
	_tprintf(_T("After %d stats: AVG_MSE = %.3lf, AVG_PSNR = %.3lfdB\n"), intStatDataCount, dblMSEAvg, dblPSNRAvg);

	getchar();
	return 0;
}

