/**
 * @author Andrey
 * @version 1.2
 * @date 17.08.2010
 *
 * Interface to use of ROOT TFile in pure C application. Just several functions are implemented now.
 */
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
//#include "TCanvas.h"
//#include "CTGraph.h"

void *C_TGraph(int n, const float *x, const float *y)

{
   return (void *)new TGraph(TGraph(n, x, y));
}

void *C_Draw(void *Graph, char *chopt = "")

{
}

void *C_Write(void *Graph)
{
   ((TGraph *)Graph)->Write();
}

void *C_Print(void *Graph, char *chopt = "")
{
   ((TGraph *)Graph)->Print(chopt);
}

void *C_CreateCanvas(char *name, char *tittle)
{
   return (void *)new TCanvas(name, tittle, 200, 10, 700, 500);
}

void *C_PrintGraphs(void *canvas, char *fileName)
{

   ((TCanvas *)canvas)->Update();
   //((TCanvas *)canvas)->GetFrame()->SetFillColor(21);
   //((TCanvas *)canvas)->GetFrame()->SetBorderSize(12);
   ((TCanvas *)canvas)->Modified();
   ((TCanvas *)canvas)->SaveAs("Tets.pdf");
}

void *PrintGraph(int n, const float *x, const float *y, char *fileName, char *graphName)
{
   int n1 = n;
   TFile f(fileName, "UPDATE");
   TCanvas *c2 = new TCanvas("c2", "c2", 600, 400);
   // TGraph *g[3];
   // Double_t x1[10] = {0,1,2,3,4,5,6,7,8,9};
   // Double_t y1[10] = {1,2,3,4,5,5,4,3,2,1};
   printf("1 \n");
   // getchar();
   TMultiGraph *mg = new TMultiGraph();
   // for (int i=0; i<3; i++) {
   //    g[i] = new TGraph(10, x1, y1);
   //    g[i]->SetMarkerStyle(20);
   //    g[i]->SetMarkerColor(i+2);
   //    for (int j=0; j<10; j++) y1[j] = y1[j]-1;
   //    mg->Add(g[i]);
   // }
   printf("2 \n");

   // getchar();
   // mg->GetXaxis()->SetTitle("E_{#gamma} (GeV)");
   /// mg->GetYaxis()->SetTitle("Coefficients");
   // Change the axis limits
   // gPad->Modified();
   // mg->GetXaxis()->SetLimits(1.5,7.5);
   // mg->SetMinimum(0.);
   // mg->SetMaximum(10.);
   mg->SetTitle("Energy");
   mg->SetName(graphName);
   printf("3 \n");

   //   getchar();
   // TCanvas *c5 = new TCanvas("c5","A Simple Graph Example",10,10,10,10);
   // Double_t x1[n1],z[n1], y1[n1];
   // c5->SetFillColor(42);
   // c5->SetGrid();
   // for (int i = 0; i<n; i++)
   //{
   //  x1[i]=i;
   //  y1[i]=i;
   //}

   //	getchar();
   TGraph *gr = new TGraph(n, x, y);
   gr->SetName(graphName);
   gr->SetLineColor(2);
   printf("4 \n");

   // getchar();

   gr->SetLineWidth(4);
   gr->SetMarkerColor(4);
   gr->SetMarkerStyle(21);
   gr->SetTitle("Iterations ErrorExam");
   (gr->GetXaxis())->SetTitle("Iterations title");
   // gr->GetYaxis()->SetRangeUser(0.6,0.99);
   // gr->GetYaxis()>SetLimits(0.3,0.6);
   mg->Add(gr);
   // TGraph  *gr1 = new TGraph(nE,xE,yE);
   // gr1->SetName(graphName);
   // gr1->SetLineColor(3);

   // gr1->SetLineWidth(4);
   // gr1->SetMarkerColor(9);
   // gr1->SetMarkerStyle(21);
   // gr1->SetTitle("Iterations Error");
   // (gr1->GetXaxis())->SetTitle("Iterations title");
   // gr1->GetYaxis()->SetRangeUser(0.6,0.99);
   // gr1->GetYaxis()>SetLimits(0.3,0.6);
   //	mg->Add(gr1);
   // gr->Draw("AC*");
   // gr->Write();
   printf("5 \n");

   // getchar();
   mg->Write();
   printf("6 \n");

   //  getchar();
   mg->Draw("AC*");
   f.Write();
   f.Close();
   // gr->Print("TestGraph.pdf")
   //  TCanvas::Update() draws the frame, after which one can change it
   // c5->Update();
   // c5->GetFrame()->SetFillColor(21);
   // c5->GetFrame()->SetBorderSize(12);
   // c5->Modified();
   // c5->SaveAs("Tets.pdf");
}

