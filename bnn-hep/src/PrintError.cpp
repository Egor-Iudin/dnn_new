#include <PrintError.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
//#include "TCanvas.h"
//#include "CTGraph.h"
// gcc                                -L/afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.10/x86_64-slc5-gcc47-opt/root/lib -lCore -lCint -lRIO -lNet -lHist -lGraf
//-lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic       printError.cxx libCROOT.a
// libCROOT.a                   -lm -o printError

// Start createNetwork  bnn_sm7_151122_190911_lLP.net  Number 1
// Start internal Iteration   0
// StartHybryb
//  Error =   0.500002

// Error =   0.404285
// Error =   0.405561
// Error =   0.405300
// StopHybryb
// Start internal Iteration   1
/// StartHybryb
// strcat(ErrFileName,iterNumber);

// void *PrintGraph(int , const float* , const float*,char *, char *);
// void *PrintGraphError(int , const float* , const float*, int , const float* , const float* ,char *, char *);
// void *PrintGraph(int n, const float* x, const float* y,char *filename, char *graphName);

PrintError::PrintError(std::vector<std::string> Files)
{
	float **EnsembleErrorList;
	float **EnsembleExamList;
	float **EnsembleEnergyList;
	float **EnsembleROCList;
	float **EnsembleROCExamList;

	float *EnsembleNumber;
	float *SingList;
	float *ErrorList;
	float *ExamList;
	float *ROCList;
	float *ROCExamList;
	float *EnsembleNumberList;
	std::tuple<float *, float *, float *, float *, float *, float *, int> distrvalues;
	std::cout << "PrintError start " << Files.size() << std::endl;
	ErrorList = (float *)malloc(3000 * sizeof(double));
	ROCList = (float *)malloc(3000 * sizeof(double));
	ROCExamList = (float *)malloc(3000 * sizeof(double));
	ExamList = (float *)malloc(3000 * sizeof(double));
	SingList = (float *)malloc(3000 * sizeof(double));
	EnsembleNumber = (float *)malloc(3000 * sizeof(double));
	EnsembleErrorList = (float **)malloc(3000 * sizeof(double *));
	EnsembleExamList = (float **)malloc(3000 * sizeof(double *));
	EnsembleEnergyList = (float **)malloc(3000 * sizeof(double *));
	EnsembleNumberList = (float *)malloc(3000 * sizeof(int));
	EnsembleROCList = (float **)malloc(3000 * sizeof(double *));
	EnsembleROCExamList = (float **)malloc(3000 * sizeof(double *));

	int inters_count = 0;
	// NumbersListLong = (float*) malloc ( 300 * sizeof(double));
	std::cout << "PrintError start " << Files.size() << std::endl;
	for (int i = 0; i < Files.size(); i++)
	{

		std::string sss(Files[i]);
		std::string dir((sss + "_png").c_str());
		std::cout << "FileName=" << (sss + ".log").c_str() << std::endl;
		// try
		//{
		system(("mkdir " + dir).c_str());
		distrvalues = ReadFile(sss + ".log");
		EnsembleEnergyList[i] = std::get<0>(distrvalues);
		EnsembleErrorList[i] = std::get<1>(distrvalues);
		EnsembleExamList[i] = std::get<2>(distrvalues);
		EnsembleROCList[i] = std::get<3>(distrvalues);
		EnsembleROCExamList[i] = std::get<4>(distrvalues);

		EnsembleNumberList = std::get<5>(distrvalues);
		EnsembleNumber[i] = i;

		ErrorList = EnsembleErrorList[i];
		ExamList = EnsembleExamList[i]; //(float*) malloc( 300 * sizeof(double));
		SingList = EnsembleEnergyList[i];
		ROCList = EnsembleROCList[i];
		ROCExamList = EnsembleROCExamList[i];
		SingList = EnsembleEnergyList[i];
		//}
		// catch
		//{

		//}
		std::cout << SingList[i] << std::endl;
		// getchar();
		std::cout << ("MMMMMMMMMMMMM" + sss).c_str() << std::endl;
		system(("cp *.png " + dir + "/").c_str());
		system("rm *.png ");
	}
	inters_count = std::get<6>(distrvalues);

	PrintEnsemble(Files.size(), inters_count, EnsembleNumberList, EnsembleEnergyList, "ensembleEnergy.root", "ensembleEnergy", "ensembleEnergy", Files);
	PrintEnsemble(Files.size(), inters_count, EnsembleNumberList, EnsembleErrorList, "ensembleError.root", "ensembleError", "ensembleError", Files);
	PrintEnsemble(Files.size(), inters_count, EnsembleNumberList, EnsembleExamList, "ensembleExam.root", "ensembleExam", "ensembleExam", Files);
	PrintEnsemble(Files.size(), inters_count, EnsembleNumberList, EnsembleROCList, "ensembleROC.root", "ensembleROC", "ensembleROC", Files);
	PrintEnsemble(Files.size(), inters_count, EnsembleNumberList, EnsembleROCExamList, "ensembleROCExam.root", "ensembleROCExam", "ensembleROCExam", Files);

	// PrintEnsemble(int ensemblesize,int n,  float** x, float* y,const char *fileName, const char *graphName,const char *titleName);
	CreateTree1(Files.size(), SingList, ErrorList, ExamList, ROCList, ROCExamList, "EnergyDistr.root", "Energy Distribution ");
}

