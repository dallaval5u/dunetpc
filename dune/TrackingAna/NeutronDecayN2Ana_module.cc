////////////////////////////////////////////////////////////////////////
// Class:       NeutronDecayN2Ana
// Module Type: analyzer
// File:        NeutronDecayN2Ana_module.cc
//
// Generated at Sun Mar 24 09:05:02 2013 by Tingjun Yang using artmod
// from art v1_02_06.
//
//  Having a look at FD reconstruction quantities
//
//  Thomas Karl Warburton
//  k.warburton@sheffield.ac.uk
//
////////////////////////////////////////////////////////////////////////

// Framework includes
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h" 
#include "art/Framework/Principal/Event.h" 
#include "art/Framework/Principal/Handle.h" 
#include "fhiclcpp/ParameterSet.h" 
#include "art/Framework/Services/Optional/TFileService.h" 
#include "messagefacility/MessageLogger/MessageLogger.h" 

// LArSoft includes
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/Track.h"
#include "larsim/MCCheater/BackTracker.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "nusimdata/SimulationBase/MCTruth.h"

// ROOT includes
#include "TTree.h"

//standard library includes
#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <memory>
#include <limits> // std::numeric_limits<>

struct IDEYLess {
  bool operator()(const sim::IDE& first, const sim::IDE& second) {
    return first.y < second.y;
  }
};

#define MaxPart   100
#define MaxParent 100
#define HolderVal 9999

namespace NeutronDecayN2Ana {
  class NeutronDecayN2Ana;
}

class NeutronDecayN2Ana::NeutronDecayN2Ana : public art::EDAnalyzer {
public:
  explicit NeutronDecayN2Ana(fhicl::ParameterSet const & p);
  virtual ~NeutronDecayN2Ana();

  void analyze(art::Event const & e) override;

  void beginRun(art::Run& run);
  void beginJob() override;
  void endJob();
  void endRun();
  //void reconfigure(fhicl::ParameterSet const & p) override;
  
private:
  // ------ My functions ------
  void ResetVars();
  void FillVars( std::vector<int> &TrackIDVec, int &numParts, double EDep[MaxPart], double DaughtEDep[MaxPart], double DecayEDep[MaxPart], double Start[MaxPart][4], double End[MaxPart][4],
		 int nParents[MaxPart], int Parent[MaxPart][MaxParent], int PDG[MaxPart],
		 int ThisID, unsigned int ThisTDC, sim::IDE ThisIDE, const simb::MCParticle& MCPart, bool Decay, bool &Written );
 
  // Handles
  art::ServiceHandle<geo::Geometry> geom;
  art::ServiceHandle<cheat::BackTracker> bktrk;

  // Parameter List
  std::string fHitsModuleLabel;
  std::string fTrackModuleLabel;
  std::string fMCTruthT0ModuleLabel;

  double ActiveBounds[6]; // Cryostat boundaries ( neg x, pos x, neg y, pos y, neg z, pos z )

  std::map<int, const simb::MCParticle*> truthmap; // A map of the truth particles.
  std::vector<int> AllTrackIDs; // A vector of all of my stored TrackIDs
  int NEvent;

  TTree* fDecayTree;
  int Run;
  int Event;
  double PrimMuonRange;
  double PrimMuonEDep;
  double PrimMuonShadowEDep;
  double DistEdge[3][2];
  double TotalEDep;

