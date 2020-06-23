////////////////////////////////////////////////////////////////////////
//
// \file EnergyReco_module.cc
//
// n.grant.3@warwick.ac.uk
//
///////////////////////////////////////////////////////////////////////

#ifndef EnergyReco_H
#define EnergyReco_H

// Generic C++ includes
#include <iostream>
#include <limits>

//ROOT
#include "TTree.h"

// Framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDProducer.h" 
#include "art/Framework/Principal/Event.h" 
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/ParameterSet.h" 
#include "messagefacility/MessageLogger/MessageLogger.h" 
#include "art_root_io/TFileService.h"
#include "art/Persistency/Common/PtrMaker.h"

#include "nugen/NuReweight/art/NuReweight.h"
#include "Framework/Utils/AppInit.h"
#include "nusimdata/SimulationBase/GTruth.h"
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
#include "larcoreobj/SummaryData/POTSummary.h"

#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RecoBase/Wire.h"
#include "lardataobj/RecoBase/Track.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/SpacePoint.h"
#include "lardataobj/RecoBase/Shower.h"
#include "larcoreobj/SimpleTypesAndConstants/PhysicalConstants.h"

#include "lardataobj/AnalysisBase/Calorimetry.h"
#include "larreco/Calorimetry/CalorimetryAlg.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "lardata/Utilities/AssociationUtil.h"
#include "larreco/RecoAlg/TrackMomentumCalculator.h"

#include "dune/FDSensOpt/FDSensOptData/EnergyRecoOutput.h"
#include "dune/FDSensOpt/NeutrinoEnergyRecoAlg/NeutrinoEnergyRecoAlg.h"
#include "dune/AnaUtils/DUNEAnaEventUtils.h"
#include "dune/AnaUtils/DUNEAnaHitUtils.h"
#include "dune/AnaUtils/DUNEAnaShowerUtils.h"



#include "TTimeStamp.h"
#include "TTree.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLorentzVector.h"

#include "Framework/ParticleData/PDGCodes.h"
#include "Framework/ParticleData/PDGUtils.h"
#include "Framework/ParticleData/PDGLibrary.h"

#include "Framework/EventGen/EventRecord.h"
#include "Framework/GHEP/GHepParticle.h"

namespace dune {

    class EnergyReco : public art::EDProducer {

        public:

            explicit EnergyReco(fhicl::ParameterSet const& pset);
            //virtual ~EnergyReco();
            void produce(art::Event& evt) override;

        private:
            art::Ptr<recob::Track> GetLongestTrack(const art::Event& event);
            art::Ptr<recob::Shower> GetHighestChargeShower(const art::Event& event);

            //Run information
            std::string fWireLabel;
            std::string fHitLabel;
            std::string fTrackLabel;
            std::string fShowerLabel;
            std::string fTrackToHitLabel;
            std::string fShowerToHitLabel;
            std::string fHitToSpacePointLabel;

            int fRecoMethod;


            NeutrinoEnergyRecoAlg fNeutrinoEnergyRecoAlg;
    }; // class EnergyReco


    //------------------------------------------------------------------------------
    EnergyReco::EnergyReco(fhicl::ParameterSet const& pset) :
        EDProducer(pset), 
        fWireLabel(pset.get<std::string>("WireLabel")),
        fHitLabel(pset.get<std::string>("HitLabel")),
        fTrackLabel(pset.get<std::string>("TrackLabel")),
        fShowerLabel(pset.get<std::string>("ShowerLabel")),
        fTrackToHitLabel(pset.get<std::string>("TrackToHitLabel")),
        fShowerToHitLabel(pset.get<std::string>("ShowerToHitLabel")),
        fHitToSpacePointLabel(pset.get<std::string>("HitToSpacePointLabel")),
        fRecoMethod(pset.get<int>("RecoMethod")),
        fNeutrinoEnergyRecoAlg(pset.get<fhicl::ParameterSet>("NeutrinoEnergyRecoAlg"),fTrackLabel,fShowerLabel,
            fHitLabel,fWireLabel,fTrackToHitLabel,fShowerToHitLabel,fHitToSpacePointLabel)
    {
        produces<dune::EnergyRecoOutput>();
        produces<art::Assns<dune::EnergyRecoOutput, recob::Track>>();
        produces<art::Assns<dune::EnergyRecoOutput, recob::Shower>>();
    }