std::vector<std::string> PrintError::parcestr(std::string str)
{
	// const char *str1 =str.c_str();
	std::vector<std::string> ret;
	const char *pch;
	// printf ("Splitting string \"%s\" into tokens:\n",(char *)str.c_str());
	pch = strtok((char *)str.c_str(), " ,.-");
	while (pch != NULL)
	{
		// printf ("%s\n",pch);
		ret.push_back(pch);
		pch = strtok(NULL, " ,");
	}
	return ret;
}

std::tuple<float *, float *, float *, float *, float *, float *, int> PrintError::ReadFile(std::string fileName1)
{

	float *ROCListLong;
	float *ROCNumbersListLong;
	ROCListLong = (float *)malloc(3000 * sizeof(double));
	ROCNumbersListLong = (float *)malloc(3000 * sizeof(double));

	float *ROCList;
	float *ROCNumbersList;
	ROCList = (float *)malloc(3000 * sizeof(double));
	ROCNumbersList = (float *)malloc(3000 * sizeof(double));

	float *ROCListAll;
	float *ROCNumbersListAll;
	ROCListAll = (float *)malloc(20000 * sizeof(double));
	ROCNumbersListAll = (float *)malloc(20000 * sizeof(double));

	float *ROCExamListLong;
	float *ROCExamNumbersListLong;
	ROCExamListLong = (float *)malloc(3000 * sizeof(double));
	ROCExamNumbersListLong = (float *)malloc(3000 * sizeof(double));

	float *ROCExamList;
	float *ROCExamNumbersList;
	ROCExamList = (float *)malloc(3000 * sizeof(double));
	ROCExamNumbersList = (float *)malloc(3000 * sizeof(double));

	float *ROCExamListAll;
	float *ROCExamNumbersListAll;
	ROCExamListAll = (float *)malloc(20000 * sizeof(double));
	ROCExamNumbersListAll = (float *)malloc(20000 * sizeof(double));

	float *ErrorListLong;
	float *NumbersListLong;
	ErrorListLong = (float *)malloc(3000 * sizeof(double));
	NumbersListLong = (float *)malloc(3000 * sizeof(double));

	float *SingListLong;
	float *SingNumberListLong;
	SingListLong = (float *)malloc(3000 * sizeof(double));
	SingNumberListLong = (float *)malloc(3000 * sizeof(double));

	float *ErrorList;
	float *NumbersList;
	ErrorList = (float *)malloc(3000 * sizeof(double));
	NumbersList = (float *)malloc(3000 * sizeof(double));

	float *SingList;
	float *SingNumberList;
	SingList = (float *)malloc(3000 * sizeof(double));
	SingNumberList = (float *)malloc(3000 * sizeof(double));

	float *SingListAll;
	float *SingNumberListAll;
	SingListAll = (float *)malloc(20000 * sizeof(double));
	SingNumberListAll = (float *)malloc(20000 * sizeof(double));

	float *ErrorListExam;
	float *NumbersListExam;
	ErrorListExam = (float *)malloc(10000 * sizeof(double));
	NumbersListExam = (float *)malloc(10000 * sizeof(double));

	float *ErrorListExamLong;
	float *NumbersListExamLong;
	ErrorListExamLong = (float *)malloc(10000 * sizeof(double));
	NumbersListExamLong = (float *)malloc(10000 * sizeof(double));

	float *ErrorListExamAll;
	float *NumbersListExamAll;
	ErrorListExamAll = (float *)malloc(20000 * sizeof(double));
	NumbersListExamAll = (float *)malloc(20000 * sizeof(double));

	float *ErrorListAll;
	float *NumbersListAll;
	ErrorListAll = (float *)malloc(20000 * sizeof(double));
	NumbersListAll = (float *)malloc(20000 * sizeof(double));

	std::vector<std::string> parsedstr;
	std::string STRING;
	std::ifstream infile;
	std::string firststep("Start createNetwork");
	std::string secondstep("StartHybryb");
	std::string thirdstep("Error");
	std::string stopstep("Stop createNetwork");
	std::string graphName;
	std::string fileName;
	infile.open(fileName1);
	// int a=0;
	std::string previousLine = "";
	// float val;

	int internalNumber = 0;
	int iterNumber = 0;
	int AllNumber = 0;

	while (getline(infile, STRING)) // To get you all the lines.
	{
		if (STRING.find("End File") != std::string::npos)
		{
			std::cout << "End File" << std::endl;
			// getchar();
			PrintGraphError1(iterNumber, NumbersListLong, ErrorListLong, iterNumber, NumbersListExamLong, ErrorListExamLong, rFileName.c_str(), ("Long_Errors_" + rGraphName).c_str(), true);

			PrintGraph1(iterNumber, SingNumberListLong, SingListLong, rFileName.c_str(), ("Long_Energy_" + rGraphName).c_str(), true);

			PrintGraphError1(AllNumber, NumbersListAll, ErrorListAll, AllNumber, NumbersListExamAll, ErrorListExamAll, rFileName.c_str(), ("All_Errors_" + rGraphName).c_str(), true);

			PrintGraph1(AllNumber, SingNumberListAll, SingListAll, rFileName.c_str(), ("All_Energy_" + rGraphName).c_str(), true);
			// getchar();
			// iterNumber=0;

			PrintGraphError1(AllNumber, NumbersListAll, ROCListAll, AllNumber, NumbersListExamAll, ROCExamListAll, rFileName.c_str(), ("All_ROCs_" + rGraphName).c_str(), true);

			PrintGraphError1(iterNumber, NumbersListLong, ROCListLong, iterNumber, NumbersListExamLong, ROCExamListLong, rFileName.c_str(), ("Long_ROCs_" + rGraphName).c_str(), true);

			break;
		}
		if (STRING.find(firststep) != std::string::npos)
		{
			std::cout << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << std::endl;
			// getchar();

			parsedstr = parcestr(STRING);

			// string nnn (parsedstr[2]);

			parsedstr[2].erase(parsedstr[2].end() - 8, parsedstr[2].end());
			// string nameRoot = name.c_str()+".root";
			rGraphName = (parsedstr[2] + "");
			rFileName = (parsedstr[2] + ".root");
			std::cout << "  sssss " << std::endl;
			std::cout << "  sssss " << rGraphName.c_str() << "  " << rFileName.c_str() << std::endl;
			// getchar();
			while (getline(infile, STRING)) // To get you all the lines.
			{
				std::cout << STRING.c_str() << std::endl;
				// getchar();
				if (STRING.find(stopstep) != std::string::npos)
				{
					// getchar();

					std::cout << "Stop!!!!!!!!!!!" << std::endl;
					break;
				}

				if (STRING.find(secondstep) != std::string::npos)
				{

					internalNumber = 0;
					while (getline(infile, STRING)) // To get you all the lines.
					{
						// std::cout<<"CCCCCCCCCCCCCCCCCCCCCCCCCCc"<<std::endl;
						if (STRING.find("ErrE") != std::string::npos)
						{

							parsedstr = parcestr(STRING);
							// cout<<STRING<<endl; // Prints our STRING.
							float val;
							// fff.erase (fff.end()-4, fff.end());
							// cout<<fff<<endl; // Prints our STRING.
							// std::cout<<"parsestr = "<<parsedstr[2].c_str()<<std::endl;
							val = atof(parsedstr[2].c_str());
							// std::cout<<"atof Error = "<<val<<std::endl;
							ErrorListExam[internalNumber] = val;
							// ErrorListExam[internalNumber]=1;
							NumbersListExam[internalNumber] = internalNumber;
							ErrorListExamAll[AllNumber] = val;
							// ErrorListExam[internalNumber]=1;
							NumbersListExamAll[AllNumber] = AllNumber;

							// getchar();
						}
						if (STRING.find("Error") != std::string::npos)
						{

							parsedstr = parcestr(STRING);

							float val;
							val = atof(parsedstr[2].c_str());
							// std::cout<<"atof Exam = "<<val<<std::endl;
							ErrorList[internalNumber] = val;
							// ErrorList[internalNumber]=1;
							NumbersList[internalNumber] = internalNumber;
							// internalNumber++;

							ErrorListAll[AllNumber] = val;
							// ErrorList[internalNumber]=1;
							NumbersListAll[AllNumber] = AllNumber;
							// internalNumber++;

							// getchar();
						}

						if (STRING.find("Sign") != std::string::npos)
						{

							parsedstr = parcestr(STRING);
							// cout<<STRING<<endl; // Prints our STRING.
							float val;
							val = atof(parsedstr[2].c_str());
							SingList[internalNumber] = val;
							SingNumberList[internalNumber] = internalNumber;
							SingListAll[AllNumber] = val;
							SingNumberListAll[AllNumber] = AllNumber;

							// getchar();
						}

						if (STRING.find("Roc_auc") != std::string::npos)
						{

							parsedstr = parcestr(STRING);
							// cout<<STRING<<endl; // Prints our STRING.
							float val;
							// fff.erase (fff.end()-4, fff.end());
							// cout<<fff<<endl; // Prints our STRING.
							// std::cout<<"parsestr = "<<parsedstr[2].c_str()<<std::endl;
							val = atof(parsedstr[2].c_str());
							// std::cout<<"atof Error = "<<val<<std::endl;
							ROCList[internalNumber] = val;
							// ErrorListExam[internalNumber]=1;
							ROCNumbersList[internalNumber] = internalNumber;
							ROCListAll[AllNumber] = val;
							// ErrorListExam[internalNumber]=1;
							ROCNumbersListAll[AllNumber] = AllNumber;

							// getchar();
						}
						if (STRING.find("Roc_Exam") != std::string::npos)
						{

							parsedstr = parcestr(STRING);
							// cout<<STRING<<endl; // Prints our STRING.
							float val;
							// fff.erase (fff.end()-4, fff.end());
							// cout<<fff<<endl; // Prints our STRING.
							// std::cout<<"parsestr = "<<parsedstr[2].c_str()<<std::endl;
							val = atof(parsedstr[2].c_str());
							// std::cout<<"atof Error = "<<val<<std::endl;
							ROCExamList[internalNumber] = val;
							// ErrorListExam[internalNumber]=1;
							ROCExamNumbersList[internalNumber] = internalNumber;
							ROCExamListAll[AllNumber] = val;
							// ErrorListExam[internalNumber]=1;
							ROCExamNumbersListAll[AllNumber] = AllNumber;
							internalNumber++;
							AllNumber++;

							// getchar();
						}

						if (STRING.find("StopHybryb") != std::string::npos)
						{

							PrintGraphError1(internalNumber, NumbersList, ErrorList, internalNumber, NumbersListExam, ErrorListExam, rFileName.c_str(), ("Errors_" + rGraphName).c_str(), false);
							PrintGraphError1(internalNumber, NumbersList, ROCList, internalNumber, NumbersListExam, ROCExamList, rFileName.c_str(), ("ROCs_" + rGraphName).c_str(), false);

							PrintGraph1(internalNumber, SingNumberList, SingList, rFileName.c_str(), ("Energy_" + rGraphName).c_str(), false);

							ErrorListExamLong[iterNumber] = ErrorListExam[internalNumber - 1];

							NumbersListExamLong[iterNumber] = iterNumber;

							SingListLong[iterNumber] = SingList[internalNumber - 1];

							SingNumberListLong[iterNumber] = iterNumber;

							ErrorListLong[iterNumber] = ErrorList[internalNumber - 1];
							;
							// ErrorList[internalNumber]=1;
							NumbersListLong[iterNumber] = iterNumber;

							ROCListLong[iterNumber] = ROCList[internalNumber - 1];
							;
							// ErrorList[internalNumber]=1;
							ROCNumbersListLong[iterNumber] = iterNumber;

							ROCExamListLong[iterNumber] = ROCExamList[internalNumber - 1];
							;
							// ErrorList[internalNumber]=1;
							ROCExamNumbersListLong[iterNumber] = iterNumber;

							iterNumber++;

							// std::cout<<" bbbb  "<<rFileName<<"  "<<rGraphName<<std::endl;
							// std::cout<< "mmm "<<stopstep.c_str()<<std::endl;
							// getchar();
							// delete fname;
							// delete gname;
							// std::cout<<"Break11111111111"<<std::endl;
							break;
						}
					}
				}
			}
		}
	}
	// getchar();
	infile.close();
	// system ("pause");
	// delete  ErrorList;
	// delete NumbersList;
	// delete SingList;
	// delete SingNumberList;
	// delete ErrorListExam;
	// delete NumbersListExam ;
	// std::cout<<"End First step"<<std::endl;
	// delete parsedstr;
	// delete STRING;
	// delete infile;
	/// delete firststep ;
	// delete secondstep ;
	// delete thirdstep ;
	// delete stopstep ;
	// delete graphName;
	// delete fileName;
	// return SingListLong[iterNumber-1];
	return std::make_tuple(SingListLong, ErrorListLong, ErrorListExamLong, ROCListLong, ROCExamListLong, NumbersListLong, iterNumber - 1);
	// getchar();
}

