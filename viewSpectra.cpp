#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void viewSpectra(TString file, Int_t windowStart = 0, Int_t windowEnd = 0, Bool_t saveSpectra = 0)
{
  //View spectra from a run
  //Inputs are:
  ////TString file: file where data is stored
  ////Int_t windowStart: if =windowEnd=0, use accumulator 1. Otherwise, integrate between these two values
  ////Bool_t saveSpectra: 0 to not save, 1 to save as a .root file

  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  Double_t baseline = 0;
  Double_t polarity = 0;
  vector<UShort_t> *waveform = 0;
  vector<UShort_t> readWaveform;
  Double_t accumulatorStart = 0;
  Int_t accumulatorStart = 0;
  Int_t accumulatorEnd = 0;

  const Double_t binSize = 4.0; //length of each time bin in nanoseconds
  const Double_t histLower = -1000;
  const Double_t histUpper = 3000;
  const Double_t numberBins = 100; //binSize
  const Double_t histLength = (histUpper-histLower)/numberBins;

  Bool_t usingAccumulator = 0;
  Double_t temp = 0;
  Double_t integral = 0;
  Int_t counter = 0;

  //Get tree values
  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("baseline", &baseline);
  T->SetBranchAddress("polarity", &polarity);
  T->SetBranchAddress("accumulator1", &accumulator);
  T->SetBranchAddress("accumulator1Start", &accumulatorStart);
  T->SetBranchAddress("accumulator1End", &accumulatorEnd);
  T->GetEntry(0);

  //Determine if we're using accumulators or not
  if (windowStart == 0 && windowEnd == 0)
  {
    windowStart = accumulatorStart;
    windowEnd = accumulatorEnd;
    usingAccumulator = 1;
  }
  Int_t windowLength = windowEnd-windowStart+1;

  //Create histogram
  TH1D* hist = new TH1D("", "", histLength, histLower, histUpper);

  //Looking at first waveform
  const Int_t numberEntries = T->GetEntries();
  readWaveform = *waveform;
  Double_t upTime = readWaveform.size()*binSize*numberEntries; //convert run time into nanoseconds
  upTime /= 1e9; //and now into seconds

  for (Int_t eventNumber = 0; eventNumber < numberEntries; eventNumber++)
  {
    temp = 0;
    integral = 0;
    if (eventNumber % 1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    readWaveform = *waveform;
    
    if (usingAccumulator)
    {
      integral = (accumulator-baseline*windowLength*1.0)*polarity*1.0;
    }
    if (!usingAccumulator)
    {
      integral = 0;
      for (Int_t i = 0; i<readWaveform.size(); i++)
      {
        temp = (readWaveform[i]-baseline*1.0)*polarity*1.0;
	if (i >= windowStart && i <= windowEnd)
	{
	  integral += temp;
	}
      }
    }
    hist->Fill(integral, 1/upTime*1.0); //increment by counts/sec
  }

  //Draw
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  hist->GetXaxis()->SetTitle("Integral");
  hist->GetYaxis()->SetTitle("Counts/second");
  hist->Draw("hist");

  if (saveSpectra)
  {
    //if option is chosen,  create a file with the spectra
    TString newFile = file;
    TString extension = ".root";
    newFile = newFile.Remove(newFile.Length()-extension.Length(), extension.Length());
    newFile += "Spectra.root";
    TFile *myFile = new TFile(newFile, "RECREATE");
    hist->Write("hist");
    myFile->Close();
  }
}
