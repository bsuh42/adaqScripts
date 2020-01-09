#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void darkRate(TString file, const Double_t trigger, const Double_t holdOff = 0, Int_t roiStart = 0, Int_t roiEnd = 0)
{
  //Count the number of pulses in each waveform and fit (works if there are multiple pulses in a single waveform
  //Count the number of total pulses and returns a rate (works if there is less than one pulse per waveform)
  //Double_t trigger: look for a rising pulse that goes over this value
  //Double_t holdOff: after the pulse falls, hold off for a certain amount of time before looking for a new rising pulse
  //roi: region of interest. If both set to 0, use full waveform

  //Load file and tree
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
  Int_t pulseEnd = 0; //if using holdoff, save where the pulse ends
  const Double_t binSize = 4.0; //length of time bin in ns

  //Create histogram for multiple pulses per waveform
  const Int_t countsLower = 0;
  const Int_t countsUpper = 600;
  const Int_t countsLength = countsUpper-countsLower;
  TH1D* counts = new TH1D("", "", countsLength, countsLower, countsUpper);

  //Grab branches
  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("polarity", &polarity);
  T->SetBranchAddress("baseline", &baseline);
  T->GetEntry(0);
  readWaveform = *waveform;

  //Set region of interest
  if (roiStart == 0 && roiEnd == 0)
  {
    roiStart = 0;
    roiEnd = readWaveform.size();
  }
  const Int_t roiLength = roiEnd-roiStart;

  //Set time length
  const Int_t numberEntries = T->GetEntries();
  Double_t upTime = roiLength*binSize*numberEntries;
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
    pulsing = 0;
    pulseEnd = -1e9;
    for (Int_t i = roiStart; i < roiEnd; i++)
    {
      temp = (readWaveform[i]-baseline*1.0)*polarity*1.0;
      if (!pulsing)
      {
        if (temp > trigger && temp > (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
	{
	  if (i >= (pulseEnd+holdOff))
	  {
	    stormCount += 1;
	    pulsing = 1;
	  }
	}
      }
      if (pulsing)
      {
        if (temp < trigger && temp < (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
	{
	  pulsing = 0;
	  pulseEnd = i;
	}
      }
    }
    counter += stormCount;
    counts->Fill(stormCount);
  }

  //Fit multiple pulse plot
  TF1 *countFit = new TF1("countFit", "gaus", countsLower, countsUpper);

  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  counts->Draw("hist");
  counts->Fit(countFit);
  countFit->Draw("same");

  //Calculate dark rate
  Double_t darkRateTotal = (counter*1.0)/upTime;
  Double_t darkRateFit = (countFit->GetParameter(1))*numberEntries/upTime;

  printf("For an uptime of %f sec\n", upTime);
  printf("Dark rate from counting total = %f Hz\n", darkRateTotal);
  printf("Dark rate from fit = %f Hz\n", darkRateFit);
}
