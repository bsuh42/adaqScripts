#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void darkRateOverTime (TString file, Int_t roiStart = 0, Int_t roiEnd = 0)
{
  //Count the number of total pulses and returns a rate
  //Make a plot of that value as a function of trigger
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
  Double_t counter = 0;
  

  const Int_t numberThreshold = 400;
  const Int_t lowerThreshold = 5;
  const Int_t threshStep = 5;

  Double_t thresholds[numberThreshold];
  Double_t counts[numberThreshold];
  Bool_t pulsing[numberThreshold];
  Double_t stormCount[numberThreshold];
  for (Int_t i = 0; i < numberThreshold; i ++)
  {
    thresholds[i] = lowerThreshold + i*threshStep;
  }
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
  const Int_t roiLength = roiEnd-roiStart;

  Double_t upTime = roiLength*4.0*numberEntries;
  upTime /= 1e9;
  printf("upTime = %f\n", upTime);
  for (Int_t eventNumber = 0; eventNumber < numberEntries; eventNumber++)
  {
    for (Int_t i = 0; i < numberThreshold; i++)
    {
      stormCount[i] = 0;
    }
    if (eventNumber % 1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    readWaveform = *waveform;

    for (Int_t i = roiStart; i <= roiEnd; i++)
    {
      temp = (readWaveform[i]-baseline*1.0)*polarity*1.0;
      for (Int_t j = 0; j < numberThreshold; j++)
      {
        pulsing[j] = 0;
	if (!pulsing[j])
	{
	  if (temp > thresholds[j] && temp > (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
	  {
	    stormCount[j] += 1.0;
	    pulsing[j] = 1;
	  }
	}
	if (pulsing[j])
	{
	  if (temp < thresholds[j] && temp < (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
	  {
	    pulsing[j] = 0;
	  }
	}
      }
    }
    for (Int_t i = 0; i < numberThreshold; i++)
    {
      //printf("stormCount = %d\n", stormCount[i]);
      counts[i] += stormCount[i]*1.0;
    }
  }

  for (Int_t i = 0; i < numberThreshold; i++)
  {
    //printf("counts = %f\n", counts[i]);
    counts[i] = (counts[i]*1.0)/upTime;
  }
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  TGraph *gr1 = new TGraph(numberThreshold, thresholds, counts);
  
  gr1->GetXaxis()->SetTitle("Tresholds");
  gr1->GetYaxis()->SetTitle("Rate(Hz)");
  gr1->Draw("A*");
}
