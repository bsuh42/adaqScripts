#include <iostream>
#include <string>
using namespace std;
#include <cmath>
#include <vector>

R__LOAD_LIBRARY(libADAQReadout)

void adaq2root(TString file, Double_t baseline, Double_t polarity, Int_t waveformFrequency = 1, Int_t accumulator0Start = 0, Int_t accumulator0End = 140, Int_t accumulator1Start = 141, Int_t accumulator1End = 160, Int_t accumulator2Start = 161, Int_t accumulator2End = 500)
{
  //Convert adaq to root file
  //TString file: location of file to convert
  //Double_t baseline: fixed baseline estimate. Get this number from watching run in adaq gui
  //Double_t polarity: -1 for a negative going pulse, 1 for a positive going pulse
  //Accumulator windows take the raw integral in that region
  //Accumulator 0 should be pre-signal
  //Accumulator 1 should be signal
  //Accumulator 2 should be post-signal
  //Int_t waveformFrequency: how many waveforms to store. If 0 is chosen, do not save waveforms. If 1 is chosen, save all waveforms
  
  //Load adaq file and open tree
  gSystem->Load("libADAQReadout.so");
  TFile *F = new TFile(file);
  TTree *T = (TTree *)F->Get("WaveformTree");

  //Variables
  Double_t temp = 0;
  Int_t counter = 0;
  Double_t accumulator0 = 0;
  Double_t accumulator1 = 0;
  Double_t accumulator2 = 0;
  Double_t amplitude = 0; //Baseline subtracted max amplitude of waveform
  Int_t maxAmplitudePosition = 0;
  vector<UShort_t> waveform;
  const Int_t numberEntries = T->GetEntries();

  //Create target file and tree to store new waveform
  TString newFile = file;
  TString extension = ".adaq.root";
  newFile = newFile.Remove(newFile.Length()-extension.Length(), extension.Length());
  newFile += ".root";
  TFile *myFile = new TFile(newFile, "CREATE"); //Use create so as not to overwrite original file by accident
  TTree *tree = new TTree("waveformTree", "stores data from ADAQ");
  //Create branches
  tree->Branch("waveform", &waveform);
  tree->Branch("baseline", &baseline);
  tree->Branch("polarity", &polarity);
  tree->Branch("accumulator0Start", &accumulator0Start);
  tree->Branch("accumulator0End", &accumulator0End);
  tree->Branch("accumulator0", &accumulator0);
  tree->Branch("accumulator1Start", &accumulator1Start);
  tree->Branch("accumulator1End", &accumulator1End);
  tree->Branch("accumulator1", &accumulator1);
  tree->Branch("accumulator2Start", &accumulator2Start);
  tree->Branch("accumulator2End", &accumulator2End);
  tree->Branch("accumulator2", &accumulator2);
  tree->Branch("amplitude", &amplitude);
  tree->Branch("maxAmplitudePosition", &maxAmplitudePosition);

  //Read in data from adaq file
  vector<UShort_t> *Waveforms[8];
  T->SetBranchAddress("WaveformCh0", &Waveforms[0]); //Currently only reads from channel 0
  vector<UShort_t> readWaveform;

  //keep these histograms. Not used
  TH1D* spectrumHist = new TH1D("spectrumHist", "spectrumHist", 1,0,0);
  TH1D* amplitudeHist = new TH1D("amplitudeHist", "amplitudeHist", 1,0,0);

  //Loop through events and write to tree
  for (Int_t eventNumber = 0; eventNumber < numberEntries; eventNumber++)
  {
    accumulator0 = 0;
    accumulator1 = 0;
    accumulator2 = 0;
    amplitude = 0;
    maxAmplitudePosition = 0;
    if (eventNumber % 1000 == 0)
    {
      printf("Currently on event %d of %d\n", eventNumber, numberEntries);
    }
    T->GetEntry(eventNumber);
    readWaveform = *Waveforms[0];
    waveform.clear();

    for (Int_t i = 0; i < readWaveform.size(); i++)
    {
      temp = (readWaveform[i]*1.0-baseline)*polarity;
      if (i >= accumulator0Start && i <= accumulator0End)
      {
        accumulator0 += readWaveform[i];
      }
      if (i >= accumulator1Start && i <= accumulator1End)
      {
        accumulator1 += readWaveform[i];
      }
      if (i >= accumulator2Start && i <= accumulator2End)
      {
	accumulator2 += readWaveform[i];
      }
      if (temp >= amplitude)
      {
	amplitude = temp;
	maxAmplitudePosition = i;
      }
      if (waveformFrequency != 0 && eventNumber % waveformFrequency != 0)
      {
        waveform.clear();
      }
      if (waveformFrequency == 0 && eventNumber != 0)
      {
	waveform.clear();
      }
      waveform.push_back(readWaveform[i]);
    }
    tree->Fill();
  }
  myFile->Write();
}
