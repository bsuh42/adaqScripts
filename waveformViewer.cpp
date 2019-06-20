#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

void viewWaveform(TString file, const Int_t waveformNumber=0)
{
  //View waveforms
  //TString file: file containing waveform
  //Int_t waveformNumber: which waveform to view
  //Load file
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("waveformTree");

  //Variables
  Double_t baseline = 0;
  Double_t polarity = 0;
  Double_t temp = 0;

  vector<UShort_t> *waveform = 0;
  T->SetBranchAddress("waveform", &waveform);
  T->SetBranchAddress("baseline", &baseline);//Do some baseline subtracting and positivizing
  T->SetBranchAddress("polarity", &polarity);
  T->GetEntry(waveformNumber);
  vector<UShort_t> readWaveform = *waveform;

  const Int_t waveformStart = 0;
  const Int_t waveformEnd = readWaveform.size()*4;
  const Int_t waveformLength = (waveformEnd-waveformStart)/4;

  TH1D* hist = new TH1D("", "", waveformLength, waveformStart, waveformEnd);

 for (Int_t i = 0; i < readWaveform.size(); i++)
 {
   temp = (readWaveform[i]-baseline)*polarity;
   hist->Fill(i*4, temp);
 }
 
 TCanvas *c1 = new TCanvas("c1", "", 1000, 600);
 gStyle->SetOptStat(0);
 hist->GetXaxis()->SetTitle("Time(ns)");
 hist->Draw("hist");
 c1->Update();
}
