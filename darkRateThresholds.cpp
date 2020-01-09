#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void darkRateThresholds(TString file, const Int_t numberThreshold, const Int_t lowerThreshold, const Int_t threshStep, Int_t holdOff = 0, Int_t roiStart = 0, Int_t roiEnd = 0)
{
  //Plots dark rate as a function of threshold
  //numberThreshold: how many threshold values to plot
  //lowerThreshold: where to start testing values
  //threshSte: how much to step by

  //Load files
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  Double_t polarity = 0;
  Double_t baseline = 0;
  vector<UShort_t> *waveform = 0;
  vector<UShort_t> readWaveform;
  const Double_t binSize = 4.0; //time length of bin, ns

  Double_t temp = 0;
  Double_t thresholds[numberThreshold];
  Double_t counts[numberThreshold];
  Bool_t pulsing[numberThreshold];
  Double_t stormCount[numberThreshold];
  Int_t pulseEnd[numberThreshold];

  for (Int_t i = 0; i < numberThreshold; i++)
  {
    //Initialize threshold values
    thresholds[i]=lowerThreshold+i*threshStep;
    pulsing[i] = 0;
    stormCount[i] = 0;
  }

  //Branches
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
  upTime /= 1e9;
  
  for (Int_t eventNumber = 0; eventNumber < numberEntries; eventNumber++)
  {
    for (Int_t i = 0; i < numberThreshold; i++)
    {
      stormCount[i]=0;
      pulsing[i]=0;
      pulseEnd[i] = -1e9;
    }
    if (eventNumber % 1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    readWaveform = *waveform;
    for (Int_t i = roiStart; i < roiEnd; i++)
    {
      temp = (readWaveform[i]-baseline*1.0)*polarity*1.0;
      for (Int_t j = 0; j < numberThreshold; j++)
      {
        if (!pulsing[j])
	{
	  if (temp > thresholds[j] && temp > (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
	  {
	    if (i >= (pulseEnd[j]+holdOff))
	    {
	      stormCount[j] += 1;
	      pulsing[j] = 1;
	    }
	  }
	}
	if (pulsing[j])
	{
	  if (temp < thresholds[j] && temp < (readWaveform[i-1]-baseline*1.0)*polarity*1.0)
	  {
	    pulsing[j] = 0;
	    pulseEnd[j] = i;
	  }
	}
      }
    }
    for (Int_t i = 0; i < numberThreshold; i++)
    {
      counts[i] += stormCount[i];
    }
  }

  for (Int_t i = 0; i < numberThreshold; i++)
  {
    stormCount[i] = counts[i];
    counts[i] /= (upTime*1.0);
    printf("For a threshold of %f, saw %f pulses, which converts to %f rate\n", thresholds[i], stormCount[i], counts[i]);
  }

  //Draw
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  TGraph *gr1 = new TGraph(numberThreshold, thresholds, counts);
  gr1->GetXaxis()->SetTitle("Thresholds");
  gr1->GetYaxis()->SetTitle("Rate(Hz)");
  gr1->Draw("A*");
}
