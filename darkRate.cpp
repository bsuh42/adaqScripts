#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void darkRate(TString file, const Double_t trigger, Int_t roiStart = 0, Int_t roiEnd = 0)
{
  //Count the number of pulses in each waveform and fit (works if there are multiple pulses in a single waveform
  //Count the number of total pulses and returns a rate (works if there is less than one pulse per waveform)
  //Double_t trigger: look for a rising pulse that goes over this value
  //Int_t roi: region of interest. If both set to 0, use full waveform
  
  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  Double_t polarity = 0;
  Double_t baseline = 0;
  vector<UShort_t> *waveform = 0;
  vector<UShort_t> readWaveform;
  Double_t temp = 0;
  Int_t counter = 0; //total count
  Int_t stormCount = 0; //count per waveform
  Bool_t pulsing = 0;

  const Int_t countsLower = 0;
  const Int_t countsUpper = 600;
  const Int_t countsLength = countsUpper - countsLower;
  TH1D* counts = new TH1D("", "", countsLength, countsLower, countsUpper);

  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("polarity", &polarity);
  T->SetBranchAddress("baseline", &baseline);
  T->GetEntry(0);

  const Int_t numberEntries = T->GetEntries();
  readWaveform = *waveform;
  if (roiStart == 0 && roiEnd == 0)
  {
    roiStart = 0;
    roiEnd = readWaveform.size();
  }
  const Int_t roiLength = roiEnd - roiStart;

  Double_t upTime = roiLength*4.0*numberEntries; //how long data was taken for in nanoseconds
  upTime /= 1e9; //convert to seconds

  for (Int_t eventNumber = 0; eventNumber < numberEntries; eventNumber++)
  {
    stormCount = 0;
    if (eventNumber % 1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    readWaveform = *waveform;

    for (Int_t i = roiStart; i < roiEnd; i++)
    {
      pulsing = 0;
      temp = (readWaveform[i]-baseline*1.0)*polarity*1.0;
      if (!pulsing)
      {
	if (temp > trigger && temp > (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
	{
          stormCount += 1;
	  pulsing = 1;
	}
      }
      if (pulsing)
      {
        if (temp < trigger && temp < (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
	{
	  pulsing = 0;
	}
      }
    }
    counter += stormCount;
    counts->Fill(stormCount);
  }
  TF1 *countFit = new TF1("countFit", "gaus", countsLower, countsUpper);

  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  counts->Fit(countFit);
  counts->Draw("hist");

  Double_t darkRateTotal = (counter*1.0)/upTime;
  Double_t darkRateFit = (countFit->GetParameter(1))*numberEntries/upTime;

  printf("Dark rate from counting total = %f Hz\n", darkRateTotal);
  printf("Dark rate from fit = %f Hz\n", darkRateFit);
}
