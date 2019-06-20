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

  TF1 *f1 = new TF1("f1", "gaus", lowerBound, upperBound);
  h1->Fit(f1, "R");

  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  h1->Draw("hist");  
}
