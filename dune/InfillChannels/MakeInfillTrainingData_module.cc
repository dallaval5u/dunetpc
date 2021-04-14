////////////////////////////////////////////////////////////////////////
// Class:       MakeInfillTrainingData
// Plugin Type: analyzer (art v3_05_01)
// File:        MakeInfillTrainingData
//
// Generated at Wed Dec  9 11:34:56 2020 by Alexander Wilkinson using cetskelgen
// from cetlib version v3_10_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "art_root_io/TFileService.h"
#include "art_root_io/TFileDirectory.h"
#include "lardataobj/RawData/RawDigit.h"
#include "larcorealg/Geometry/GeometryCore.h"
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RawData/raw.h"
#include "larevt/CalibrationDBI/Interface/ChannelStatusService.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"

#include <TH2F.h>
#include <TTree.h>
#include <TGeoVolume.h>
#include <TFile.h>

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <iterator>
#include <utility>

namespace Infill {
  class MakeInfillTrainingData;
}


class Infill::MakeInfillTrainingData : public art::EDAnalyzer {
public:
  explicit MakeInfillTrainingData(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  MakeInfillTrainingData(MakeInfillTrainingData const&) = delete;
  MakeInfillTrainingData(MakeInfillTrainingData&&) = delete;
  MakeInfillTrainingData& operator=(MakeInfillTrainingData const&) = delete;
  MakeInfillTrainingData& operator=(MakeInfillTrainingData&&) = delete;

  // Required functions.
  void analyze(art::Event const& e) override;

  // Selected optional functions.
  void beginJob() override;
  void endJob() override;

private:
  // Declare member data here.
  const geo::GeometryCore* fGeom;

  art::ServiceHandle<art::TFileService> tfs;

  std::set<raw::ChannelID_t> fBadChannels;
  std::set<raw::ChannelID_t> fNoisyChannels;
  std::set<raw::ChannelID_t> fDeadChannels;

  std::set<readout::ROPID> fActiveRops;

  std::string fInputLabel;
};

Infill::MakeInfillTrainingData::MakeInfillTrainingData(fhicl::ParameterSet const& p)
  : EDAnalyzer{p},
  fInputLabel           (p.get<std::string> ("inputLabel"))
{
  consumes<std::vector<raw::RawDigit>>(fInputLabel);
}

void Infill::MakeInfillTrainingData::analyze(art::Event const& e)
{
  std::string sEvent = (
    "Ev" + std::to_string(e.id().event()) + "Run" + std::to_string(e.id().run()) + 
    "SRun" + std::to_string(e.id().subRun())
  );
  std::map<readout::ROPID, TH2F*> ropImages;

  auto const detProp = art::ServiceHandle<detinfo::DetectorPropertiesService>()->DataFor(e);
  // Networks expect a fixed image size
  assert(detProp.NumberTimeSamples() == 6000);

  // Prepare TH2s
  for (const readout::ROPID& rop : fActiveRops) {
    std::string sTitle = (
      sEvent + "_TPCset" + std::to_string(rop.TPCset) + "_ROP" + std::to_string(rop.ROP)
    );
    ropImages[rop] = tfs->make<TH2F>(
      sTitle.c_str(), sTitle.c_str(), fGeom->Nchannels(rop), 0, fGeom->Nchannels(rop), 6000, 0, 6000
    );
  }

  art::Handle<std::vector<raw::RawDigit>> digs;
  e.getByLabel(fInputLabel, digs);

  // Fill TH2s
  for(const raw::RawDigit& dig : *digs){ 
    readout::ROPID rop = fGeom->ChannelToROP(dig.Channel());
    if(ropImages.count(rop)){ // Ignores ROPs associated with only inactive TPCIDs
      raw::RawDigit::ADCvector_t adcs(dig.Samples());
      // raw::Uncompress(dig.ADCs(), adcs, dig.GetPedestal(), dig.Compression());
      raw::Uncompress(dig.ADCs(), adcs, dig.Compression());

      const raw::ChannelID_t firstCh = fGeom->FirstChannelInROP(rop);
      for(unsigned int tick = 0; tick < adcs.size(); ++tick){
        const int adc = adcs[tick] ? int(adcs[tick]) - dig.GetPedestal() : 0;

        ropImages[rop]->Fill(dig.Channel() - firstCh, tick, adcs[tick]);
      }
    }
  }

  tfs->file().Write();
  tfs->file().Flush();
  for(auto it: ropImages) delete it.second;
}

void Infill::MakeInfillTrainingData::beginJob()
{
  fGeom = art::ServiceHandle<geo::Geometry>()->provider();

  // Get active ROPs (not facing a wall)
  geo::ROP_id_iterator iRop, rBegin(fGeom, geo::ROP_id_iterator::begin_pos), 
    rEnd(fGeom, geo::ROP_id_iterator::end_pos);
  for (iRop = rBegin; iRop != rEnd; ++iRop) { // Iterate over ROPs in detector
    for (const geo::TPCID tpcId : fGeom->ROPtoTPCs(*iRop)) {
      const geo::TPCGeo tpc = fGeom->TPC(tpcId);
      const TGeoVolume* tpcVol = tpc.ActiveVolume();
      
      if (tpcVol->Capacity() > 1000000) { // At least one of the ROP's TPCIDs needs to be active
        // Networks expect a fixed image size
        if(fGeom->SignalType(*iRop) == geo::kInduction) assert(fGeom->Nchannels(*iRop) == 800);
        if(fGeom->SignalType(*iRop) == geo::kCollection) assert(fGeom->Nchannels(*iRop) == 480);

        fActiveRops.insert(*iRop);
        break;
      }
    }
  }
}

void Infill::MakeInfillTrainingData::endJob()
{
  std::cout << "Getting dead channels..." << std::endl;

  fBadChannels = art::ServiceHandle<lariov::ChannelStatusService const>()->GetProvider().BadChannels();
  fNoisyChannels = art::ServiceHandle<lariov::ChannelStatusService const>()->GetProvider().NoisyChannels();

  std::merge(
    fBadChannels.begin(), fBadChannels.end(), fNoisyChannels.begin(), fNoisyChannels.end(), 
    std::inserter(fDeadChannels, fDeadChannels.begin())
  );

  // Print dead channels to terminal
  for(const readout::ROPID& rop : fActiveRops) {
    std::cout << "(TPCset " << rop.TPCset << ", ROP " << rop.ROP << "): ";
    const raw::ChannelID_t firstCh = fGeom->FirstChannelInROP(rop);
    for (const raw::ChannelID_t ch : fDeadChannels) {
      if (fGeom->ChannelToROP(ch) == rop) {
        std::cout << ch - firstCh << " ";
      }
    } // Could break early if ch > last ch in currentRop to save a bit of looping?
    std::cout << std::endl;
  }
}

DEFINE_ART_MODULE(Infill::MakeInfillTrainingData)
