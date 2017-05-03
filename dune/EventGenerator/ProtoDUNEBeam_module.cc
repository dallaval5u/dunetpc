////////////////////////////////////////////////////////////////////////
// Class:       ProtoDUNEBeam
// Module Type: producer
// File:        ProtoDUNEBeam_module.cc
//
// Generated at Thu Nov 17 11:20:31 2016 by Leigh Howard Whitehead,42 3-039,+41227672470, using artmod
// from cetpkgsupport v1_11_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "cetlib/exception.h"

#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "larcore/Geometry/Geometry.h"
#include "larcoreobj/SummaryData/RunData.h"

#include <memory>
#include <string>
#include <map>
#include <utility>
#include <vector>

// art extensions
#include "nutools/RandomUtils/NuRandomService.h"

#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TDatabasePDG.h>
#include <TParticlePDG.h>
#include "TSystem.h" 

#include "CLHEP/Random/RandFlat.h"
#include "ifdh.h"
#include <sys/stat.h> 

namespace evgen{
  class ProtoDUNEBeam;

  class ProtoDUNEBeam : public art::EDProducer {
  public:
    explicit ProtoDUNEBeam(fhicl::ParameterSet const & p);
    // The destructor generated by the compiler is fine for classes
    // without bare pointers or other resource use.
    ~ProtoDUNEBeam();

    // Plugins should not be copied or assigned.
    ProtoDUNEBeam(ProtoDUNEBeam const &) = delete;
    ProtoDUNEBeam(ProtoDUNEBeam &&) = delete;
    ProtoDUNEBeam & operator = (ProtoDUNEBeam const &) = delete;
    ProtoDUNEBeam & operator = (ProtoDUNEBeam &&) = delete;

    // Required functions.
    void produce(art::Event & e) override;
    void beginJob() override;
    void beginRun(art::Run& run) override;
    void endJob() override;

  private:

    // We need to make a map of good particle event numbers and all
    // matching entries in the overlay events in the main particle list.
    std::map<int,std::vector<std::pair<int,std::vector<int> > > > fEventParticleMap;

    // A second map storing the trigger time of the good particle.
    std::map<int,float> fGoodParticleTriggerTime;

    // Track ID of the good particle.
    std::map<int,int> fGoodParticleTrackID;

    // A list of good events and an index for it.
    unsigned int fCurrentGoodEvent;
    std::vector<int> fGoodEventList;

		// Calculate how many overlay events we need.
		void CalculateNOverlays();

		// Check if a given beam event is close enough to a good particle event to be useful.
		int IsOverlayEvent(int event, int nOverlay);
		std::vector<int> GetAllOverlays(int event, int nOverlay);

    // Fill the above maps and vector.
    void FillParticleMaps();

    // Generate a true event based on a single entry from the input tree.
    void GenerateTrueEvent(simb::MCTruth &mcTruth);
    
    // Handle root files from beam instrumentation group
    void OpenInputFile();

    // Generate a TLorentzVector for position making sure we get the
    // coordinates as we need them. 
    TLorentzVector ConvertCoordinates(float x, float y, float z, float t);

    // Make the momentum vector, rotating as required.
    TLorentzVector MakeMomentumVector(float px, float py, float pz, int pdg);

    std::string fFileName;
    std::string fGoodParticleTreeName;
    std::string fAllParticlesTreeName;

    // The current event number. Ideally this could be an unsigned int,
    // but we will need to compare it to some ints later on. 
    int fEventNumber;

    // Let the user define the event to start at
    int fStartEvent;

    TFile* fInputFile;
    // Input file provides a TTree that we need to read.
    TTree* fGoodParticleTree;
    TTree* fAllParticlesTree;  

    // Members we need to extract from the tree
    float fX, fY, fZ;
    float fPx, fPy, fPz;
    float fPDG; // Input tree has all floats
    float fBeamEvent;
    float fTrackID;
    // We need two times: the trigger time, and the time at the entry point
    // to the TPC where we generate the event.
    float fEntryT, fTriggerT;

    // Define the coordinate transform from the beam frame to the detector frame
    float fBeamX;
    float fBeamY;
    float fBeamZ;
    float fRotateXZ;
    float fRotateYZ;

		// Parameters from the .fcl file to deal with overlaying events
		float fIntensity; // Number of interactions on the secondary target per SPS spill
		float fReadoutWindow; // Readout window (needs to match the values used in the simulation) in milliseconds
		float fBeamSpillLength; // The SPS spill length in seconds
 