    //dune::EnergyReco::~EnergyReco(){}

    //------------------------------------------------------------------------------
    void EnergyReco::produce(art::Event& evt)
    {
        std::unique_ptr<dune::EnergyRecoOutput> energyRecoOutput;
        auto assnstrk = std::make_unique<art::Assns<dune::EnergyRecoOutput, recob::Track>>();
        auto assnsshw = std::make_unique<art::Assns<dune::EnergyRecoOutput, recob::Shower>>();

        art::Ptr<recob::Track> longestTrack(this->GetLongestTrack(evt));
        art::Ptr<recob::Shower> highestChargeShower(this->GetHighestChargeShower(evt));

        if (fRecoMethod == 1)
        {
            energyRecoOutput = std::make_unique<dune::EnergyRecoOutput>(fNeutrinoEnergyRecoAlg.CalculateNeutrinoEnergy(longestTrack, evt));
        }
        else if (fRecoMethod == 2)
        {
            energyRecoOutput = std::make_unique<dune::EnergyRecoOutput>(fNeutrinoEnergyRecoAlg.CalculateNeutrinoEnergy(highestChargeShower, evt));
        }
        else if (fRecoMethod == 3)
            energyRecoOutput = std::make_unique<dune::EnergyRecoOutput>(fNeutrinoEnergyRecoAlg.CalculateNeutrinoEnergy(evt));

        art::ProductID const prodId = evt.getProductID<dune::EnergyRecoOutput>();
        art::EDProductGetter const* prodGetter = evt.productGetter(prodId);
        art::Ptr<dune::EnergyRecoOutput> EnergyRecoOutputPtr{ prodId, 0U, prodGetter };
        //art::Ptr<dune::EnergyRecoOutput> EnergyRecoOutputPtr = makeEnergyRecoOutputPtr(0);
        if (longestTrack.isAvailable()) assnstrk->addSingle(EnergyRecoOutputPtr, longestTrack);
        if (highestChargeShower.isAvailable()) assnsshw->addSingle(EnergyRecoOutputPtr, highestChargeShower);
        evt.put(std::move(energyRecoOutput));
        evt.put(std::move(assnstrk));
        evt.put(std::move(assnsshw));
    }

    art::Ptr<recob::Track> EnergyReco::GetLongestTrack(const art::Event &event)
    {
        art::Ptr<recob::Track> pTrack(art::Ptr<recob::Track>(art::ProductID("nullTrack")));
        const std::vector<art::Ptr<recob::Track> > tracks(dune_ana::DUNEAnaEventUtils::GetTracks(event, fTrackLabel));
        if (0 == tracks.size())
            return pTrack;

        double longestLength(std::numeric_limits<double>::lowest());
        for (unsigned int iTrack = 0; iTrack < tracks.size(); ++iTrack)
        {
            const double length(tracks[iTrack]->Length());
            if (length-longestLength > std::numeric_limits<double>::epsilon())
            {
                longestLength = length;
                pTrack = tracks[iTrack];
            }
        }
        return pTrack;
    }

    art::Ptr<recob::Shower> EnergyReco::GetHighestChargeShower(const art::Event &event)
    {
        art::Ptr<recob::Shower> pShower(art::Ptr<recob::Shower>(art::ProductID("nullShower")));
        const std::vector<art::Ptr<recob::Shower> > showers(dune_ana::DUNEAnaEventUtils::GetShowers(event, fShowerLabel));
        if (0 == showers.size())
            return pShower;

        double maxCharge(std::numeric_limits<double>::lowest());
        for (unsigned int iShower = 0; iShower < showers.size(); ++iShower)
        {
            const std::vector<art::Ptr<recob::Hit> > showerHits(dune_ana::DUNEAnaHitUtils::GetHitsOnPlane(dune_ana::DUNEAnaShowerUtils::GetHits(showers[iShower],
                event,fShowerToHitLabel),2));
            const double showerCharge(dune_ana::DUNEAnaHitUtils::LifetimeCorrectedTotalHitCharge(showerHits));
            if (showerCharge-maxCharge > std::numeric_limits<double>::epsilon())
            {
                maxCharge = showerCharge;
                pShower = showers[iShower];
            }
        }
        return pShower;
    }

    DEFINE_ART_MODULE(EnergyReco)

} // namespace dune

#endif // EnergyReco_H
