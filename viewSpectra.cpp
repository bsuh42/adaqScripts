#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void viewSpectra(TString file, Bool_t saveSpectra = 0, Int_t windowStart = 0, Int_t windowEnd = 0)
{
  //View single spectra
  //TString file: file where data is stored
  //Bool_t saveSpectra: 0 to not save the spectra, 1 to save
  //Int_t windowStart: if =windowEnd=0, use accumulators. Otherwise, integrate waveform
  
  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  Double_t baseline = 0;
  Double_t polarity = 0;
  Double_t temp = 0;
  Double_t integral = 0;
  Double_t accumulator = 0;
  Int_t accumulatorStart = 0;
  Int_t accumulatorEnd = 0;
  Int_t counter = 0;
  Int_t windowLength = 0;
  vector<UShort_t> *waveform = 0;
  vector<UShort_t> readWaveform;
  Bool_t usingAccumulator = 0;

  const Double_t histLower = -1000;
  const Double_t histUpper = 3000;
  const Double_t histLength = histUpper-histLower; //change these variables so as to view histogram. At the moment, have no binning

  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("baseline", &baseline);
  T->SetBranchAddress("polarity", &polarity);
  T->SetBranchAddress("accumulator1", &accumulator);
  T->SetBranchAddress("accumulator1Start", &accumulatorStart);
  T->SetBranchAddress("accumulator1End", &accumulatorEnd);
  T->GetEntry(0);

  if (windowStart == 0 && windowEnd == 0)
  {
    //If the window isn't set, use accumulator1
    windowStart = accumulatorStart;
    windowEnd = accumulatorEnd;
    usingAccumulator = 1;
  }

  windowLength = windowEnd-windowStart + 1; //Since we use inclusive counting

  TH1D* hist = new TH1D("", "", histLength, histLower, histUpper);

  const Int_t numberEntries = T->GetEntries();
  readWaveform = *waveform;
  Double_t upTime = readWaveform.size()*4.0*numberEntries; //convert runTime into nanoseconds
  upTime /= 1e9; //convert to seconds

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
    integral = (accumulator - baseline*windowLength*1.0)*polarity*1.0; //set the integral using accumulator first rather than putting this in an if statement

    if (!usingAccumulator)
    {
      integral = 0;
      for (Int_t i = 0; i < readWaveform.size(); i++)
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

  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  hist->GetXaxis()->SetTitle("Integral");
  hist->GetYaxis()->SetTitle("Counts/second");
  hist->Draw("hist");

  if (saveSpectra)
  {
    //If option chosen, create a file with the spectra
    TString newFile = file;
    TString extension = ".root";
    newFile = newFile.Remove(newFile.Length()-extension.Length(), extension.Length());
    newFile += "Spectra.root";
    TFile *myFile = new TFile(newFile, "RECREATE");
    hist->Write("hist");
    myFile->Close();
  }
}
