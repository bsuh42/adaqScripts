#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void spectra(TString file, Double_t lowerBound, Double_t upperBound)
{
  //Take a spectra and fit peak. Largely meant for spe
  TFile *F = new TFile(file);
  TH1D *h1 = new TH1D("", "", 4000, -1000, 3000);
  h1= (TH1D*)F->Get("hist");

  TF1 *combinedFit = new TF1("combinedFit", "expo(0)+gaus(3)", lowerBound, upperBound);
  TF1 *f1 = new TF1("f1", "expo", lowerBound, upperBound);
  TF1 *f2 = new TF1("f2", "gaus", lowerBound, upperBound);
  Double_t fitParameters[5];
  h1->Fit(f1, "qR0");
  h1->Fit(f2, "qR0");
  f1->GetParameters(&fitParameters[0]);
  f2->GetParameters(&fitParameters[2]);
  combinedFit->SetParameters(fitParameters);
  h1->Fit(combinedFit, "R");

  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  h1->Draw("hist");  
}