void PrintError::PrintEnsemble(int ensemblesize, int n, float *x, float **y, const char *fileName, const char *graphName, const char *titleName, std::vector<std::string> files)
{

	std::ostringstream pngname;

	const char *fname = fileName;  //   fileName;
	const char *gname = graphName; // graphName;
	const char *tName = titleName;

	// printf("1  xxxxx1111  %s \n",fname);
	// getchar();
	TFile f(fname, "UPDATE");
	TCanvas *c2 = new TCanvas("ensemble", "c2", 600, 400);

	// printf("1  xxxxx2222  %s \n",fname);
	// getchar();
	TMultiGraph *mg = new TMultiGraph();

	// printf("2 \n");

	mg->SetTitle(tName);
	mg->SetName(gname);
	// printf("3 \n");
	/// TLegend leg(.1,.7,.3,.9,"Test Multi");
	// leg.SetFillColor(0);
	for (int ensemble = 0; ensemble < ensemblesize; ensemble++)
	{
		TGraph *gr = new TGraph(n, x, y[ensemble]);
		gr->SetName(gname);
		gr->SetLineColor(2 + ensemble);
		gr->SetTitle(files[ensemble].c_str());
		// gr->SetDrawOption("AP");
		// printf("4 \n");

		gr->SetLineWidth(2);
		gr->SetMarkerColor(4 + ensemble);
		gr->SetMarkerStyle(21);

		(gr->GetXaxis())->SetTitle("Iterations");
		// leg.AddEntry(gr,"Exp. Points");
		mg->Add(gr);

		// printf("5 \n");
	}

	// printf("6 \n");

	//  getchar();
	mg->Draw("AC*");
	mg->GetXaxis()->SetTitle("Iterations");
	mg->GetYaxis()->SetTitle(tName);
	c2->BuildLegend();
	// mg->Write();
	c2->Write();
	pngname.str("");
	pngname << gname << ".png";
	c2->Print(pngname.str().c_str());

	f.Write();
	f.Close();

	// graph.SetFillColor(0);
	// leg.AddEntry(&gr1,"Exp. Points");
	// leg.AddEntry(&gr2,"Th. Law");
	// leg.Draw("Same");
	// printf("7 \n");
}

