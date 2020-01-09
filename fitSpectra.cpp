#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void fitSpectra(TString file, Double_t lowerBound, Double_t upperBound, Double_t rebinFactor = 1)
{
  //Take a spectra and fit peak for spe. Can also rebin hist
  TFile *F = new TFile(file);
  const Int_t lowerHist = -1000;
  const Int_t upperHist = 3000;
  const Int_t histLength = upperHist-lowerHist;
  TH1D *h1 = new TH1D("", "", histLength, lowerHist, upperHist);
  h1 = (TH1D*)F->Get("hist");
  h1->Rebin(rebinFactor);

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
  combinedFit->Draw("same");
  c1->Update();
}