		// Number of beam interactions to overlay.
		int fOverlays;
  
		// In the absense of the full shielding in the beam simulation, add a momentum cut away from the beam pipe.
		float fBackgroundMomentumCut; // Momentum cut in GeV/c
		float fBackgroundRadiusCut; // Radius away from beam pipe to consider in cm
 
    ifdh_ns::ifdh* fIFDH;
  };
}

evgen::ProtoDUNEBeam::ProtoDUNEBeam(fhicl::ParameterSet const & pset)
{

  // Call appropriate produces<>() functions here.
  produces< std::vector<simb::MCTruth> >();
  produces< sumdata::RunData, art::InRun >();

  // File reading variable initialisations
  fFileName = pset.get< std::string>("FileName");
  fGoodParticleTreeName = pset.get< std::string>("GoodParticleTreeName");
  fAllParticlesTreeName = pset.get< std::string>("AllParticlesTreeName");

	// Intensity variables 
	fIntensity = pset.get<float>("Intensity");
	fReadoutWindow = pset.get<float>("ReadoutWindow");
	fBeamSpillLength = pset.get<float>("BeamSpillLength");

	// Background cut variables
	fBackgroundMomentumCut = pset.get<float>("BkgMomentumCut");
	fBackgroundRadiusCut = pset.get<float>("BkgRadiusCut");

  // See if the user wants to start at an event other than zero.
  fStartEvent = pset.get<int>("StartEvent");

  // Or maybe there was --nskip specified in the command line or skipEvents in FHiCL?
  for (auto const & p : fhicl::ParameterSetRegistry::get())
  {
    if (p.second.has_key("source.skipEvents"))
    {
      fStartEvent += p.second.get<int>("source.skipEvents");
      break; // take the first occurence
    } // no "else", if parameter not found, then just don't change anything
  }
  // ...and if there is -e option or firstEvent in FHiCL, this add up to the no. of events to skip.
  for (auto const & p : fhicl::ParameterSetRegistry::get())
  {
    if (p.second.has_key("source.firstEvent"))
    {
      int fe = p.second.get<int>("source.firstEvent") - 1; // events base index is 1
      if (fe > 0) fStartEvent += fe;
      break; // take the first occurence
    } // no "else", if parameter not found, then just don't change anything
  }
  mf::LogInfo("ProtoDUNEBeam") << "Skip " << fStartEvent << " first events from the input file.";

  fEventNumber = 0;

  // Coordinate transform
  fBeamX = pset.get<float>("BeamX");
  fBeamY = pset.get<float>("BeamY");
  fBeamZ = pset.get<float>("BeamZ");
  fRotateXZ = pset.get<float>("RotateXZ");
  fRotateYZ = pset.get<float>("RotateYZ");

  // Initialise the input file and tree to be null.
  fInputFile = 0x0;
  fGoodParticleTree = 0x0;
  fAllParticlesTree = 0x0;
  fIFDH = 0;
  
  fCurrentGoodEvent = 0;

	// Make sure we use ifdh to open the beam input file.
  OpenInputFile();

	// Create the random number generator
	std::string const instanceName = "protoDUNEBeam";
	auto& Seeds = *(art::ServiceHandle<rndm::NuRandomService>());

	// declare an engine; NuRandomService associates an (unknown) engine, in
	// the current module and an instance name, with a seed (returned)
	auto const seed = Seeds.declareEngine(instanceName);

	// now create the engine (for example, use art); seed will be set
	createEngine(seed, "HepJamesRandom", instanceName);

	// finally, complete the registration; seed will be set again
//	art::ServiceHandle<art::RandomNumberGenerator> RNG;
//	Seeds.defineEngine(RNG->getEngine(instanceName));
}

evgen::ProtoDUNEBeam::~ProtoDUNEBeam()
{
	fIFDH->cleanup();
}