void PrintError::PrintGraph1(int n, float *x, float *y, const char *fileName, const char *graphName, bool png)
{
	int n1 = n;
	std::ostringstream pngname;
	const char *fname = fileName;  //   fileName;
	const char *gname = graphName; // graphName;
	// printf("1  xxxxx1111  %s \n",fname);
	// getchar();
	TFile f(fname, "UPDATE");
	TCanvas *c2 = new TCanvas(graphName, "c2", 600, 400);

	// printf("1  xxxxx2222  %s \n",fname);
	// getchar();
	TMultiGraph *mg = new TMultiGraph();

	// printf("2 \n");

	mg->SetTitle("Energy");
	mg->SetName(gname);
	// printf("3 \n");

	TGraph *gr = new TGraph(n, x, y);
	gr->SetName(gname);
	gr->SetLineColor(2);
	// printf("4 \n");

	gr->SetTitle(graphName);
	gr->SetLineWidth(4);
	gr->SetMarkerColor(4);
	gr->SetMarkerStyle(21);

	(gr->GetXaxis())->SetTitle("Iterations");

	mg->Add(gr);

	// printf("5 \n");

	// printf("6 \n");

	//  getchar();
	mg->Draw("AC*");
	mg->GetXaxis()->SetTitle("Iterations");
	mg->GetYaxis()->SetTitle("Energy");
	// mg->Write();
	c2->BuildLegend();
	// mg->Write();
	c2->Write();
	if (png == true)
	{
		pngname.str("");
		pngname << gname << ".png";
		c2->Print(pngname.str().c_str());
	}
	f.Write();
	f.Close();

	// printf("7 \n");
}

