//C++ libraries
#include <iostream>
#include <fstream>
#include <vector>

//ROOT libraries
#include <TApplication.h>
#include <TH1.h>
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
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

  int Multiplicity = 0;
  
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

  TH1F* MultiplicityDistribution = new TH1F("MultiplicityDist", "", 32, 0, 32);

  string FileNameString = (string)FileName;
  bool IsRootFile = false;

  if(FileNameString.find(".root") != string::npos){
    IsRootFile = true;
  }

  cout << endl << "Processing file " << FileName << "..." << endl;

  //Reading if file is not of ROOT type i.e. of ASCII type
  if(IsRootFile == false){
    
    ifstream InputDataFile;
    InputDataFile.open(Form("%s", FilePath));
    
    //Reads one line (up to 1024 characters) from file to remove the header
    char DummyLine[1024];
    InputDataFile.getline(DummyLine, 1024);

    //Loops over full ASCII file (abort criterion below)
    while(true){
      
      //Loops over all active channels (32 unless otherwise specified)
      for(int ReadingChannel = 0; ReadingChannel < NumberOfChannels; ReadingChannel++){
	//Reads the three entries (hit flag, low gain value, high gain value) for one channel
	InputDataFile >> InputFlag >> InputLowGainValue >> InputHighGainValue;
	//Pushes read values into the vector corresponding to the read channel
	Flags[ReadingChannel].push_back(InputFlag);
	LowGainValues[ReadingChannel].push_back(InputLowGainValue);
	HighGainValues[ReadingChannel].push_back(InputHighGainValue);
      }
      
      if(InputDataFile.eof()) break;
      
      //After reading NumberOfChannels x three entries, the temperature is read
      InputDataFile >> InputTemperature;
      Temperatures.push_back(InputTemperature);
      
    }  //End of while loop reading full ASCII file

  }  //End of case "is not a ROOT file but ASCII file"

  if(IsRootFile == true){

    TFile* InputRootFile = new TFile(Form("%s", FilePath));

    if(InputRootFile->GetListOfKeys()->Contains("tree") == 0){
      cout << endl;
      cout << "Relevant data not found! Incorrect ROOT file given?" << endl;
      cout << endl;
      return 0;
    }

    //Obtains a pointer to the tree in the file
    TTree* InputTree = (TTree*)InputRootFile->Get("tree");

    //Pointer to the leaf where data will be read
    TLeaf* InputLeaf;

    for(int EntryNumber = 0; EntryNumber < InputTree->GetEntries(); EntryNumber++){

      InputTree->GetEntry(EntryNumber);
      
      for(int ReadingChannel = 0; ReadingChannel < NumberOfChannels; ReadingChannel++){
	
	//Getting hit flag
	InputLeaf = (TLeaf*)InputTree->GetLeaf(Form("hit%i", ReadingChannel));
	Flags[ReadingChannel].push_back(InputLeaf->GetValue());

	//Getting low gain value
	InputLeaf = (TLeaf*)InputTree->GetLeaf(Form("ChargeLG%i", ReadingChannel));
	LowGainValues[ReadingChannel].push_back(InputLeaf->GetValue());

	//Getting high gain value
	InputLeaf = (TLeaf*)InputTree->GetLeaf(Form("ChargeHG%i", ReadingChannel));
	HighGainValues[ReadingChannel].push_back(InputLeaf->GetValue());

      }

      //Getting temperature (just once per event, not once per channel)
      InputLeaf = (TLeaf*)InputTree->GetLeaf("temp");
      Temperatures.push_back(InputLeaf->GetValue());

    }

  }

  //Number of entries is size of Temperatures vector (note: "size" is unsigned)
  for(unsigned int EntryNumber = 0; EntryNumber < Temperatures.size(); EntryNumber++){

    for(int ReadingChannel = 0; ReadingChannel < NumberOfChannels; ReadingChannel++){
      
      if(Flags[ReadingChannel][EntryNumber] == 1){
    	Multiplicity++;
	if(ReadingChannel == PlottingChannel){
	    SingleSpectrum_LG->Fill(LowGainValues[ReadingChannel][EntryNumber]);
	    SingleSpectrum_HG->Fill(HighGainValues[ReadingChannel][EntryNumber]);
	}
	if(ReadingChannel < 16){
	  MPPC_Array_LG[ReadingChannel]->Fill(LowGainValues[ReadingChannel][EntryNumber]);
	  MPPC_Array_HG[ReadingChannel]->Fill(HighGainValues[ReadingChannel][EntryNumber]);
	  InputLowSum += LowGainValues[ReadingChannel][EntryNumber];
	  InputHighSum += HighGainValues[ReadingChannel][EntryNumber];
	}
 
      }
      
    }  //End of for all channels

    //Filling histograms
    ArraySummedSpectrum_LG->Fill(InputLowSum);
    ArraySummedSpectrum_HG->Fill(InputHighSum);
    MultiplicityDistribution->Fill(Multiplicity);
    
    //Resetting for next run
    InputLowSum = 0;
    InputHighSum = 0;
    Multiplicity = 0;
      
  }  //End of for all entries
    
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

  TCanvas* MultiplicityCanvas = new TCanvas("MultiplicityCanvas", "Multiplicity", 0, 0, 900, 700);
  MultiplicityCanvas->cd();
  MultiplicityDistribution->Draw();
  MultiplicityCanvas->SetLogy();

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
