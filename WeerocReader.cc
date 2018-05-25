//C++ libraries
#include <iostream>
#include <fstream>
#include <vector>

//ROOT libraries
#include <TApplication.h>
#include <TH1.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TLegend.h>
#include <TPaveStats.h>
#include <TStyle.h>

using namespace std;

int main(int argc, char* argv[]){

  gStyle->SetOptStat(111111);

  TApplication theApp("App", 0, 0);

  //Plotting is suppressed if an additional argument (any) is given
  if(argc == 3){
    gROOT->SetBatch(kTRUE);
  }

  if(argc < 2){
    cout << endl;
    cout << "Please give valid txt file as argument!" << endl;
    cout << endl;
    return 0;
  }

  char* FilePath = argv[1];
  char* FileName = basename(argv[1]);

  //Checks for the existance of the two-hit data root file
  if(access(Form("%s", FilePath), F_OK) != 0){
    cout << endl << "Error opening file! Please check target directory!" << endl << endl;
    return 0;
  }

  cout << endl << "Processing file " << FileName << "..." << endl;

  ifstream InputDataFile;
  InputDataFile.open(Form("%s", FilePath));

  //Number of channels in use (may change through masking or zero suppression)
  const int NumberOfChannels = 32;

  //Definition of variables
  //Values to be read from file are arrays of vectors (flags, gains) or only vectors (temperatures)
  vector<int> Flags[NumberOfChannels];
  vector<int> LowGainValues[NumberOfChannels];
  vector<int> HighGainValues[NumberOfChannels];
  vector<int> Temperatures;

  int InputFlag = 0;
  int InputLowGainValue = 0;
  int InputHighGainValue = 0;
  int InputTemperature = 0;
  int PlottingChannel = 31;
  int InputLowSum = 0;
  int InputHighSum = 0;
  
  cout << endl << "Please input which channel to plot: ";
  cin >> PlottingChannel;
  TH1F* SingleSpectrum_LG = new TH1F(Form("SingleSpectrum_LG ch. %i", PlottingChannel), "", 4096, 0, 4095);
  TH1F* SingleSpectrum_HG = new TH1F(Form("SingleSpectrum_HG ch. %i", PlottingChannel), "", 4096, 0, 4095);
  TH1F* MPPC_Array_LG[NumberOfChannels];
  TH1F* MPPC_Array_HG[NumberOfChannels];
  for(int Channel = 0; Channel < NumberOfChannels; Channel++){
    MPPC_Array_LG[Channel] = new TH1F(Form("MPPC_Array_LG ch. %i", Channel), "", 4096, 0, 4095);
    MPPC_Array_LG[Channel]->SetLineColor(kRed);
    MPPC_Array_HG[Channel] = new TH1F(Form("MPPC_Array_HG ch. %i", Channel), "", 4096, 0, 4095);
    MPPC_Array_HG[Channel]->SetLineColor(kGreen);
  }
  TH1F* ArraySummedSpectrum_LG = new TH1F("ArraySummedSpectrum_LG", "", 4096, 0, 16*4096);
  TH1F* ArraySummedSpectrum_HG = new TH1F("ArraySummedSpectrum_HG", "", 4096, 0, 16*4096);
  

  //Reads one line (up to 1024 characters) from file to remove the header
  char DummyLine[1024];
  InputDataFile.getline(DummyLine, 1024);

  while(true){
    if(InputDataFile.eof()) break;
    //Loops over all active channels (32 unless otherwise specified)
    for(int ReadingChannel = 0; ReadingChannel < NumberOfChannels; ReadingChannel++){
      //Reads the three entries (hit flag, low gain value, high gain value) for one channel
      InputDataFile >> InputFlag >> InputLowGainValue >> InputHighGainValue;
      //Pushes read values into the vector corresponding to the read channel
      Flags[ReadingChannel].push_back(InputFlag);
      LowGainValues[ReadingChannel].push_back(InputLowGainValue);
      HighGainValues[ReadingChannel].push_back(InputHighGainValue);
      if((ReadingChannel == PlottingChannel) && (InputFlag == 1)){
	SingleSpectrum_LG->Fill(InputLowGainValue);
	SingleSpectrum_HG->Fill(InputHighGainValue);
      }
      if((ReadingChannel < 16) && (InputFlag == 1)){
      	MPPC_Array_LG[ReadingChannel]->Fill(InputLowGainValue);
      	MPPC_Array_HG[ReadingChannel]->Fill(InputHighGainValue);
	InputLowSum += InputLowGainValue;
	InputHighSum += InputHighGainValue;
      }
    }
    
    //After reading NumberOfChannels x three entries, the temperature is read
    InputDataFile >> InputTemperature;
    Temperatures.push_back(InputTemperature);

    ArraySummedSpectrum_LG->Fill(InputLowSum);
    ArraySummedSpectrum_HG->Fill(InputHighSum);
    InputLowSum = 0;
    InputHighSum = 0;

  }

  TCanvas* ArrayCanvas = new TCanvas("ArrayCanvas", "Array canvas", 0, 0, 1100, 900);
  ArrayCanvas->Divide(4, 4);
  for(int Row = 0; Row < 4; Row++){
    for(int ColumnFromRight = 0; ColumnFromRight < 4; ColumnFromRight++){
      ArrayCanvas->cd(Row*4 + (4 - ColumnFromRight))->SetLogy();
      MPPC_Array_LG[Row*4+ColumnFromRight]->Draw();
      MPPC_Array_HG[Row*4+ColumnFromRight]->Draw("sames");
    }
  }

  TCanvas* ArraySummedSpectrumCanvas = new TCanvas("ArraySummedSpectrumCanvas", "MPPC array summed spectrum", 1100, 0, 600, 434);
  //TCanvas* ArraySummedSpectrumCanvas = new TCanvas();
  ArraySummedSpectrumCanvas->cd();
  ArraySummedSpectrum_LG->SetLineColor(kRed);
  ArraySummedSpectrum_HG->SetLineColor(kGreen);
  ArraySummedSpectrum_LG->Draw();
  ArraySummedSpectrum_HG->Draw("sames");
  ArraySummedSpectrumCanvas->SetLogy();

  TLegend* I_Am_Legend_Array = new TLegend(0.5, 0.5, 0.7, 0.6);
  I_Am_Legend_Array->AddEntry(ArraySummedSpectrum_LG, "Low gain", "l");
  I_Am_Legend_Array->AddEntry(ArraySummedSpectrum_HG, "High gain", "l");
  I_Am_Legend_Array->Draw();

  ArraySummedSpectrumCanvas->Update();
  
  //Needed to ensure that the stat box is not overlapping anything important
  TPaveStats* PointerToStats = (TPaveStats*)ArraySummedSpectrum_HG->FindObject("stats");
  PointerToStats->SetY1NDC(0.68);
  PointerToStats->SetY2NDC(0.44);

  ArraySummedSpectrumCanvas->Modified();
  ArraySummedSpectrumCanvas->Update();

  TCanvas* SingleSpectrumCanvas = new TCanvas("SingleSpectrumCanvas", Form("Single MPPC spectrum ch. %i", PlottingChannel), 1100, 466, 600, 434);
  SingleSpectrumCanvas->cd();
  SingleSpectrum_LG->SetLineColor(kRed);
  SingleSpectrum_HG->SetLineColor(kGreen);
  SingleSpectrum_LG->Draw();
  SingleSpectrum_HG->Draw("sames");
  SingleSpectrumCanvas->SetLogy();

  TLegend* I_Am_Legend = new TLegend(0.5, 0.5, 0.7, 0.6);
  I_Am_Legend->AddEntry(SingleSpectrum_LG, "Low gain", "l");
  I_Am_Legend->AddEntry(SingleSpectrum_HG, "High gain", "l");
  I_Am_Legend->Draw();

  SingleSpectrumCanvas->Update();

  //Needed to ensure that the stat box is not overlapping anything important
  PointerToStats = (TPaveStats*)SingleSpectrum_HG->FindObject("stats");
  PointerToStats->SetY1NDC(0.68);
  PointerToStats->SetY2NDC(0.44);

  SingleSpectrumCanvas->Modified();
  SingleSpectrumCanvas->Update();

  //Execution is stopped automatically if an additional argument (any) is given
  if(argc < 3){
    theApp.Run();
  }

  return 0;

}