void evgen::ProtoDUNEBeam::beginJob(){

  fInputFile = new TFile(fFileName.c_str(),"READ");
  // Check we have the file
  if(fInputFile == 0x0){
    throw cet::exception("ProtoDUNEBeam") << "Input file " << fFileName << " cannot be read.\n";
  }

  fGoodParticleTree = (TTree*)fInputFile->Get(fGoodParticleTreeName.c_str());
  // Check we have the tree
  if(fGoodParticleTree == 0x0){
    throw cet::exception("ProtoDUNEBeam") << "Input tree " << fGoodParticleTreeName << " cannot be read.\n";
  }

  fAllParticlesTree = (TTree*)fInputFile->Get(fAllParticlesTreeName.c_str());
  // Check we have the tree
  if(fAllParticlesTree == 0x0){
    throw cet::exception("ProtoDUNEBeam") << "Input tree " << fAllParticlesTreeName << " cannot be read.\n";
  }

  // Since this is technically an ntuple, all objects are floats
  // Position four-vector components
  fAllParticlesTree->SetBranchAddress("x",&fX);
  fAllParticlesTree->SetBranchAddress("y",&fY);
  fAllParticlesTree->SetBranchAddress("z",&fZ);
  fAllParticlesTree->SetBranchAddress("t",&fEntryT);
  // Momentum components
  fAllParticlesTree->SetBranchAddress("Px",&fPx);
  fAllParticlesTree->SetBranchAddress("Py",&fPy);
  fAllParticlesTree->SetBranchAddress("Pz",&fPz);
  // PDG code
  fAllParticlesTree->SetBranchAddress("PDGid",&fPDG);
  // Event and track number
  fAllParticlesTree->SetBranchAddress("EventID",&fBeamEvent);
  fAllParticlesTree->SetBranchAddress("TrackID",&fTrackID);

  // We only need the trigger time and event number from the good particle tree
  fGoodParticleTree->SetBranchAddress("Lag_ENTRY_EventID",&fBeamEvent);
  fGoodParticleTree->SetBranchAddress("Lag_ENTRY_TrackID",&fTrackID);
  fGoodParticleTree->SetBranchAddress("TRIG2_t",&fTriggerT);
 
	// Calculate the number of events to overlay
	CalculateNOverlays(); 

  // Now we need to fill the particle map
  FillParticleMaps();
}

void evgen::ProtoDUNEBeam::beginRun(art::Run& run)
{
  // Grab the geometry object to see what geometry we are using
  art::ServiceHandle<geo::Geometry> geo;
  std::unique_ptr<sumdata::RunData> runcol(new sumdata::RunData(geo->DetectorName()));
  
  run.put(std::move(runcol));
}
                   
void evgen::ProtoDUNEBeam::endJob(){
  fInputFile->Close();
}

void evgen::ProtoDUNEBeam::produce(art::Event & e)
{

  // Define the truth collection for this event.
  auto truthcol = std::make_unique< std::vector<simb::MCTruth> >();
  simb::MCTruth truth;

  // Fill the MCTruth object  
  GenerateTrueEvent(truth);

  // Add the MCTruth to the vector
  truthcol->push_back(truth);

  // Finally, add the MCTruth to the event
  e.put(std::move(truthcol));

  // We have made our event, increment the event number.
  ++fEventNumber;
}

