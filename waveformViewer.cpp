#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void viewWaveform(TString file, const Int_t waveformNumber = 0)
{
  //View waveforms
  //Inputs:
  ////TString file: file containing waveform
  ////Int_t waveformNumber: which waveform to view
  
  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  Double_t baseline = 0;
  Double_t polarity = 0;
  Double_t temp = 0;
  Double_t integral = 0;
  Double_t binSize = 4; //Length in nanoseconds of each time bin

  const Int_t integralStart = 155;
  const Int_t integralEnd = 180; //If you want, find integral between these two values

  //Get tree variables
  vector<UShort_t> *waveform = 0;
  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("baseline", &baseline);
  T->SetBranchAddress("polarity", &polarity); //Baseline subtracting and positivizing
  T->GetEntry(waveformNumber);
  vector<UShort_t> readWaveform = *waveform;

  //Create the histogram to draw waveform
  const Int_t waveformStart = 0;
  const Int_t waveformEnd = readWaveform.size()*binSize;
  const Int_t waveformLength = (waveformEnd-waveformStart)/binSize;
  TH1D* hist = new TH1D("", "", waveformLength, waveformStart, waveformEnd);

  for (Int_t i = 0; i < readWaveform.size(); i++)
  {
    temp = (readWaveform[i]-baseline)*polarity;
    hist->Fill(i*binSize, temp);

    if (i >= integralStart && i <= integralEnd)
    {
      integral += temp;
    }
  }

  //Draw waveform
  TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
  gStyle->SetOptStat(0);
  hist->GetXaxis()->SetTitle("Time(ns)");
  hist->Draw("hist");
  printf("Integral = %f\n", integral);
  c1->Update();
}