void *CreateTree(int n, const float *x, const float *y, char *fileName, char *graphName)
{
   TFile f("tree1.root", "recreate");
   TTree t1("t1", "a simple Tree with simple variables");
   Float_t px, py, pz;
   Double_t energy;
   Double_t random;
   Int_t ev;
   t1.Branch("energy", &energy);
   // t1.Branch("py",&py,"py/F");
   // t1.Branch("pz",&pz,"pz/F");
   // t1.Branch("random",&random,"random/D");
   t1.Branch("ev", &ev);

   // fill the tree
   for (Int_t i = 0; i < n; i++)
   {
      gRandom->Rannor(px, py);
      // pz = px*px + py*py;
      energy = y[i];
      ev = i;
      t1.Fill();
   }
   t1.Write();
}

void *PrintGraphError(int n, const float *x, const float *y, int nE, const float *xE, const float *yE, char *fileName, char *graphName)
{
   int n1 = n;
   TFile f(fileName, "UPDATE");
   TCanvas *c2 = new TCanvas("c2", "c2", 600, 400);
   // TGraph *g[3];
   // Double_t x1[10] = {0,1,2,3,4,5,6,7,8,9};
   // Double_t y1[10] = {1,2,3,4,5,5,4,3,2,1};
   printf("1 \n");
   // getchar();
   TMultiGraph *mg = new TMultiGraph();
   // for (int i=0; i<3; i++) {
   //    g[i] = new TGraph(10, x1, y1);
   //    g[i]->SetMarkerStyle(20);
   //    g[i]->SetMarkerColor(i+2);
   //    for (int j=0; j<10; j++) y1[j] = y1[j]-1;
   //    mg->Add(g[i]);
   // }
   printf("2 \n");

   // getchar();
   // mg->GetXaxis()->SetTitle("E_{#gamma} (GeV)");
   /// mg->GetYaxis()->SetTitle("Coefficients");
   // Change the axis limits
   // gPad->Modified();
   // mg->GetXaxis()->SetLimits(1.5,7.5);
   // mg->SetMinimum(0.);
   // mg->SetMaximum(10.);
   mg->SetTitle("Errors");
   mg->SetName(graphName);
   printf("3 \n");

   //   getchar();
   // TCanvas *c5 = new TCanvas("c5","A Simple Graph Example",10,10,10,10);
   // Double_t x1[n1],z[n1], y1[n1];
   // c5->SetFillColor(42);
   // c5->SetGrid();
   // for (int i = 0; i<n; i++)
   //{
   //  x1[i]=i;
   //  y1[i]=i;
   //}

   //	getchar();
   TGraph *gr = new TGraph(n, x, y);
   gr->SetName(graphName);
   gr->SetLineColor(4);
   printf("4 \n");

   // getchar();

   gr->SetLineWidth(4);
   gr->SetMarkerColor(4);
   gr->SetMarkerStyle(21);
   gr->SetTitle("Iterations ErrorExam");
   (gr->GetXaxis())->SetTitle("Iterations title");
   // gr->GetYaxis()->SetRangeUser(0.6,0.99);
   // gr->GetYaxis()>SetLimits(0.3,0.6);
   mg->Add(gr);
   TGraph *gr1 = new TGraph(nE, xE, yE);
   gr1->SetName(graphName);
   gr1->SetLineColor(5);

   gr1->SetLineWidth(4);
   gr1->SetMarkerColor(9);
   gr1->SetMarkerStyle(21);
   gr1->SetTitle("Iterations Error");
   (gr1->GetXaxis())->SetTitle("Iterations title");
   // gr1->GetYaxis()->SetRangeUser(0.6,0.99);
   // gr1->GetYaxis()>SetLimits(0.3,0.6);
   mg->Add(gr1);
   gr->Draw("AC*");
   gr->Write();
   printf("5 \n");

   // getchar();
   mg->Write();
   printf("6 \n");

   //  getchar();
   mg->Draw("AC*");
   f.Write();
   f.Close();
   // gr->Print("TestGraph.pdf")
   //  TCanvas::Update() draws the frame, after which one can change it
   // c5->Update();
   // c5->GetFrame()->SetFillColor(21);
   // c5->GetFrame()->SetBorderSize(12);
   // c5->Modified();
   // c5->SaveAs("Tets.pdf");
}