void PrintError::CreateTree1(int n, float *y, float *err, float *exam, float *roc, float *rocExam, const char *fileName, const char *graphName)
{
	TFile f(fileName, "recreate");
	TTree t1("t1", "a simple Tree with simple variables");
	Float_t px, py, pz;
	Double_t energy;
	Float_t error, examError, Roc, RocExam;
	Double_t random;
	Int_t ev;
	t1.Branch("energy", &energy);
	t1.Branch("Error", &error);
	t1.Branch("examError", &examError);
	t1.Branch("ROC", &Roc);
	t1.Branch("ROCExam", &RocExam);
	// t1.Branch("random",&random,"random/D");
	t1.Branch("ev", &ev);

	// fill the tree
	for (Int_t i = 0; i < n; i++)
	{
		gRandom->Rannor(px, py);
		// pz = px*px + py*py;
		energy = y[i];
		ev = i;
		error = err[i];
		examError = exam[i];
		Roc = roc[i];
		RocExam = rocExam[i];
		t1.Fill();
	}
	t1.Write();
}

void PrintError::PrintGraphError1(int n, float *x, float *y, int nE, float *xE, float *yE, const char *fileName, const char *graphName, bool png)
{
	// const char *fname =   fileName;
	// const char *gname =   graphName;
	std::ostringstream pngname;
	const char *fname = fileName;  //   fileName;
	const char *gname = graphName; // graphName;

	// printf("1  vvvv33333  %s \n",fname);
	// ggetchar();
	int n1 = n;
	TFile f(fname, "UPDATE");
	TCanvas *c2 = new TCanvas(graphName, "c2", 600, 400);
	// TGraph *g[3];
	// Double_t x1[10] = {0,1,2,3,4,5,6,7,8,9};
	// Double_t y1[10] = {1,2,3,4,5,5,4,3,2,1};
	// printf("1  vvvv4444  %s \n",fname);
	// getchar();
	TMultiGraph *mg = new TMultiGraph();
	// for (int i=0; i<3; i++) {
	//    g[i] = new TGraph(10, x1, y1);
	//    g[i]->SetMarkerStyle(20);
	//    g[i]->SetMarkerColor(i+2);
	//    for (int j=0; j<10; j++) y1[j] = y1[j]-1;
	//    mg->Add(g[i]);
	// }
	// printf("2 \n");

	// getchar();

	// Change the axis limits
	// gPad->Modified();
	// mg->GetXaxis()->SetLimits(1.5,7.5);
	// mg->SetMinimum(0.);
	// mg->SetMaximum(10.);
	mg->SetTitle(graphName);
	//(mg->GetXaxis())->SetTitle("Iterations");

	mg->SetName(gname);
	// printf("3 \n");

	// getchar();
	// TCanvas *c5 = new TCanvas("c5","A Simple Graph Example",10,10,10,10);
	// Double_t x1[n1],z[n1], y1[n1];
	// c5->SetFillColor(42);
	// c5->SetGrid();
	// for (int i = 0; i<n; i++)
	// {
	// 	x1[i]=i;
	// 	y1[i]=i;
	// }

	//	getchar();
	TGraph *gr = new TGraph(n, x, y);
	gr->SetName(gname);
	gr->SetLineColor(4);
	// printf("4 \n");

	// getchar();

	gr->SetLineWidth(4);
	gr->SetMarkerColor(4);
	gr->SetMarkerStyle(21);
	gr->SetTitle("train");
	(gr->GetXaxis())->SetTitle("Iterations");
	// gr->GetYaxis()->SetRangeUser(0.6,0.99);
	// gr->GetYaxis()>SetLimits(0.3,0.6);
	mg->Add(gr);

	TGraph *gr1 = new TGraph(nE, xE, yE);
	gr1->SetName(gname);
	gr1->SetLineColor(5);

	gr1->SetLineWidth(4);
	gr1->SetMarkerColor(9);
	gr1->SetMarkerStyle(21);
	gr1->SetTitle("Exam");

	(gr1->GetXaxis())->SetTitle("Iterations");

	mg->Add(gr1);
	// gr->Draw("AC*");
	//  gr->Write();
	// printf("5 \n");

	// getchar();

	// mg->Write();
	// printf("6 \n");

	//  getchar();
	mg->Draw("AC*");
	mg->GetXaxis()->SetTitle("Iterations");
	mg->GetYaxis()->SetTitle("Error");
	// mg->Write();
	c2->BuildLegend();
	// mg->Write();
	c2->Write();

	if (png == true)
	{
		pngname.str("");
		pngname << gname << ".png";
		c2->Print(pngname.str().c_str());
	}

	f.Write();
	f.Close();
	// gr->Print("TestGraph.pdf")
	// TCanvas::Update() draws the frame, after which one can change it
	// c5->Update();
	// c5->GetFrame()->SetFillColor(21);
	// c5->GetFrame()->SetBorderSize(12);
	// c5->Modified();
	// c5->SaveAs("Tets.pdf");
	// printf("7 \n");
}
