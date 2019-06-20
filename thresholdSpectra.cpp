#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void thresholdSpectra(TString file, const Double_t trigger)
{
  //Search for a rising pulse that goes above a certain threshold then integrate until it drops below that threshold. Requires waveform

  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  Double_t polarity = 0;
  Double_t baseline = 0;
  Double_t integral = 0;
  vector<UShort_t> *waveform = 0;
  vector<UShort_t> readWaveform;
  Double_t temp = 0;
  Int_t counter = 0;
  Bool_t pulsing = 0;

  const Int_t spectraLower = -1000;
  const Int_t spectraUpper = 2000;
  const Int_t spectraLength = spectraUpper-spectraLower;

  TH1D* spectra = new TH1D("","", spectraLength, spectraLower, spectraUpper);

  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("polarity", &polarity);
  T->SetBranchAddress("baseline", &baseline);
  T->GetEntry(0);

  const Int_t numberEntries = T->GetEntries();
  readWaveform = *waveform;
  Double_t upTime = readWaveform.size()*4.0*numberEntries; //nanoseconds length of run
  upTime /= 1e9; //convert to seconds

  for (Int_t eventNumber = 0; eventNumber< numberEntries; eventNumber++)
  {
    pulsing = 0;
    if (eventNumber % 1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    readWaveform = *waveform;

    for (Int_t i = 0; i < readWaveform.size(); i++)
    {
      integral = 0;
      temp = (readWaveform[i]-baseline*1.0)*polarity*1.0;
      if (!pulsing && temp >= trigger && temp >= (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
      {
	//Only check for rising pulses
	pulsing = 1;
      }
      if (pulsing && temp <= trigger && temp <= (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
      {
        //Stop integrating if below that threshold
	pulsing = 0;
	integral = 0;
      }
      if (pulsing)
      {
        integral += temp;
      }
      spectra->Fill(integral, 1.0/upTime*1.0);
    }
  }
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  spectra->GetXaxis()->SetTitle("Integral");
  spectra->GetYaxis()->SetTitle("Counts/second");
  spectra->Draw("hist");
}
