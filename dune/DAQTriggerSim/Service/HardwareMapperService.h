// HardwareMapperService.h
//
// Jonathan Davies j.p.davies@sussex.ac.uk
// August 2016
//
// Description

#ifndef HARDWAREMAPPERSERVICE_H
#define HARDWAREMAPPERSERVICE_H

//The service declaration macros are defined in here
#include "art/Framework/Services/Registry/ServiceMacros.h"
//Needed to get a service handle
#include "art/Framework/Services/Registry/ServiceHandle.h"

//Testing
#include "lbne-raw-data/Services/ChannelMap/ChannelMapService.h"
//Not testing
#include "larcore/Geometry/Geometry.h"

#include <vector>
#include <set>
#include <iostream>
#include <iosfwd>

//Forward Class Declarations
namespace fhicl {
  class ParameterSet;
}

namespace art {
  class ActivityRegistry;
}

#define INFO  std::cerr << "INFO   : "
#define ERROR std::cerr << "ERROR  : "
#define INFO_FILE_FUNCTION  std::cerr << "INFO   : " << __FILE__ << " : " << __FUNCTION__


//......................................................
namespace Hardware{
  typedef unsigned int ID;

  class Element
  {
  public:
    Element(ID id, std::string this_type) : fID(id), fType(this_type) {}
    ID const& getID() const{ return fID;}
    std::string const& getType() const{ return fType; }
    std::set<raw::ChannelID_t> const& getChannels() const{ return fChannelIDs;}
    size_t getNChannels() const{ return fChannelIDs.size();}

    bool addChannel(raw::ChannelID_t this_channel){
      auto result = fChannelIDs.insert(this_channel);
      if(result.first != fChannelIDs.end()) return result.second;
      else return false;
    }
    bool operator<( const Element& rhs ) const {
      return this->getID() < rhs.getID();
    }
    friend std::ostream & operator << (std::ostream &os,  Element &rhs){
      os << rhs.getType() << ": " << rhs.getID() << " - " << rhs.getNChannels() << " channels";
      std::set<raw::ChannelID_t> channels = rhs.getChannels();
      unsigned int max_num_channels = 10;
      unsigned int this_channel_num = 0;
      for(auto channel : channels){
        if(this_channel_num==0)  os << ":";
        if(this_channel_num++ >= max_num_channels) break;
        os << " " << channel;
      }
      return os;
    }

  private:
    ID fID;
    std::string fType;
    std::set<raw::ChannelID_t> fChannelIDs;
  };
  
  class ASIC : public Element{ public: ASIC(ID id) : Element(id, "ASIC") {} };

  class Board  : public Element{ public: Board(ID id) : Element(id, "Board" ) {} };

  class TPC  : public Element{ public: TPC(ID id) : Element(id, "TPC" ) {} };

  class APA  : public Element{ public: APA(ID id) : Element(id, "APA" ) {} };

  class APAGroup  : public Element{ public: APAGroup(ID id) : Element(id, "APAGroup" ) {} };

  class Cryostat  : public Element{ public: Cryostat(ID id) : Element(id, "Cryostat" ) {} };

  using APAMap =   std::map<ID, std::shared_ptr<APA>>;
  using TPCMap =   std::map<ID, std::shared_ptr<TPC>>;
  
}

//......................................................
class HardwareMapperService{
 public:
  HardwareMapperService(fhicl::ParameterSet const& pset, art::ActivityRegistry& reg);
  //  ~HardwareMapperService() = default;
  
  int getHardwareElement(int const& element, std::vector<int>& channelVec);
  void printHardwareElement(int const& element);
  void printChannelMap();
  void printGeometryInfo();

  void fillAPAMap();
  void fillTPCMap();

  void printAPAMap(unsigned int num_apas_to_print=10);
  void printTPCMap(unsigned int num_tpcs_to_print=10);
  
  std::set<raw::ChannelID_t> const& getAPAChannels(Hardware::ID apa_id);
  std::set<raw::ChannelID_t> const& getTPCChannels(Hardware::ID tpc_id);

 private:
  art::ServiceHandle<lbne::ChannelMapService> fChannelMapService; //FIXME -- just for testing
  art::ServiceHandle<geo::Geometry> fGeometryService; //FIXME -- just for testing

  Hardware::APAMap fAPAMap;
  Hardware::TPCMap fTPCMap;

};

//......................................................
DECLARE_ART_SERVICE(HardwareMapperService, LEGACY)

#endif
