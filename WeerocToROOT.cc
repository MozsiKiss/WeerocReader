void WeerocToROOT(string FileName){

  cout << endl;

  //Output file
  TFile* f = new TFile("OutputFile.root", "RECREATE");

  //Setting up a ROOT tree structure for reading
  TTree* Tree = new TTree("tree", "", 0);

  //File for reading
  Tree->ReadFile("TempFile.txt");

  //Writes the tree to the root file
  Tree->Write();

  //Closes the input file
  f->Close();

  return;

}
