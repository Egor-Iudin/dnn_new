#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <boost/algorithm/string.hpp>
#include "iostream"
#include "stdio.h"
#include "TAxis.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TClass.h"
#include "TDirectory.h"
#include "TStyle.h"
#include "TText.h"
#include "TBox.h"
#include "TCanvasImp.h"
#include "TDialogCanvas.h"
#include "TGuiFactory.h"
#include "TEnv.h"
#include "TError.h"
#include "TGraph.h"
#include "TFile.h"
#include "TMultiGraph.h"
#include "TTree.h"
#include "TRandom.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TLegend.h"

class PrintError
{

public:
	PrintError(std::vector<std::string> Files);
	void PrintGraph1(int n, float *x, float *y, const char *fileName, const char *graphName, bool png);
	// void PrintGraphError(int n, float* x,  float* y,int nE,  float* xE,  float* yE, char *fileName, char *graphName);
	void PrintGraphError1(int n, float *x, float *y, int nE, float *xE, float *yE, const char *fileName, const char *graphName, bool png);
	void PrintEnsemble(int ensemblesize, int n, float *x, float **y, const char *fileName, const char *graphName, const char *titleName, std::vector<std::string> files);
	void CreateTree1(int n, float *y, float *err, float *exam, float *roc, float *rocExam, const char *fileName, const char *graphName);
	std::tuple<float *, float *, float *, float *, float *, float *, int> ReadFile(std::string fileName);

	std::vector<std::string> parcestr(std::string str);

	std::string rFileName;
	std::string rGraphName;
};