  // NumParts
  int nMuon, nPion, nPi0, nKaon, nElec;
  // PdgCode
  int MuonPDG[MaxPart], PionPDG[MaxPart], Pi0PDG[MaxPart], KaonPDG[MaxPart], ElecPDG[MaxPart];
  // NumParents
  int nParentMuon[MaxPart], nParentPion[MaxPart], nParentPi0[MaxPart], nParentKaon[MaxPart], nParentElec[MaxPart];
  // NumParents
  int MuonParents[MaxPart][MaxParent], PionParents[MaxPart][MaxParent], Pi0Parents[MaxPart][MaxParent], KaonParents[MaxPart][MaxParent], ElecParents[MaxPart][MaxParent];
  // EDep
  double MuonEDep[MaxPart], PionEDep[MaxPart], Pi0EDep[MaxPart], KaonEDep[MaxPart], ElecEDep[MaxPart];
  // Daughter EDep
  double MuonDaughtersEDep[MaxPart], PionDaughtersEDep[MaxPart], Pi0DaughtersEDep[MaxPart], KaonDaughtersEDep[MaxPart], ElecDaughtersEDep[MaxPart];
  // Decay EDep
  double MuonDecayEDep[MaxPart], PionDecayEDep[MaxPart], Pi0DecayEDep[MaxPart], KaonDecayEDep[MaxPart], ElecDecayEDep[MaxPart];
  // Start
  double MuonStart[MaxPart][4], PionStart[MaxPart][4], Pi0Start[MaxPart][4], KaonStart[MaxPart][4], ElecStart[MaxPart][4];
  // End
  double MuonEnd[MaxPart][4]  , PionEnd[MaxPart][4]  , Pi0End[MaxPart][4]  , KaonEnd[MaxPart][4]  , ElecEnd[MaxPart][4];
};
// ******************************** Reset Variables *****************************************************
void NeutronDecayN2Ana::NeutronDecayN2Ana::ResetVars() {
  Run = Event = 0;
  PrimMuonRange = PrimMuonEDep = PrimMuonShadowEDep = TotalEDep = 0;
  // DistEdge
  for (int i=0; i<3; i++)
    for (int j=0; j<2; j++)
      DistEdge[i][j]=HolderVal;
  // Each particle type
  nMuon = nPion = nPi0 = nKaon = nElec = 0;
  for (int i=0; i<MaxPart; ++i) {
    MuonPDG[i]     = PionPDG[i]     = Pi0PDG[i]     = KaonPDG[i]     = ElecPDG[i]     = 0;
    MuonEDep[i]    = PionEDep[i]    = Pi0EDep[i]    = KaonEDep[i]    = ElecEDep[i]    = 0;
    nParentMuon[i] = nParentPion[i] = nParentPi0[i] = nParentKaon[i] = nParentElec[i] = 0;
    for (int j=0; j<MaxParent; ++j) {
      MuonParents[i][j] = PionParents[i][j] = Pi0Parents[i][j] = KaonParents[i][j] = ElecParents[i][j] = 0;
    }
    MuonDaughtersEDep[i] = PionDaughtersEDep[i] = Pi0DaughtersEDep[i] = KaonDaughtersEDep[i] = ElecDaughtersEDep[i] = 0;
    MuonDecayEDep[i]     = PionDecayEDep[i]     = Pi0DecayEDep[i]     = KaonDecayEDep[i]     = ElecDecayEDep[i]     = 0;
    for (int j=0; j<4; ++j) {
      MuonStart[i][j] = PionStart[i][j] = Pi0Start[i][j] = KaonStart[i][j] = ElecStart[i][j] = HolderVal;
      MuonEnd[i][j]   = PionEnd[i][j]   = Pi0End[i][j]   = KaonEnd[i][j]   = ElecEnd[i][j]   = HolderVal;
    }

  }
}
// ********************************** Begin Run *******************************************************
void NeutronDecayN2Ana::NeutronDecayN2Ana::beginRun(art::Run& run) {
  NEvent = 0;
}
// *********************************** Begin Job ********************************************************
void NeutronDecayN2Ana::NeutronDecayN2Ana::beginJob()
{
  // Build my Cryostat boundaries array...Taken from Tyler Alion in Geometry Core. Should still return the same values for uBoone.
  ActiveBounds[0] = ActiveBounds[2] = ActiveBounds[4] = DBL_MAX;
  ActiveBounds[1] = ActiveBounds[3] = ActiveBounds[5] = -DBL_MAX;

  // ----- FixMe: Assume single cryostats ------
  auto const* geom = lar::providerFrom<geo::Geometry>();
  for (geo::TPCGeo const& TPC: geom->IterateTPCs()) {
    // get center in world coordinates
    const double origin[3] = {0.};
    double center[3] = {0.};
    TPC.LocalToWorld(origin, center);
    //double tpcDim[3] = {TPC.ActiveHalfWidth(), TPC.ActiveHalfHeight(), 0.5*TPC.ActiveLength() };
    double tpcDim[3] = {TPC.HalfWidth(), TPC.HalfHeight(), 0.5*TPC.Length() }; // Gives same result as what Matt has
    if( center[0] - tpcDim[0] < ActiveBounds[0] ) ActiveBounds[0] = center[0] - tpcDim[0];
    if( center[0] + tpcDim[0] > ActiveBounds[1] ) ActiveBounds[1] = center[0] + tpcDim[0];
    if( center[1] - tpcDim[1] < ActiveBounds[2] ) ActiveBounds[2] = center[1] - tpcDim[1];
    if( center[1] + tpcDim[1] > ActiveBounds[3] ) ActiveBounds[3] = center[1] + tpcDim[1];
    if( center[2] - tpcDim[2] < ActiveBounds[4] ) ActiveBounds[4] = center[2] - tpcDim[2];
    if( center[2] + tpcDim[2] > ActiveBounds[5] ) ActiveBounds[5] = center[2] + tpcDim[2];
  } // for all TPC
  std::cout << "Active Boundaries: "
	    << "\n\tx: " << ActiveBounds[0] << " to " << ActiveBounds[1]
	    << "\n\ty: " << ActiveBounds[2] << " to " << ActiveBounds[3]
	    << "\n\tz: " << ActiveBounds[4] << " to " << ActiveBounds[5]
	    << std::endl;

  // Implementation of optional member function here.
  art::ServiceHandle<art::TFileService> tfs;
  fDecayTree = tfs->make<TTree>("ReconstructedTree","analysis tree");

  fDecayTree->Branch("Run"               ,&Run               ,"Run/I"               );
  fDecayTree->Branch("Event"             ,&Event             ,"Event/I"             );
  fDecayTree->Branch("PrimMuonRange"     ,&PrimMuonRange     ,"PrimMuonRange/D"     );
  fDecayTree->Branch("PrimMuonEDep"      ,&PrimMuonEDep      ,"PrimMuonEDep/D"      );
  fDecayTree->Branch("PrimMuonShadowEDep",&PrimMuonShadowEDep,"PrimMuonShadowEDep/D");
  fDecayTree->Branch("DistEdge"          ,&DistEdge          ,"DistEdge[3][2]/D"    );
  fDecayTree->Branch("TotalEDep"         ,&TotalEDep         ,"TotalEDep/D"         );

  // Muon
  fDecayTree->Branch("nMuon"            ,&nMuon            ,"nMuon/I"                   );
  fDecayTree->Branch("nParentMuon"      ,&nParentMuon      ,"nParentMuon[nMuon]/I"      );
  fDecayTree->Branch("MuonParents"      ,&MuonParents      ,"MuonParents[nMuon][100]/I" );
  fDecayTree->Branch("MuonPDG"          ,&MuonPDG          ,"MuonPDG[nMuon]/I"          );
  fDecayTree->Branch("MuonEDep"         ,&MuonEDep         ,"MuonEDep[nMuon]/D"         );
  fDecayTree->Branch("MuonStart"        ,&MuonStart        ,"MuonStart[nMuon][4]/D"     );
  fDecayTree->Branch("MuonEnd"          ,&MuonEnd          ,"MuonEnd[nMuon][4]/D"       );
  fDecayTree->Branch("MuonDaughtersEDep",&MuonDaughtersEDep,"MuonDaughtersEDep[nMuon]/D");
  fDecayTree->Branch("MuonDecayEDep"    ,&MuonDecayEDep    ,"MuonDecayEDep[nMuon]/D"    );
  // Pion
  fDecayTree->Branch("nPion"            ,&nPion            ,"nPion/I"                   );
  fDecayTree->Branch("nParentPion"      ,&nParentPion      ,"nParentPion[nPion]/I"      );
  fDecayTree->Branch("PionParents"      ,&PionParents      ,"PionParents[nPion][100]/I" );
  fDecayTree->Branch("PionPDG"          ,&PionPDG          ,"PionPDG[nPion]/I"          );
  fDecayTree->Branch("PionEDep"         ,&PionEDep         ,"PionEDep[nPion]/D"         );
  fDecayTree->Branch("PionStart"        ,&PionStart        ,"PionStart[nPion][4]/D"     );
  fDecayTree->Branch("PionEnd"          ,&PionEnd          ,"PionEnd[nPion][4]/D"       );
  fDecayTree->Branch("PionDaughtersEDep",&PionDaughtersEDep,"PionDaughtersEDep[nPion]/D");
  fDecayTree->Branch("PionDecayEDep"    ,&PionDecayEDep    ,"PionDecayEDep[nPion]/D"    );
  // Pi0
  fDecayTree->Branch("nPi0"            ,&nPi0            ,"nPi0/I"                  );
  fDecayTree->Branch("nParentPi0"      ,&nParentPi0      ,"nParentPi0[nPi0]/I"      );
  fDecayTree->Branch("Pi0Parents"      ,&Pi0Parents      ,"Pi0Parents[nPi0][100]/I" );
  fDecayTree->Branch("Pi0PDG"          ,&Pi0PDG          ,"Pi0PDG[nPi0]/I"          );
  fDecayTree->Branch("Pi0EDep"         ,&Pi0EDep         ,"Pi0EDep[nPi0]/D"         );
  fDecayTree->Branch("Pi0Start"        ,&Pi0Start        ,"Pi0Start[nPi0][4]/D"     );
  fDecayTree->Branch("Pi0End"          ,&Pi0End          ,"Pi0End[nPi0][4]/D"       );
  fDecayTree->Branch("Pi0DaughtersEDep",&Pi0DaughtersEDep,"Pi0DaughtersEDep[nPi0]/D");
  fDecayTree->Branch("Pi0DecayEDep"    ,&Pi0DecayEDep    ,"Pi0DecayEDep[nPi0]/D"    );
  // Kaon
  fDecayTree->Branch("nKaon"            ,&nKaon            ,"nKaon/I"                   );
  fDecayTree->Branch("nParentKaon"      ,&nParentKaon      ,"nParentKaon[nKaon]/I"      );
  fDecayTree->Branch("KaonParents"      ,&KaonParents      ,"KaonParents[nKaon][100]/I" );
  fDecayTree->Branch("KaonPDG"          ,&KaonPDG          ,"KaonPDG[nKaon]/I"          );
  fDecayTree->Branch("KaonEDep"         ,&KaonEDep         ,"KaonEDep[nKaon]/D"         );
  fDecayTree->Branch("KaonStart"        ,&KaonStart        ,"KaonStart[nKaon][4]/D"     );
  fDecayTree->Branch("KaonEnd"          ,&KaonEnd          ,"KaonEnd[nKaon][4]/D"       );
  fDecayTree->Branch("KaonDaughtersEDep",&KaonDaughtersEDep,"KaonDaughtersEDep[nKaon]/D");
  fDecayTree->Branch("KaonDecayEDep"    ,&KaonDecayEDep    ,"KaonDecayEDep[nKaon]/D"    );
  // Electron
  fDecayTree->Branch("nElec"            ,&nElec            ,"nElec/I"                   );
  fDecayTree->Branch("nParentElec"      ,&nParentElec      ,"nParentElec[nElec]/I"      );
  fDecayTree->Branch("ElecParents"      ,&ElecParents      ,"ElecParents[nElec][100]/I" );
  fDecayTree->Branch("ElecPDG"          ,&ElecPDG          ,"ElecPDG[nElec]/I"          );
  fDecayTree->Branch("ElecEDep"         ,&ElecEDep         ,"ElecEDep[nElec]/D"         );
  fDecayTree->Branch("ElecStart"        ,&ElecStart        ,"ElecStart[nElec][4]/D"     );
  fDecayTree->Branch("ElecEnd"          ,&ElecEnd          ,"ElecEnd[nElec][4]/D"       );
  fDecayTree->Branch("ElecDaughtersEDep",&ElecDaughtersEDep,"ElecDaughtersEDep[nElec]/D");
  fDecayTree->Branch("ElecDecayEDep"    ,&ElecDecayEDep    ,"ElecDecayEDep[nElec]/D"    );
    
}
// ************************************ End Job *********************************************************
void NeutronDecayN2Ana::NeutronDecayN2Ana::endJob() {
  
}
// ************************************ End Run *********************************************************
void NeutronDecayN2Ana::NeutronDecayN2Ana::endRun() {
}

