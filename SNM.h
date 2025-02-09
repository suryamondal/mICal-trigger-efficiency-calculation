//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Sun Mar  3 02:53:32 2019 by ROOT version 5.34/36
// from TTree SNM/SNM
// found on file: SNM_multiTDC1_RPC_evtraw-16082017-114755_00000.root
//////////////////////////////////////////////////////////

#ifndef SNM_h
#define SNM_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TString.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class SNM {
 public :
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain
   
  // Declaration of leaf types
  ULong64_t       nevt;
  Double_t        evetime[12];
  ULong64_t       xydata[2][12];
  UChar_t         xythit[2][12][8];
  Int_t           xytime[2][12][8][256];
  UShort_t        plWidth[2][12][8][256];
  
  // List of branches
  TBranch        *b_nevt;   //!
  TBranch        *b_evetime;   //!
  TBranch        *b_xydata;   //!
  TBranch        *b_xythit[2][12][8];   //!
  TBranch        *b_xytime[2][12][8];   //!
  TBranch        *b_plWidth[2][12][8];   //!
  
  SNM(TTree *tree=0);
  virtual ~SNM();
  virtual Int_t    Cut(Long64_t entry);
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Loop();
  virtual Bool_t   Notify();
  virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef SNM_cxx
SNM::SNM(TTree *tree) : fChain(0) 
{
  // if parameter tree is not specified (or zero), connect the file
  // used to generate this class and read the Tree.
  if (tree == 0) {
    TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("SNM_multiTDC1_RPC_evtraw-16082017-114755_00000.root");
    if (!f || !f->IsOpen()) {
      f = new TFile("SNM_multiTDC1_RPC_evtraw-16082017-114755_00000.root");
    }
    f->GetObject("SNM",tree);

  }
  Init(tree);
}

SNM::~SNM()
{
  if (!fChain) return;
  delete fChain->GetCurrentFile();
}

Int_t SNM::GetEntry(Long64_t entry)
{
  // Read contents of entry.
  if (!fChain) return 0;
  return fChain->GetEntry(entry);
}
Long64_t SNM::LoadTree(Long64_t entry)
{
  // Set the environment to read one entry
  if (!fChain) return -5;
  Long64_t centry = fChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fChain->GetTreeNumber() != fCurrent) {
    fCurrent = fChain->GetTreeNumber();
    Notify();
  }
  return centry;
}

void SNM::Init(TTree *tree)
{
  // The Init() function is called when the selector needs to initialize
  // a new tree or chain. Typically here the branch addresses and branch
  // pointers of the tree will be set.
  // It is normally not necessary to make changes to the generated
  // code, but the routine can be extended by the user if needed.
  // Init() will be called many times when running on PROOF
  // (once per file to be processed).

  // Set branch addresses and branch pointers

  const char *sideMark[2] = {"x","y"};
  
  if (!tree) return;
  fChain = tree;
  fCurrent = -1;
  fChain->SetMakeClass(1);

  fChain->SetBranchAddress("nevt", &nevt, &b_nevt);
  fChain->SetBranchAddress("evetime", evetime, &b_evetime);
  fChain->SetBranchAddress("xydata", xydata, &b_xydata);
  for(int nj=0;nj<2;nj++) {
    for(int ij=0;ij<12;ij++) {
      for(int jk=0;jk<8;jk++) {
	fChain->SetBranchAddress(TString::Format("xythit_%s_l%i_%i",sideMark[nj],ij,jk),&xythit[nj][ij][jk],&b_xythit[nj][ij][jk]);
	fChain->SetBranchAddress(TString::Format("xytime_%s_l%i_%i",sideMark[nj],ij,jk),xytime[nj][ij][jk],&b_xytime[nj][ij][jk]);
	fChain->SetBranchAddress(TString::Format("plWidth_%s_l%i_%i",sideMark[nj],ij,jk),plWidth[nj][ij][jk],&b_plWidth[nj][ij][jk]);
      }
    }
  }
  Notify();
}

Bool_t SNM::Notify()
{
  // The Notify() function is called when a new file is opened. This
  // can be either for a new TTree in a TChain or when when a new TTree
  // is started when using PROOF. It is normally not necessary to make changes
  // to the generated code, but the routine can be extended by the
  // user if needed. The return value is currently not used.

  return kTRUE;
}

void SNM::Show(Long64_t entry)
{
  // Print contents of entry.
  // If entry is not specified, print current entry
  if (!fChain) return;
  fChain->Show(entry);
}
Int_t SNM::Cut(Long64_t entry)
{
  // This function may be called from Loop.
  // returns  1 if entry is accepted.
  // returns -1 otherwise.
  return 1;
}
#endif // #ifdef SNM_cxx