// Fill the particle maps using the input files. This links the events of interest
// to the entry number in fAllParticlesTree.
void evgen::ProtoDUNEBeam::FillParticleMaps(){

  // First off, loop over the good particles tree.
  int goodEventCounter = 0;
  for(int i = 0; i < fGoodParticleTree->GetEntries(); ++i){

    // If we want to skip some events, make sure we don't bother reading them in.
    if(fStartEvent > goodEventCounter){
      ++goodEventCounter;
      continue;
    }
    else{
      ++goodEventCounter;
    } 

    fGoodParticleTree->GetEntry(i);
    int event = (int)fBeamEvent;

    // Initialise the event - particle map. This will be filled
    // in the next loop.
    if(fEventParticleMap.find(event) == fEventParticleMap.end()){
      std::vector<int> tempVec; // This will be the vector of track ids for each event near to the good event.
			std::pair<int,std::vector<int> > tempPair = std::make_pair(event,tempVec); 
			std::vector<std::pair<int,std::vector<int> > > tempMainVec;
			tempMainVec.push_back(tempPair);
      fEventParticleMap.insert(std::make_pair(event,tempMainVec));
      fGoodEventList.push_back(event);
    }

    // Trigger times map
/*    if(fGoodParticleTriggerTime.find(event) == fGoodParticleTriggerTime.end()){
      std::vector<float> trigTimes;
      trigTimes.push_back(fTriggerT);
      fGoodParticleTriggerTime.insert(std::make_pair(event,trigTimes));
    }
    else{
      fGoodParticleTriggerTime[event].push_back(fTriggerT);
    }
*/

		fGoodParticleTriggerTime.insert(std::make_pair(event,fTriggerT));

    // Track ID map
    int trackID = (int)fTrackID;
//    std::cout << "GoodParticle: " << event << ", " << trackID << std::endl; 
/*    if(fGoodParticleTrackID.find(event) == fGoodParticleTrackID.end()){
      std::vector<int> tracks;
      tracks.push_back(trackID);
      fGoodParticleTrackID.insert(std::make_pair(event,tracks));
    }
    else{
      fGoodParticleTrackID[event].push_back(trackID);
    }
*/

		fGoodParticleTrackID.insert(std::make_pair(event,trackID));
  }

  // Print a message incase a user starts thinking something has broken.
  mf::LogInfo("ProtoDUNEBeam") << "About to loop over the beam simulation tree, this could take some time.";

  // Now we need to loop over the main particle tree
  for(int i = 0; i < fAllParticlesTree->GetEntries(); ++i){
    fAllParticlesTree->GetEntry(i);

    // Is this an event we care about?
    int event = int(fBeamEvent);

		// We need to calculate if this is an interesting event, ie is it within +/- fOverlays/2 of a good particle event?
//		int goodEvent = IsOverlayEvent(event,fOverlays);
//		if(goodEvent == -1){
//			continue;
//		}
		std::vector<int> goodEventList = GetAllOverlays(event,fOverlays);
		for(auto const goodEvent : goodEventList){
	    if(fEventParticleMap.find(goodEvent) != fEventParticleMap.end()){
	    	// Store the index of this event so that we can quickly access
	      // it later when building events
	      std::vector<std::pair<int, std::vector<int> > > tracksForEvents = fEventParticleMap[goodEvent];
				bool foundEvent = false;
				unsigned int element = 0;
				for(unsigned int v = 0; v < tracksForEvents.size(); ++v){
					if(tracksForEvents[v].first == event){
						foundEvent = true;
						element = v;
						break;
					}
				}
	 			if(foundEvent){
	      	fEventParticleMap[goodEvent][element].second.push_back(i);
				}
				else{
					std::vector<int> newVec;
					newVec.push_back(i);
					std::pair<int,std::vector<int> > newEvent = std::make_pair(event,newVec);
					fEventParticleMap[goodEvent].push_back(newEvent);
				}
    	}
		} // End loop over matching events to overlay (this re-uses beam interactions...)
  } // End loop over the main tree.

  mf::LogInfo("ProtoDUNEBeam") << "Found " << fGoodEventList.size() << " good events containing " << goodEventCounter << " good particles.";
  mf::LogInfo("ProtoDUNEBeam") << "All maps built, beginning event generation.";

}

