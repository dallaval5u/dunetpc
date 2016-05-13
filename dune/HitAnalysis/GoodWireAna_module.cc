////////////////////////////////////////////////////////////////////////
// Class:       GoodWireAna
// Module Type: analyzer
// File:        GoodWireAna_module.cc
//
// Generated at Tue Apr 26 20:18:40 2016 by Ryan Linehan using artmod
// from cetpkgsupport v1_10_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Services/Optional/TFileService.h"



#include "larcore/Geometry/Geometry.h"
#include "larcore/Geometry/PlaneGeo.h"
#include "larcore/Geometry/WireGeo.h"
#include "lardata/RecoBase/Hit.h"


#include "TH1D.h"
#include "TF1.h"
#include <fstream>
#include <iostream>

class GoodWireAna;

class GoodWireAna : public art::EDAnalyzer {
public:
  explicit GoodWireAna(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  GoodWireAna(GoodWireAna const &) = delete;
  GoodWireAna(GoodWireAna &&) = delete;
  GoodWireAna & operator = (GoodWireAna const &) = delete;
  GoodWireAna & operator = (GoodWireAna &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

  // Selected optional functions.
  void beginJob() override;
  void beginRun(art::Run const & r) override;
  void beginSubRun(art::SubRun const & sr) override;
  void endJob() override;
  void endRun(art::Run const & r) override;
  void endSubRun(art::SubRun const & sr) override;
  void reconfigure(fhicl::ParameterSet const & p) override;
  void respondToCloseInputFile(art::FileBlock const & fb) override;
  void respondToCloseOutputFiles(art::FileBlock const & fb) override;
  void respondToOpenInputFile(art::FileBlock const & fb) override;
  void respondToOpenOutputFiles(art::FileBlock const & fb) override;


  //User-defined functions
  void makeHistoSetForThisRun( int runID );
  void writeListOfDeadWires();
  void fillHitOccDistHists(std::vector<std::vector<size_t> > & badWireVect);
  void fitHitOccDistHists( std::vector<std::vector<size_t> > & badWireList, std::vector<std::vector<size_t> > & goodWireList);
  void writeListOfBadWires(std::vector<std::vector<size_t> > badWireVect );
  void writeListOfGoodWires(std::vector<std::vector<size_t> > goodWireVect );
  

private:

  // Declare member data here.
  
  //Histograms
  std::map<int,std::map<std::pair<size_t,size_t>,std::vector<TH1D*> > > fRunToCryTPCToPlaneMap;
  std::map<int,std::map<std::pair<size_t,size_t>,std::vector<TH1D*> > > fRunToCryTPCToPlaneMapDist;


  //Geometry service
  geo::Geometry*            fGeometry;              ///<  pointer to the Geometry service

  //TFileService
  art::TFileService*        fTFS;

  //Detector parameters
  size_t fNCry;

  //fcl file parameters: module labels
  std::string fHitModuleLabel;

  //Additional parameters
  double fNSigmaGoodWire;
  double fHitOccLimit;
  size_t fHitLimitPerWirePerEventCol;
  size_t fHitLimitPerWirePerEventInd;


  //Misc
  std::map<int,size_t> fNEvtsPerRun;
  bool fVerbose;
  bool fOnlyWriteCollectionPlane;



};


//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
GoodWireAna::GoodWireAna(fhicl::ParameterSet const & pset)
  :
  EDAnalyzer(pset)  // ,
 // More initializers here.
{

  //Setting geometry info
  art::ServiceHandle<geo::Geometry>            geometry;
  fGeometry = &*geometry;
  fNCry = fGeometry->Ncryostats();
 
  //TFile Service
  art::ServiceHandle<art::TFileService> tfs;
  fTFS = &*tfs;

  //Reconfigure to set the data members
  this->reconfigure(pset);
}



//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
void GoodWireAna::analyze(art::Event const & e)
{
  // Implementation of required member function here.
  
  //Check to see if this is a new run. If it is, then make a new set of histos
  if( !fRunToCryTPCToPlaneMap.count(e.run()) ) makeHistoSetForThisRun(e.run());

  //Increment nEvents for this run
  if( fNEvtsPerRun.count(e.run()) ) fNEvtsPerRun.at(e.run())++;
  else{ fNEvtsPerRun.emplace(e.run(),1); }

  //Now get all a da hits from this event
  art::Handle<std::vector<recob::Hit> > hitListHandle;
  e.getByLabel( fHitModuleLabel, hitListHandle );
  
  //Loop over the hits. Find their Cryo/TPC, plane, and fill the appropriate histogram
  //labeled by those three things.
  for( size_t iHit = 0; iHit < hitListHandle->size(); ++iHit ){
    
    //Get the hit
    recob::Hit theHit = hitListHandle->at(iHit);
    
    //Get hit info
    size_t iCry = theHit.WireID().Cryostat;
    size_t iTPC = theHit.WireID().TPC;
    size_t iPlane = theHit.WireID().Plane;
    size_t iWire = theHit.WireID().Wire;


    //Make the pair for Cryostat/TPC
    std::pair<size_t,size_t> CryTPCPair( iCry,iTPC );
    
    //Now fill the appropriate histogram
    fRunToCryTPCToPlaneMap.at( e.run() ).at( CryTPCPair ).at(iPlane)->Fill(iWire);

  }
  

  


}


//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//Here, we want to make a new set of hit occupancy histos for a run. This is 
//called only if a set of histos doesn't already exist for a given run.
void GoodWireAna::makeHistoSetForThisRun( int runID )
{
  //Create the output map of (Cryo/TPC) to (HistoVect)
  std::map< std::pair<size_t,size_t>, std::vector<TH1D*> > outputMap;

  //Create a second output map for histos plotting wire hit occupancy distributions (not the occupancies themselves)
  std::map< std::pair<size_t,size_t>, std::vector<TH1D*> > outputDistMap;

  //Loop over the cryostats 
  for( size_t iCry = 0; iCry < fNCry; ++iCry ){

    //Find the number of TPCs in this cryostat
    size_t nTPCs = fGeometry->NTPC(iCry);
    
    //Loop over TPCs and create a vector for each one that will
    //hold the different planes' histograms
    for( size_t iTPC = 0; iTPC < nTPCs; ++iTPC ){
      std::vector<TH1D*> histoVect;
      std::vector<TH1D*> histoDistVect;

      //Define the pair
      std::pair<size_t,size_t> CryTPCPair(iCry,iTPC);

      //Get geometry information
      size_t nWiresU = fGeometry->Nwires(0,iTPC,iCry);
      size_t nWiresV = fGeometry->Nwires(1,iTPC,iCry);
      size_t nWiresW = fGeometry->Nwires(2,iTPC,iCry);

      char name[50];
      char title[70];
      
      //U Plane
      int n = sprintf( name, "r%d_cry%lu_tpc%lu_U_WireHitOcc", runID, iCry, iTPC);
      int t = sprintf( title, ";Run %d, Cryostat %lu, TPC %lu, U Plane Wire Hit Occupancies;", runID, iCry, iTPC);
      TH1D* uHisto = fTFS->make<TH1D>(name,title,nWiresU,0,nWiresU);
 
      //U Plane
      n = sprintf( name, "r%d_cry%lu_tpc%lu_V_WireHitOcc", runID, iCry, iTPC);
      t = sprintf( title, ";Run %d, Cryostat %lu, TPC %lu, V Plane Wire Hit Occupancies;", runID, iCry, iTPC);
      TH1D* vHisto = fTFS->make<TH1D>(name,title,nWiresV,0,nWiresV);
      
      //W Plane
      n = sprintf( name, "r%d_cry%lu_tpc%lu_W_WireHitOcc", runID, iCry, iTPC);
      t = sprintf( title, ";Run %d, Cryostat %lu, TPC %lu, W Plane Wire Hit Occupancies;", runID, iCry, iTPC);
      TH1D* wHisto = fTFS->make<TH1D>(name,title,nWiresW,0,nWiresW);

      //Push the three histograms back into the histoVect
      histoVect.push_back(uHisto);
      histoVect.push_back(vHisto);
      histoVect.push_back(wHisto);

      //Now make the hit occupancy distributions
      //U Plane
      n = sprintf( name, "r%d_cry%lu_tpc%lu_U_HitOccDist", runID, iCry, iTPC);
      t = sprintf( title, ";Run %d, Cryostat %lu, TPC %lu, U Plane Hit Occupancy Distribution;", runID, iCry, iTPC);
      TH1D* uHistoDist = fTFS->make<TH1D>(name,title,100,0,-1);
 
      //U Plane
      n = sprintf( name, "r%d_cry%lu_tpc%lu_V_HitOccDist", runID, iCry, iTPC);
      t = sprintf( title, ";Run %d, Cryostat %lu, TPC %lu, V Plane Hit Occupancy Distribution;", runID, iCry, iTPC);
      TH1D* vHistoDist = fTFS->make<TH1D>(name,title,100,0,-1);
      
      //W Plane
      n = sprintf( name, "r%d_cry%lu_tpc%lu_W_HitOccDist", runID, iCry, iTPC);
      t = sprintf( title, ";Run %d, Cryostat %lu, TPC %lu, W Plane Hit Occupancy Distribution;", runID, iCry, iTPC);
      TH1D* wHistoDist = fTFS->make<TH1D>(name,title,100,0,-1);

      //Push back these three histograms into the histoDistVect
      histoDistVect.push_back(uHistoDist);
      histoDistVect.push_back(vHistoDist);
      histoDistVect.push_back(wHistoDist);

      //Have to "use" n and t for stupid compiler reasons
      if( n ) n = 0;
      if( t ) t = 0;
     
      //Now put this vector into the map to (Cryo/TPC) pair
      outputMap.emplace(CryTPCPair,histoVect);
      outputDistMap.emplace(CryTPCPair,histoDistVect);
    }      
  }
 
  //Now push back this output vector into the persistent one, with the run
  //number as the key
  fRunToCryTPCToPlaneMap.emplace(runID,outputMap);
  fRunToCryTPCToPlaneMapDist.emplace(runID,outputDistMap);
  


}

//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
void GoodWireAna::writeListOfDeadWires()
{
  //Create an ofstream object (outfile)
  ofstream outfile;
  outfile.open("deadWireList.txt");

  if( fVerbose )
    std::cout << "Inside writing function." << std::endl;

  //Loop through all hit occupancy histos
  for( std::map<int,std::map<std::pair<size_t,size_t>,std::vector<TH1D*> > >::iterator iterRun = fRunToCryTPCToPlaneMap.begin(); iterRun != fRunToCryTPCToPlaneMap.end(); ++iterRun ){
    
    //Get run info
    int runID = iterRun->first;
    std::map<std::pair<size_t,size_t>,std::vector<TH1D*> > theCryTPCToPlaneVectMap = iterRun->second;
    
    //Debug
    if( fVerbose )
      std::cout << "runID: " << runID << std::endl;

    for( std::map<std::pair<size_t,size_t>,std::vector<TH1D*> >::iterator iterCryTPC = theCryTPCToPlaneVectMap.begin(); iterCryTPC != theCryTPCToPlaneVectMap.end(); ++iterCryTPC ){
      
      //Get Cry/TPC info
      size_t CryID = iterCryTPC->first.first;
      size_t TPCID = iterCryTPC->first.second;
      std::vector<TH1D*> planeVect = iterCryTPC->second;

      if( fVerbose )
	std::cout << "Cryo/TPC: " << CryID << "/" << TPCID << std::endl;
      

      //Now loop over planes
      for( size_t iPlane = 0; iPlane < planeVect.size(); ++iPlane ){

	
	//Get the plane information
	size_t PlaneID = iPlane;

	//Debug
	if( fVerbose )
	  std::cout << "PlaneID: " << PlaneID << std::endl;
	
	//Get the histogram for this plane
	TH1D * thisPlaneHist = planeVect.at(iPlane);
	
	//Now loop over histogram bins
	for( int iBin = 1; iBin <= thisPlaneHist->GetNbinsX(); ++iBin ){
	  
	  std::cout << "BinID: " << iBin << ", binContent: " << thisPlaneHist->GetBinContent(iBin) << std::endl;


	  //If the bin occupancy is zero, then get the wire number (which is I believe offset from bin number by -1, but Imma check first.
	  if( thisPlaneHist->GetBinContent(iBin) == 0 ){
	    int WireID = iBin-1;

	    std::cout << "Inside the final if statement." << std::endl;

	    //Now print all of this info on one line in our text file.
	    outfile << runID << " " << CryID << " " << TPCID << " " << PlaneID << " " << WireID << "\n";
    
	  }
	}
      }
    }
  }
  
  //Close file
  outfile.close();

}


//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//This function is called after all runs are processed. It loops through all of the
//wires in each wire occ histogram and for each wire, increments a bin in the dist 
//histo corresp to the hit occupancy of that wire. At the end, there will be 24 
//dist histos per run that can be used to make cuts on good/bad wires
void GoodWireAna::fillHitOccDistHists(std::vector<std::vector<size_t> > & badWireVect)
{
  //Loop through each run
  for( std::map<int,std::map<std::pair<size_t,size_t>,std::vector<TH1D*> > >::iterator iter = fRunToCryTPCToPlaneMap.begin(); iter != fRunToCryTPCToPlaneMap.end(); ++iter ){
    
    //Basic info
    int runID = iter->first;
    
    //Loop over the Cry/TPCs
    for( std::map<std::pair<size_t,size_t>,std::vector<TH1D*> >::iterator iterCryTPC = iter->second.begin(); iterCryTPC != iter->second.end(); ++iterCryTPC ){
      
      //Get the TPC/Plane pair
      std::pair<size_t,size_t> CryTPCPair = iterCryTPC->first;

      //Get the plane wire occupancy histograms corresponding to this pair
      std::vector<TH1D*> planeHistos = iterCryTPC->second;
      
      //Loop over these histograms
      for( size_t iPlane = 0; iPlane < planeHistos.size(); ++iPlane ){
	
	//Get the histogram for this plane and loop over its bins
	TH1D * thisPlaneHist = planeHistos.at(iPlane);
	for( int iBin = 1; iBin <= thisPlaneHist->GetNbinsX(); ++iBin ){
	  
	  if( fVerbose )
	    std::cout << "BinID: " << iBin << ", binContent: " << thisPlaneHist->GetBinContent(iBin) << std::endl;

	  //Get the bin occupancy and increment the correct dist histogram in the corresponding bin
	  int binContent = thisPlaneHist->GetBinContent(iBin);

	  //If the bin content is too large (way larger than some reasonable expected value), then don't push
	  //it back into the distribution. This prevents us from having all of the good wires' occupancies fill
	  //a single bin.
	  if( iPlane == 2 ) fHitOccLimit = fNEvtsPerRun.at(runID)*fHitLimitPerWirePerEventCol;
	  if( iPlane != 2 ) fHitOccLimit = fNEvtsPerRun.at(runID)*fHitLimitPerWirePerEventInd;


	  if( binContent > fHitOccLimit ){
	    if( fVerbose )
	      std::cout << "Not filling occupancy distributions with this wire - it's waaaay too big of an outlier on the high side." << std::endl;
	  }
	  
	  //Also, if the bin content is zero, then don't push it back into the distribution. We know that 
	  //this is a dead wire.
	  else if( binContent == 0 ){
	    if( fVerbose )
	      std::cout << "Not filling occupancy distributions with this wire - it's waaaay too big of an outlier on the low side." << std::endl;
	  }	    
	 
	  else{
	    fRunToCryTPCToPlaneMapDist.at(runID).at(CryTPCPair).at(iPlane)->Fill(binContent);
	  }
	}
      }
    }
  }
}


//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//This function fits the occupancy distribution histograms to gaussians and 
//extracts fit parameters to use in good hit wire cuts.
void GoodWireAna::fitHitOccDistHists( std::vector<std::vector<size_t> > & badWireList, std::vector<std::vector<size_t> > & goodWireList )
{
  //Loop through each run
  for( std::map<int,std::map<std::pair<size_t,size_t>,std::vector<TH1D*> > >::iterator iter = fRunToCryTPCToPlaneMapDist.begin(); iter != fRunToCryTPCToPlaneMapDist.end(); ++iter ){
 
    //Basic info
    int runID = iter->first;

    //Loop over the Cry/TPCs
    for( std::map<std::pair<size_t,size_t>,std::vector<TH1D*> >::iterator iterCryTPC = iter->second.begin(); iterCryTPC != iter->second.end(); ++iterCryTPC ){

      //Get the TPC/Plane pair
      std::pair<size_t,size_t> CryTPCPair = iterCryTPC->first;

      //Get the plane wire occupancy histograms corresponding to this pair
      std::vector<TH1D*> planeHistos = iterCryTPC->second;

      //Loop over these histograms
      for( size_t iPlane = 0; iPlane < planeHistos.size(); ++iPlane ){
	
	//Get the histogram for this plane and loop over its bins
	TH1D * thisPlaneHist = planeHistos.at(iPlane);

	//If this histogram is unfilled, note that all wires are dead, and then 
	//continue. Otherwise we get segfaults
	TH1D * occupancyHist = fRunToCryTPCToPlaneMap.at(runID).at(CryTPCPair).at(iPlane);
	if( thisPlaneHist->Integral() == 0 ){
	  for( int iBin = 1; iBin <= occupancyHist->GetNbinsX(); ++iBin ){
	    if( fVerbose )
	      std::cout << "Adding badwirevect to completely dead plane." << std::endl;
	    std::vector<size_t> theBadWire;
	    
	    //Fill the bad wire vector with information about the wire (pointing to it)
	    theBadWire.push_back(runID);
	    theBadWire.push_back(CryTPCPair.first);
	    theBadWire.push_back(CryTPCPair.second);
	    theBadWire.push_back(iPlane);
	    theBadWire.push_back(iBin);
	    theBadWire.push_back(0); //Tag wire as noisy
	    
	    //Now push the bad wire back into the bad wire vect
	    badWireList.push_back(theBadWire);
	  }	  
	  continue;
	  

	}
	//Define fitting parameters and provide initial estimates
	//based on the plane type
	double gaus_mean = 0;
	double gaus_sigma = 0;
	//	double gaus_amplitude = 0;

	/*
	if( iPlane == 2 ){
	  gaus_mean = fInitGausMeanCol;
	  gaus_sigma = fInitGauSigmaCol;
	  gaus_amp = fInitGausAmpCol;
	}
	else{ 
	  gaus_mean = fInitGausMeanInd;
	  gaus_sigma = fInitGausSigmaInd;
	  gaus_amp = fInitGausAmpInd;
	}
	
	TF1 f1("f1","gaus");
	f1->SetParameters( gaus_amp, gaus_mean, gaus_sigma );
	*/
	
	//Fit the histogram
	thisPlaneHist->Fit("gaus","0");
	TF1* fit1 = (TF1*)thisPlaneHist->GetFunction("gaus");

      	
	//Get the parameters
	gaus_mean = fit1->GetParameter(1);
	gaus_sigma = fit1->GetParameter(2);
	//	gaus_amplitude = fit1->GetParameter(0);
	
	//Print parameters
	if( fVerbose )
	  std::cout << "Fit Parameters, r" << runID << "tpc" << CryTPCPair.second << "plane" << iPlane << ": mean: " << gaus_mean << ", sigma: " << gaus_sigma << std::endl;

	
	//Now loop through the hit occupancy histogram and find wires that
	//are outside of the mean + n sigma. Get the appropriate histo first.
	for( int iBin = 1; iBin <= occupancyHist->GetNbinsX(); ++iBin ){

	  if( fVerbose )
	    std::cout << "Bin #" << iBin << ", occupancy: " << occupancyHist->GetBinContent(iBin) << std::endl;

	  //Setting hit occupancy reasonable limits as before
	  if( iPlane == 2 ) fHitOccLimit = fNEvtsPerRun.at(runID)*fHitLimitPerWirePerEventCol;
	  if( iPlane != 2 ) fHitOccLimit = fNEvtsPerRun.at(runID)*fHitLimitPerWirePerEventInd;
	  if( occupancyHist->GetBinContent(iBin) > fHitOccLimit ){
	    if( fVerbose )
	      std::cout << "Adding badwirevect." << std::endl;
	    std::vector<size_t> theBadWire;

	    //Fill the bad wire vector with information about the wire (pointing to it)
	    theBadWire.push_back(runID);
	    theBadWire.push_back(CryTPCPair.first);
	    theBadWire.push_back(CryTPCPair.second);
	    theBadWire.push_back(iPlane);
	    theBadWire.push_back(iBin);
	    theBadWire.push_back(2); //Tag wire as noisy

	    //Now push the bad wire back into the bad wire vect
	    badWireList.push_back(theBadWire);
	    
	  }
	  
	  //Also, if the bin content is zero, then don't push it back into the distribution. We know that 
	  //this is a dead wire.
	  else if( occupancyHist->GetBinContent(iBin) == 0 ){
	    if( fVerbose )
	      std::cout << "Adding badwirevect." << std::endl;
	    std::vector<size_t> theBadWire;

	    //Fill the bad wire vector with information about the wire (pointing to it)
	    theBadWire.push_back(runID);
	    theBadWire.push_back(CryTPCPair.first);
	    theBadWire.push_back(CryTPCPair.second);
	    theBadWire.push_back(iPlane);
	    theBadWire.push_back(iBin);
	    theBadWire.push_back(0); //Tag wire as dead
	    
	    //Now push the bad wire back into the bad wire vect
	    badWireList.push_back(theBadWire);
	  }	    

	  else if( fabs(float(occupancyHist->GetBinContent(iBin))-(gaus_mean)) > gaus_sigma*fNSigmaGoodWire ){
	    if( fVerbose )
	      std::cout << "Adding badwirevect." << std::endl;
	    std::vector<size_t> theBadWire;
	    
	    //Fill the bad wire vector with information about the wire (pointing to it)
	    theBadWire.push_back(runID);
	    theBadWire.push_back(CryTPCPair.first);
	    theBadWire.push_back(CryTPCPair.second);
	    theBadWire.push_back(iPlane);
	    theBadWire.push_back(iBin);

	    //Now tag wires as either being dead (0), quiet (1) or noisy (2)
	    if( float(occupancyHist->GetBinContent(iBin)) < gaus_mean ){theBadWire.push_back(1); }
	    else{ theBadWire.push_back(2); }

	    badWireList.push_back(theBadWire);
	  }
	  else{
	    std::vector<size_t> theGoodWire;
	    
	    //Fill the good wire vector with information about the wire (pointing to it)
	    theGoodWire.push_back(runID);
	    theGoodWire.push_back(CryTPCPair.first);
	    theGoodWire.push_back(CryTPCPair.second);
	    theGoodWire.push_back(iPlane);
	    theGoodWire.push_back(iBin);

	    goodWireList.push_back(theGoodWire);

	    if( fVerbose )
	      std::cout << "GoodWire run/TPC/Plane/Bin: " << runID << "/" << CryTPCPair.second << "/" << iPlane << "/" << iBin << std::endl;
	  }
	}
      }
    }
  }	 
}


//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//This takes the bad wire info and writes it to a text file
void GoodWireAna::writeListOfBadWires(std::vector<std::vector<size_t> > badWireVect )
{
  //Create an ofstream object (outfile)
  ofstream outfile;
  outfile.open("badChannelList.txt");

  //Create vectors for listing bulk properties of different runs
  std::map<size_t,std::vector<size_t> > runToTPCVectMapNoisy;
  std::map<size_t,std::vector<size_t> > runToTPCVectMapQuiet;
  std::map<size_t,std::vector<size_t> > runToTPCVectMapDead;
  
  //Create blank initialization vectors
  std::vector<size_t> tempVect;

  //Initialize these vectors to the standard of Alex
  size_t nTPCs = fGeometry->NTPC(0);
  for( size_t iQ = 0; iQ < nTPCs ; ++iQ ){
    tempVect.push_back(0);
  }

  //Now loop through the wire vector
  if( fVerbose )
    std::cout << "Size of badwirevect: " << badWireVect.size() << std::endl;
  for( size_t iWire = 0; iWire < badWireVect.size(); ++iWire ){
    
    if( fOnlyWriteCollectionPlane ){
      if( badWireVect.at(iWire).at(3) != 2 ) continue;
    }

    //Identify the run
    size_t runID = badWireVect.at(iWire).at(0);
    
    //If the run has not already been seen, emplace maps back with empty vectors
    if( runToTPCVectMapNoisy.count(runID) == 0 ){
      runToTPCVectMapNoisy.emplace(runID,tempVect);
      runToTPCVectMapQuiet.emplace(runID,tempVect);
      runToTPCVectMapDead.emplace(runID,tempVect);
    }
    
    //Now increment each vector in the appropriate spot corresp to this TPC and type of bad wire
    size_t type = badWireVect.at(iWire).at(5);
    size_t tpc = badWireVect.at(iWire).at(2);
    if( type == 0 ){
      runToTPCVectMapDead.at(runID).at(tpc)++;
    }
    if( type == 1 ){
      runToTPCVectMapQuiet.at(runID).at(tpc)++;
    }
    if( type == 2 ){
      runToTPCVectMapNoisy.at(runID).at(tpc)++;
    }

    /*
    //Specify group in Alex notation
    int group = 0;
    if( badWireVect.at(iWire).at(3) % 2 == 1 ) group = (badWireVect.at(iWire).at(2)-1)/2;
    else group = badWireVect.at(iWire).at(2)/2;
    
    //
    if( badWireVect.at(iWire).at(5) == 0 ) numDeadVect.at(group)++;
    if( badWireVect.at(iWire).at(5) == 1 ) numQuietVect.at(group)++;
    if( badWireVect.at(iWire).at(5) == 2 ) numNoisyVect.at(group)++;
    */


    //Get the channel from the Cryo, TPC, Plane, and Wire
    geo::WireID theWireID( badWireVect.at(iWire).at(1), badWireVect.at(iWire).at(2), badWireVect.at(iWire).at(3),badWireVect.at(iWire).at(4)-1);
    size_t channel = fGeometry->PlaneWireToChannel(theWireID);

    if( fVerbose )
      std::cout << "Cryo/TPC/Plane/Wire/Channel: " << badWireVect.at(iWire).at(1) << "/"  << badWireVect.at(iWire).at(2) << "/" << badWireVect.at(iWire).at(3) << "/" << badWireVect.at(iWire).at(4)-1 << "/" << channel  << std::endl;
   

    outfile << badWireVect.at(iWire).at(0) << " " << channel << " " << badWireVect.at(iWire).at(5) << "\n";


    /*
    outfile << badWireVect.at(iWire).at(0) << " " << badWireVect.at(iWire).at(1) << " " 
	    << badWireVect.at(iWire).at(2) << " " << badWireVect.at(iWire).at(3) << " "
	    << badWireVect.at(iWire).at(4)-1 << " " << badWireVect.at(iWire).at(5) << "\n";
    */
  }
  
 
  outfile.close();

  //  if( fSummary ){
  std::cout << "********************************************************" << std::endl;
  std::cout << "*************** JOB SUMMARY OF BAD WIRES ***************" << std::endl;
  std::cout << "********************************************************" << std::endl;
  
  std::cout << "Note: Collection plane only at the moment." << std::endl;
  
  //Loop over all runs
  for( std::map<size_t,std::vector<size_t> >::iterator runIter = runToTPCVectMapNoisy.begin(); runIter != runToTPCVectMapNoisy.end(); ++runIter ){
    size_t runID = runIter->first;
    std::cout << "\n **** RUN #" << runID << " SUMMARY **** " << std::endl;
        
    //Now loop over all TPCs in a given run
    for( size_t iTPC = 0; iTPC < nTPCs; ++iTPC ){
      std::cout << "Number of noisy wires in TPC #" << iTPC << ": " << runToTPCVectMapNoisy.at(runID).at(iTPC) << std::endl;
      std::cout << "Number of quiet wires in TPC #" << iTPC << ": " << runToTPCVectMapQuiet.at(runID).at(iTPC) << std::endl;
      std::cout << "Number of dead wires in TPC #" << iTPC << ":  " << runToTPCVectMapDead.at(runID).at(iTPC) << std::endl;
    }
    
  }
  /*
    for(size_t iQ = 0; iQ < 4 ; ++iQ ){
    std::cout << "Quarter: " << iQ << ", NumDead/Quiet/Noisy: " << numDeadVect.at(iQ) << "/" << numQuietVect.at(iQ) << "/" << numNoisyVect.at(iQ) << std::endl;
    }
  */
  //}
  
}



//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//This takes the good wire info and writes it to a text file
void GoodWireAna::writeListOfGoodWires(std::vector<std::vector<size_t> > goodWireVect )
{
  //Create an ofstream object (outfile)
  ofstream outfile;
  outfile.open("goodChannelList.txt");

  //Create an object for listing bulk properties
  size_t numGood = 0;
  
  //Now loop through the vector
  if( fVerbose )
    std::cout << "Size of goodwirevect: " << goodWireVect.size() << std::endl;
  for( size_t iWire = 0; iWire < goodWireVect.size(); ++iWire ){
    
    if( fOnlyWriteCollectionPlane ){
      if( goodWireVect.at(iWire).at(3) != 2 ) continue;
    }
    
    //Increment but restrict to collection planes
    numGood++;

    //Get the channel from the Cryo, TPC, Plane, and Wire
    geo::WireID theWireID( goodWireVect.at(iWire).at(1), goodWireVect.at(iWire).at(2), goodWireVect.at(iWire).at(3),goodWireVect.at(iWire).at(4)-1);
    size_t channel = fGeometry->PlaneWireToChannel(theWireID);

    if( fVerbose )
      std::cout << "Cryo/TPC/Plane/Wire/Channel: " << goodWireVect.at(iWire).at(1) << "/"  << goodWireVect.at(iWire).at(2) << "/" << goodWireVect.at(iWire).at(3) << "/" << goodWireVect.at(iWire).at(4)-1 << "/" << channel  << std::endl;
   

    outfile << goodWireVect.at(iWire).at(0) << " " << channel << std::endl;

  }
  
 
  outfile.close();

  if( fVerbose )
    std::cout << "NumGood: " << numGood << std::endl;

}


//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
void GoodWireAna::beginJob()
{
  /*
  //Setting geometry info
  art::ServiceHandle<geo::Geometry>            geometry;
  fGeometry = &*geometry;
  fNCry = fGeometry->Ncryostats();
 
  //TFile Service
  art::ServiceHandle<art::TFileService> tfs;
  fTFS = &*tfs;

  //Geometry Helper
  art::ServiceHandle<dune::DUNEGeometryHelper> geometryHelper;
  */
  
}

void GoodWireAna::beginRun(art::Run const & r)
{
  // Implementation of optional member function here.
}

void GoodWireAna::beginSubRun(art::SubRun const & sr)
{
  // Implementation of optional member function here.
}


//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//..........ooooooooooooooooooo00000OOOOOOOOO00000oooooooooooooooooooo............
//Now we want to extract information from these completed histograms and write to
//a simple text file (for now, that is).
void GoodWireAna::endJob()
{
  if( fVerbose )
    std::cout << "EndJob reached." << std::endl;
  
  //Fill Occupancy Distribution histograms
  std::vector<std::vector<size_t> > badWireVect;
  std::vector<std::vector<size_t> > goodWireVect;
  if( fVerbose )
    std::cout << "Filling." << std::endl;
  fillHitOccDistHists(badWireVect);

  //Fit those histograms and return a set of bad hits
  if( fVerbose )
    std::cout << "Fitting." << std::endl;
  fitHitOccDistHists(badWireVect,goodWireVect);
  
  //Bad Wire Handling
  if( fVerbose )
    std::cout << "Writing Bad." << std::endl;
  writeListOfBadWires(badWireVect);

  //Good Wire Handling
  if( fVerbose )
    std::cout << "Writing Good." << std::endl;
  writeListOfGoodWires(goodWireVect);

  //Dead wire handling (wires with zero signal)
  //  writeListOfDeadWires();

  //Bad wire handling (wires with noisy signal)
  //  writeListOfNoisyWires();
  
  

}

void GoodWireAna::endRun(art::Run const & r)
{
  // Implementation of optional member function here.
}

void GoodWireAna::endSubRun(art::SubRun const & sr)
{
  // Implementation of optional member function here.
}

void GoodWireAna::reconfigure(fhicl::ParameterSet const & pset)
{
  // Implementation of optional member function here.
  fHitModuleLabel = pset.get<std::string>("HitModuleLabel","fasthit");
  fNSigmaGoodWire = pset.get<size_t>("NSigmaGoodWire",3);
  fHitLimitPerWirePerEventCol = pset.get<size_t>("HitLimitPerWirePerEventCol",10);
  fHitLimitPerWirePerEventInd = pset.get<size_t>("HitLimitPerWirePerEventInd",1);
  fVerbose = pset.get<bool>("Verbosity",false);
  fOnlyWriteCollectionPlane = pset.get<bool>("OnlyWriteCollectionPlane",true);


  fHitOccLimit = 0;

}

void GoodWireAna::respondToCloseInputFile(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

void GoodWireAna::respondToCloseOutputFiles(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

void GoodWireAna::respondToOpenInputFile(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

void GoodWireAna::respondToOpenOutputFiles(art::FileBlock const & fb)
{
  // Implementation of optional member function here.
}

DEFINE_ART_MODULE(GoodWireAna)