// ********************************** pset param *******************************************************
NeutronDecayN2Ana::NeutronDecayN2Ana::NeutronDecayN2Ana(fhicl::ParameterSet const & pset)
  : EDAnalyzer(pset)
  , fHitsModuleLabel         ( pset.get< std::string >("HitsModuleLabel"))
  , fTrackModuleLabel        ( pset.get< std::string >("TrackModuleLabel"))
  , fMCTruthT0ModuleLabel    ( pset.get< std::string >("MCTruthT0ModuleLabel"))
{

}
// ******************************************************************************************************
NeutronDecayN2Ana::NeutronDecayN2Ana::~NeutronDecayN2Ana()
{
  // Clean up dynamic memory and other resources here.

}
// ************************************ Analyse *********************************************************
void NeutronDecayN2Ana::NeutronDecayN2Ana::analyze(art::Event const & evt)
{
  ++NEvent;
  ResetVars();
  Run   = evt.run();
  Event = evt.event();
  std::cout << "\n\n************* New Event / Module running - Run " << Run << ", Event " << Event << ", NEvent " << NEvent << " *************\n\n" << std::endl;

  // Any providers I need.
  auto const* geo = lar::providerFrom<geo::Geometry>();

  // Implementation of required member function here. 
  art::Handle< std::vector<recob::Track> > trackListHandle;
  std::vector<art::Ptr<recob::Track> > tracklist;
  if (evt.getByLabel(fTrackModuleLabel,trackListHandle))
    art::fill_ptr_vector(tracklist, trackListHandle);
  
  // Make a map of MCParticles which I can access later.
  art::Handle<std::vector<simb::MCParticle> > truth;
  evt.getByLabel("largeant", truth);
  truthmap.clear();
  for (size_t i=0; i<truth->size(); i++)
    truthmap[truth->at(i).TrackId()]=&((*truth)[i]);

  // Get a vector of sim channels.
  art::Handle<std::vector<sim::SimChannel> > simchannels;
  evt.getByLabel("largeant", simchannels);
  
  // Make a vector for primary IDEs to work out total Edep and length of primary muon
  std::vector<sim::IDE> priides;

  // Make vectors to hold all of my particle TrackIDs
  std::vector<int> MuonVec, PionVec, Pi0Vec, KaonVec, ElecVec;
  AllTrackIDs.clear();

  // Want to loop through the simchannels, and the ides.
  int chanIt = 0;
  
  for (auto const& simchannel:*simchannels) {
    // ------ Only want to look at collection plane hits ------
    if (geo->SignalType(simchannel.Channel()) != geo::kCollection) continue;
    int tdcideIt = 0;
    // ------ Loop through all the IDEs for this channel ------
    for (auto const& tdcide:simchannel.TDCIDEMap()) {
      unsigned int tdc = tdcide.first;
      auto const& idevec=tdcide.second;
      //std::cout << "TDC IDE " << tdcideIt << " of " << simchannel.TDCIDEMap().size() << ", has tdc " << tdc << ", and idevec of size " << idevec.size() << std::endl;
      int ideIt = 0;
      // ------ Look at each individual IDE ------
      for (auto const& ide:idevec) {
	int ideTrackID = ide.trackID;
	float ideEnergy = ide.energy;
	double idePos[3];
	idePos[0] = ide.x;
	idePos[1] = ide.y;
	idePos[2] = ide.z;
	/*
	if (ideTrackID == 1) { 
	float ideNumEl  = ide.numElectrons;
	std::cout << "      IDE " << ideIt << " of " << idevec.size() << " has TrackId " << ideTrackID << ", energy " << ideEnergy << ", NumEl " << ideNumEl << ", tdc " << tdc << ". ";
	std::cout << "Position " << idePos[0] << ", " << idePos[1] << ", " << idePos[2] << std::endl;
	}
	//*/

	// ------ Work out if this hit is in a TPC ------
	bool InTPC = false;
	if ( idePos[0] > ActiveBounds[0] && idePos[0] < ActiveBounds[1] )
	  if ( idePos[1] > ActiveBounds[2] && idePos[1] < ActiveBounds[3] )
	    if ( idePos[2] > ActiveBounds[4] && idePos[2] < ActiveBounds[5] )
	      InTPC = true;
	if (! InTPC ) {
	  std::cout << "Outside the Active volume I found at the top!" << std::endl;
	  continue;
	}	  

	// ------ Add the energy deposition from this IDE to the sum of IDEs
	TotalEDep += ideEnergy;
	if ( ideTrackID == -1 ) PrimMuonShadowEDep += ideEnergy;
	
	// ------ I want to make a vector of the IDEs for the primary muon ------
	if (ideTrackID==1) priides.push_back(ide);
	  
	// ------ I want to work the closest IDE to an edge of the active volume ------
	if ( DistEdge[0][0] > ( ide.x - ActiveBounds[0]) ) DistEdge[0][0] =  ide.x - ActiveBounds[0];
	if ( DistEdge[0][1] > (-ide.x + ActiveBounds[1]) ) DistEdge[0][1] = -ide.x + ActiveBounds[1];
	if ( DistEdge[1][0] > ( ide.y - ActiveBounds[2]) ) DistEdge[1][0] =  ide.y - ActiveBounds[2];
	if ( DistEdge[1][1] > (-ide.y + ActiveBounds[3]) ) DistEdge[1][1] = -ide.y + ActiveBounds[3];
	if ( DistEdge[2][0] > ( ide.z - ActiveBounds[4]) ) DistEdge[2][0] =  ide.z - ActiveBounds[4];
	if ( DistEdge[2][1] > (-ide.z + ActiveBounds[5]) ) DistEdge[2][1] = -ide.z + ActiveBounds[5];
		
	// ------ Now to work out which particles in particular I want to save more information about... ------
	// ----------------- I want to write out the IDE to the relevant parent particle type -----------------
	// ------- This means looping back through the IDE's parents until I find an interesting TrackID ------
	bool isDecay = false;
	bool WrittenOut = false;
	std::cout << "\nLooking at IDE " << ideIt << ", ideTrackID is " << ideTrackID << ", it was due to a " << truthmap[ abs(ideTrackID) ]->PdgCode() << ", process " << truthmap[ abs(ideTrackID) ]->Process() << std::endl;
	while ( ideTrackID != 0 && !WrittenOut ) {
	  //if ( ideTrackID < 0 ) continue; // I can't get the truth information for negative track IDs
	  const simb::MCParticle& part=*( truthmap[ abs(ideTrackID) ] );
	  int PdgCode=part.PdgCode();
	  // ========== Muons ==========
	  if      ( PdgCode == -13  || PdgCode == 13  ) FillVars( MuonVec, nMuon, MuonEDep, MuonDaughtersEDep, MuonDecayEDep, MuonStart, MuonEnd, nParentMuon, MuonParents, MuonPDG,
								  ideTrackID, tdc, ide, part, isDecay, WrittenOut  );
	  // ========== Pions ==========
	  else if ( PdgCode == -211 || PdgCode == 211 ) FillVars( PionVec, nPion, PionEDep, PionDaughtersEDep, PionDecayEDep, PionStart, PionEnd, nParentPion, PionParents, PionPDG,
								  ideTrackID, tdc, ide, part, isDecay, WrittenOut);
	  // ========== Pi0s  ==========
	  else if ( PdgCode == 111  )                   FillVars( Pi0Vec , nPi0 , Pi0EDep , Pi0DaughtersEDep , Pi0DecayEDep , Pi0Start , Pi0End , nParentPi0 , Pi0Parents , Pi0PDG ,
								  ideTrackID, tdc, ide, part, isDecay, WrittenOut );
	  // ========== Kaons ===========
	  else if ( PdgCode == 321 || PdgCode == -321 ) FillVars( KaonVec, nKaon, KaonEDep, KaonDaughtersEDep, KaonDecayEDep, KaonStart, KaonEnd, nParentKaon, KaonParents, KaonPDG,
								  ideTrackID, tdc, ide, part, isDecay, WrittenOut );
	  // ========== Elecs ===========
	  else if ( PdgCode == -11 || PdgCode == 11   ) FillVars( ElecVec, nElec, ElecEDep, ElecDaughtersEDep, ElecDecayEDep, ElecStart, ElecEnd, nParentElec, ElecParents, ElecPDG,
								  ideTrackID, tdc, ide, part, isDecay, WrittenOut );
	  // ========== If still not one of my intersting particles I need to find this particles parent ==========
	  else {
	    ideTrackID = part.Mother();
	    PdgCode = truthmap[ abs(ideTrackID) ]->PdgCode();
	    if ( part.Process() == "Decay" ) {
	      std::cout << "This particle was from a decay!" << std::endl;
	      isDecay = true;
	    }
	    std::cout << "Not something interesting so moving backwards. The parent of that particle had trackID " << ideTrackID << ", was from a " << PdgCode << ", from a decay? " << isDecay << std::endl;
	  } // If not one of chosen particles.
	} // While loop
	ideIt++;
      } // Each IDE ( ide:idevec )
      ++tdcideIt;
    } // IDE vector for SimChannel ( tdcide:simcahnnel.TPCIDEMap() )
    ++chanIt;
  } // Loop through simchannels
  
  if (nPion) {
    std::cout << "There are pions in this event!!" << std::endl;
  }
  if (nPi0) {
    std::cout << "There are pi0's in this event!!" << std::endl;
  }
  if (nKaon) {
    std::cout << "There kaons in this event!!" << std::endl;
  }
  if (nElec) {
    std::cout << "There are electrons in this event!!" << std::endl;
  }
  
  // Work out some properties of the primary muon
  if (priides.size()) {
    // ------ Sort the IDEs in y ------
    std::sort(priides.begin(), priides.end(), IDEYLess());
    // ------ Work out the range ------
    double dx=priides.front().x-priides.back().x;
    double dy=priides.front().y-priides.back().y;
    double dz=priides.front().z-priides.back().z;
    PrimMuonRange=sqrt(dx*dx+dy*dy+dz*dz);
    // ------ Work out the energy deposited ------
    for (unsigned int muIt=0; muIt<priides.size(); ++muIt) PrimMuonEDep += priides[muIt].energy;
    //PrimMuonEDep = PrimMuonEDep / priides.size();
  } 
  std::cout << "Primary muon track length = " << PrimMuonRange << " cm, and energy despoited = " << PrimMuonEDep << ", shadow EDep = " << PrimMuonShadowEDep << " and total Edep is " << TotalEDep << ".\n"
	    << "There were " << nMuon << " muons, " << nPion << " pions, " << nPi0 << " pi0s, " << nKaon << " Kaons, " << nElec << " Electrons.\n"
	    << std::endl;

  // If nMuon
  if (MuonStart[0][0] != HolderVal && priides.size()) {
    double MyRange = pow( pow((MuonStart[0][0]-MuonEnd[0][0]),2) + pow((MuonStart[0][1]-MuonEnd[0][1]),2) + pow((MuonStart[0][2]-MuonEnd[0][2]),2), 0.5 );
    std::cout << "My start " << MuonStart[0][0] << ", " << MuonStart[0][1] << ", " << MuonStart[0][2] << ", My end " << MuonEnd[0][0] << ", " << MuonEnd[0][1] << ", " << MuonEnd[0][2] << ". "
	      << "Distance of " << MyRange << ". Time range " << MuonStart[0][3] << ", " << MuonEnd[0][3] << ".\n"
	      << "Matt sta " << priides.front().x << ", " << priides.front().y << ", " << priides.front().z << ", Matt sta " << priides.back().x << ", " << priides.back().y << ", " << priides.back().z
	      << std::endl;
    
    if ( PrimMuonRange - MyRange > 2 ) {
      std::cout << "\n NOT MATCHING!! \n" << std::endl;
      for (size_t qq=0; qq<priides.size(); ++qq)
	std::cout << "PrimMuon start " << priides[qq].x << ", " << priides[qq].y << ", " << priides[qq].z << std::endl;
    }
  }
  // ------ Fill the Tree ------
  fDecayTree->Fill();
} // Analyse
// ******************************** Fill variables *****************************************************
void NeutronDecayN2Ana::NeutronDecayN2Ana::FillVars( std::vector<int> &TrackIDVec, int &numParts, double EDep[MaxPart], double DaughtEDep[MaxPart], double DecayEDep[MaxPart], double Start[MaxPart][4], double End[MaxPart][4],
						     int nParents[MaxPart], int Parent[MaxPart][MaxParent], int PDG[MaxPart],
						     int ThisID, unsigned int ThisTDC, sim::IDE ThisIDE, const simb::MCParticle& MCPart, bool Decay, bool &Written ) {

  std::vector<int>::iterator it=std::find( TrackIDVec.begin(), TrackIDVec.end(), abs(ThisID) );
  int partNum = 0;
  // ------- Figure out which partNum I want to use --------
  if ( it==TrackIDVec.end() ) {
    TrackIDVec.push_back ( abs(ThisID) ); // Push back this particle type IDVec
    AllTrackIDs.push_back( abs(ThisID) ); // Push back all particle type IDVec
    PDG[numParts] = MCPart.PdgCode();
    std::cout << "Pushing back a new ideTrackID " << ThisID << ", it was from a " << MCPart.PdgCode() << " " << PDG[numParts] << std::endl;
    // ---- Work out the particles ancestry ----
    Parent[numParts][0] = MCPart.Mother();
    int NumParent = 0;
    int ParentID  = Parent[numParts][0];
    while ( ParentID > 0 ) {
      int pdg    = truthmap[ ParentID ]->PdgCode();
      Parent[numParts][NumParent] = pdg;
      std::cout << "The particles parent was TrackID " << ParentID << ", PdgCode " << Parent[numParts][NumParent] << std::endl;
      ++NumParent;
      ParentID = truthmap[ ParentID ]->Mother();
    }
    nParents[numParts] = NumParent;
    ++numParts;
  } else {  
    partNum = it - TrackIDVec.begin();
  }

  // --------- Work out the start / end of this track ----------
  if ( ThisID > 0 && ( ThisIDE.y > Start[partNum][1] || Start[partNum][1] == HolderVal) ) {
    Start[partNum][0] = ThisIDE.x;
    Start[partNum][1] = ThisIDE.y;
    Start[partNum][2] = ThisIDE.z;
    Start[partNum][3] = ThisTDC;
  }
  if ( ThisID > 0 && ( ThisIDE.y < End[partNum][1] || End[partNum][1] == HolderVal) ) {
    End[partNum][0] = ThisIDE.x;
    End[partNum][1] = ThisIDE.y;
    End[partNum][2] = ThisIDE.z;
    End[partNum][3] = ThisTDC;
  }

  // ----------- If TrackID is positive add it to the EDep from this track, if negative then it has to be from a daughter --------------
  if ( ThisID == ThisIDE.trackID ) {
    EDep[partNum]       += ThisIDE.energy;
  } else if (Decay) {
    DecayEDep[partNum]  += ThisIDE.energy;
  } else {
    DaughtEDep[partNum] += ThisIDE.energy;
  }
  Written=true;
} // FillVars
// ******************************** Define Module *****************************************************
DEFINE_ART_MODULE(NeutronDecayN2Ana::NeutronDecayN2Ana)