// Actually produce the MCTruth object from the input particle.
void evgen::ProtoDUNEBeam::GenerateTrueEvent(simb::MCTruth &mcTruth){

  // Check we haven't exceeded the length of the input tree
  if(fEventNumber >= (int)fGoodEventList.size()){
    throw cet::exception("ProtoDUNEBeam") << "Requested entry " << fEventNumber 
                                    << " but tree only has entries 0 to " 
                                    << fGoodEventList.size() - 1 << std::endl; 
  }

  // Get the list of entries for the current event
  int beamEvent = fGoodEventList[fCurrentGoodEvent];

/*
  // Get the trigger time for this event so that 
  // we can correct all other times
  float earliestTime = 1e6;
  for(auto const &t : fGoodParticleTriggerTime[beamEvent]){
    if(t < earliestTime){
      earliestTime = t;
    }
  }
*/

  // Get the random number generator service and make some CLHEP generators
  art::ServiceHandle<art::RandomNumberGenerator> rng;
  CLHEP::HepRandomEngine &engine = rng->getEngine("protoDUNEBeam");
  CLHEP::RandFlat flatRnd(engine);

	// The trigger time comes from the good particle to correct the good particle time to roughly 0.
//	float triggerTime = fGoodParticleTriggerTime[beamEvent]; 

  // A single particle seems the most accurate description.
  mcTruth.SetOrigin(simb::kSingleParticle);

	// Find the entries that we are interested in.
//	std::cout << "Finding all particles associated with good particle event " << beamEvent << std::endl;
	for(auto const e : fEventParticleMap[beamEvent]){
		// Is this the event we would have triggered on?
		bool trigEvent = (e.first == beamEvent);
		float baseTime;
		if(trigEvent){
			// Set the base time for the triggered event equal to the negative of the good particle time.
			// This will be corrected later on to set the time to zero, but keep time offsets within the event.
			baseTime = -1.0 * fGoodParticleTriggerTime[beamEvent];
		}
		else{
			// Get a random time from -fReadoutWindow to +fReadoutWindow in ns (fReadoutWindow value is in ms).
			baseTime = (flatRnd.fire() - 0.5)*2.0*(fReadoutWindow*1000.*1000.);
		}
		for(auto const t : e.second){
			// Get the entry from the tree for this event and track.
			fAllParticlesTree->GetEntry(t);

			// Convert the pdgCode to an int
    	int intPDG = (int)fPDG;
    	// We need to ignore nuclei for now...
    	if(intPDG > 100000) continue;

    	// Check to see if this should be a primary beam particle (good particle) or beam background
    	std::string process="primaryBackground";
   		// If this track is a "good particle", use the usual "primary" tag
    	if(trigEvent && (fGoodParticleTrackID[beamEvent] == (int)fTrackID)){
      	process="primary";
    	}

    	// Get the position four vector, converting mm to cm 
    	TLorentzVector pos = ConvertCoordinates(fX/10.,fY/10.,fZ/10.,baseTime + fEntryT);
    	// Get momentum four vector, remembering to convert MeV to GeV
    	TLorentzVector mom = MakeMomentumVector(fPx/1000.,fPy/1000.,fPz/1000.,intPDG);
			
			// Apply an energy cut to things outside the beam pipe to represent the shielding.
			float r = sqrt(fX*fX + fY*fY);
			r = r / 10.;
			if(r > fBackgroundRadiusCut && mom.Vect().Mag() < fBackgroundMomentumCut){
				continue;
			}
    	
//			if(process=="primary") std::cout << " - Got the good particle (" << intPDG << ", " << e.first << ", " << (int)fTrackID << ") with momentum = " << mom.Vect().Mag() << std::endl;

    	// Track ID needs to be negative for primaries
    	int trackID = -1*(mcTruth.NParticles() + 1);
    	
    	// Create the particle and add the starting position and momentum
    	simb::MCParticle newParticle(trackID,intPDG,process);
    	newParticle.AddTrajectoryPoint(pos,mom);
    	
    	// Add the MCParticle to the MCTruth for the event.
    	mcTruth.Add(newParticle);

		} // End loop over interesting tracks for each event
	} // End loop over the vector of interesting events

	mf::LogInfo("ProtoDUNEBeam") << "Created event with " << mcTruth.NParticles() << " particles.";

/*
  // Get the required particles
  for(auto const &v : fEventParticleMap[beamEvent]){

    fAllParticlesTree->GetEntry(v);

    // Get the time of the entry into the detector relative to the trigger.
    // This might change in future, but will serve as T0 for now.
    float correctedTime = fEntryT - earliestTime;
    
    // Since the tree is actually an ntuple, everything is stored as a float. 
    // Most things want the PDG code as an int, so make one.
    int intPDG = (int)fPDG;

    // We need to ignore nuclei for now...
    if(intPDG > 100000) continue;

    // Check to see if this should be a primary beam particle (good particle) or beam background
    std::string process="primary";
    // If this track is a "beam background", use a different tag, but still containing "primary"
    if(std::find(fGoodParticleTrackID[beamEvent].begin(),fGoodParticleTrackID[beamEvent].end(),int(fTrackID)) == fGoodParticleTrackID[beamEvent].end()){
      process="primaryBackground";
    }
    // Sometimes it seems that there is a second match for the event and track ID pair. For now, just check any particle that claims to be good
    // is actually good.
    if(process == "primary"){
      if(fabs(fX) > 250 || fabs(fY) > 250){
        continue;
      }
    }

    // Get the position four vector, converting mm to cm 
    TLorentzVector pos = ConvertCoordinates(fX/10.,fY/10.,fZ/10.,correctedTime);
    // Get momentum four vector, remembering to convert MeV to GeV
    TLorentzVector mom = MakeMomentumVector(fPx/1000.,fPy/1000.,fPz/1000.,intPDG);

    // Track ID needs to be negative for primaries
    int trackID = -1*(mcTruth.NParticles() + 1);

    // Create the particle and add the starting position and momentum
    simb::MCParticle newParticle(trackID,intPDG,process);
    newParticle.AddTrajectoryPoint(pos,mom);

    // Add the MCParticle to the MCTruth for the event.
    mcTruth.Add(newParticle);

  } 
*/
  // Move on the good event iterator
  ++fCurrentGoodEvent;
}

