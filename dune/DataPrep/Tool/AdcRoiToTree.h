// AdcRoiToTree.h
//
// David Adams
// February 2021
//
// Tool to write ADC ROIs to a Root TTree.
//
// Configuration:
//            LogLevel - Logging level: 0=none, 1=init, 2=call, ...
//            OutFile - Output file name.

#ifndef AdcRoiToTree_H
#define AdcRoiToTree_H

#include "dune/DuneInterface/Tool/TpcDataTool.h"
#include "art/Utilities/ToolMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>

class AdcRoiToTree : TpcDataTool {

public:

  using Name = std::string;
  using Index = unsigned int;
  using IndexVector = std::vector<Index>;
  using FloatVector = std::vector<float>;

  struct TreeData {
    Index event =0;
    Index run =0;
    Index channel =0;
    Index status =0;
    Index nroi =0;
    IndexVector nsam;
    IndexVector isam;
    FloatVector qroi;
    FloatVector hmin;
    FloatVector hmax;
  };

  // Ctor.
  AdcRoiToTree(fhicl::ParameterSet const& ps);

  // Dtor.
  ~AdcRoiToTree() override;

  // AdcChannelTool methods.
  //DataMap view(const AdcChannelData& acd) const override;
  DataMap viewMap(const AdcChannelDataMap& acds) const override;
  bool updateWithView() const override { return true; }

  // Helpers.
  Name treeName() const { return "adcrois"; }

private:

  // Configuration data.
  int m_LogLevel;
  Name m_OutFile;

};

DEFINE_ART_CLASS_TOOL(AdcRoiToTree)

#endif