// Function written in similar way as "openDBs()" in CORSIKAGen_module.cc  
void evgen::ProtoDUNEBeam::OpenInputFile()
{	
	// Setup ifdh object
	if (!fIFDH) 
	{	
		fIFDH = new ifdh_ns::ifdh;
	}
	
	const char* ifdh_debug_env = std::getenv("IFDH_DEBUG_LEVEL");
	if ( ifdh_debug_env ) 
	{
      		mf::LogInfo("ProtoDUNEBeam") << "IFDH_DEBUG_LEVEL: " << ifdh_debug_env<<"\n";
      		fIFDH->set_debug(ifdh_debug_env);
	}
	
	std::string path(gSystem->DirName(fFileName.c_str()));
	std::string pattern(gSystem->BaseName(fFileName.c_str()));
	
	auto flist = fIFDH->findMatchingFiles(path,pattern);
	if (flist.empty())
	{
		struct stat buffer;
		if (stat(fFileName.c_str(), &buffer) != 0) 
		{
			throw cet::exception("ProtoDUNEBeam") << "No files returned for path:pattern: "<<path<<":"<<pattern<<std::endl;
		}
		else
		{
			mf::LogInfo("ProtoDUNEBeam") << "For "<< fFileName <<"\n";
		}
	}
	else
	{
		std::pair<std::string, long> f = flist.front();
	
		mf::LogInfo("ProtoDUNEBeam") << "For "<< fFileName <<"\n";
	
		// Do the fetching, store local filepaths in locallist
	
		mf::LogInfo("ProtoDUNEBeam")
        	<< "Fetching: " << f.first << " " << f.second <<"\n";       
		std::string fetchedfile(fIFDH->fetchInput(f.first));
		LOG_DEBUG("ProtoDUNEBeam") << " Fetched; local path: " << fetchedfile;
	
		fFileName = fetchedfile;
	}
}

TLorentzVector evgen::ProtoDUNEBeam::ConvertCoordinates(float x, float y, float z, float t){

  float finalX = x + fBeamX;
  float finalY = y + fBeamY;
  float finalZ = (z - z) + fBeamZ; // Just use the z position

  TLorentzVector newPos(finalX,finalY,finalZ,t);
  return newPos;
}

TLorentzVector evgen::ProtoDUNEBeam::MakeMomentumVector(float px, float py, float pz, int pdg){

  float rotationXZ = fRotateXZ;
  float rotationYZ = fRotateYZ; 

  // Make the momentum vector and rotate it
  TVector3 momVec(px,py,pz);
  momVec.RotateY(rotationXZ * TMath::Pi() / 180.);
  momVec.RotateX(rotationYZ * TMath::Pi() / 180.);

  // Find the particle mass so we can form the energy
  const TDatabasePDG* databasePDG = TDatabasePDG::Instance();
  const TParticlePDG* definition = databasePDG->GetParticle(pdg);
  float mass = definition->Mass();

  float energy = sqrt(mass*mass + momVec.Mag2());
  
  TLorentzVector newMom(momVec,energy);
  return newMom; 
}

void evgen::ProtoDUNEBeam::CalculateNOverlays(){

	// The number of events to overlay is as follows:
	// N = Intensity * 2.0 * ReadoutWindow / BeamSpillLength	
	fOverlays = fIntensity * (2.0 * fReadoutWindow / 1000.) / fBeamSpillLength;

}

int evgen::ProtoDUNEBeam::IsOverlayEvent(int event, int nOverlay){

	// Check if this event lies within nOverlay/2 of each 
	for(auto const e : fGoodEventList){
		if(fabs(event - e) < nOverlay/2){
			return e;
		}
	}
	return -1;
}

std::vector<int> evgen::ProtoDUNEBeam::GetAllOverlays(int event, int nOverlay){

	std::vector<int> nMatches;
	for(auto const e : fGoodEventList){
    if(fabs(event - e) < nOverlay/2){
      nMatches.push_back(e);
    }
  }
  return nMatches;

}

DEFINE_ART_MODULE(evgen::ProtoDUNEBeam)

