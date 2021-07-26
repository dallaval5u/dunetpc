////////////////////////////////////////////////////////////////////////
// Class:       CheckGeometry
// Module Type: analyzer
// File:        CheckGeometry_module.cc
//
// Generated at Tue Jan  6 22:27:12 2015 by Tingjun Yang using artmod
// from cetpkgsupport v1_07_00.
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art_root_io/TFileService.h" 
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "larcore/Geometry/Geometry.h"
#include "lardata/DetectorInfoServices/DetectorPropertiesService.h"
#include "TCanvas.h"
#include "TBox.h"
#include "TH2F.h"
#include "TLine.h"
#include "TLatex.h"
#include "TLegend.h"

#include <iostream>
#include <vector>

//constexpr unsigned short kMaxAuxDets = 100; // unused
//constexpr unsigned short kMaxTkIDs = 100; // unused

namespace dune {
  class CheckGeometry;
}

class dune::CheckGeometry : public art::EDAnalyzer {
public:
  explicit CheckGeometry(fhicl::ParameterSet const & p);
  // The destructor generated by the compiler is fine for classes
  // without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  CheckGeometry(CheckGeometry const &) = delete;
  CheckGeometry(CheckGeometry &&) = delete;
  CheckGeometry & operator = (CheckGeometry const &) = delete;
  CheckGeometry & operator = (CheckGeometry &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

  // Selected optional functions.
  void reconfigure(fhicl::ParameterSet const & p) ;

private:

  // Declare member data here.

};


dune::CheckGeometry::CheckGeometry(fhicl::ParameterSet const & p)
  :
  EDAnalyzer(p)
{
  reconfigure(p);
}

void dune::CheckGeometry::analyze(art::Event const & evt)
{

  art::ServiceHandle<geo::Geometry> geo;

  //std::cout<<"channel = "<<geo->PlaneWireToChannel(0,600,1)<<std::endl;

  TCanvas *can = new TCanvas("c1","c1");
  can->cd();
  std::vector<TBox*> TPCBox;
  std::vector<TLine*> Wires;

  double minx = 1e9;
  double maxx = -1e9;
  double miny = 1e9;
  double maxy = -1e9;
  double minz = 1e9;
  double maxz = -1e9;
  
  int nwires = 0;
  std::vector<int> nwires_tpc(geo->NTPC());
  for (size_t i = 0; i<geo->NTPC(); ++i) nwires_tpc[i] = 0;
  for (size_t t = 0; t<geo->NTPC(); ++t){
    //if (t%2==0) continue;
    double local[3] = {0.,0.,0.};
    double world[3] = {0.,0.,0.};
    const geo::TPCGeo &tpc = geo->TPC(t);
    tpc.LocalToWorld(local,world);
    if (minx>world[0]-tpc.ActiveHalfWidth())
      minx = world[0]-tpc.ActiveHalfWidth();
    if (maxx<world[0]+tpc.ActiveHalfWidth())
      maxx = world[0]+tpc.ActiveHalfWidth();
    if (miny>world[1]-tpc.ActiveHalfHeight())
      miny = world[1]-tpc.ActiveHalfHeight();
    if (maxy<world[1]+tpc.ActiveHalfHeight())
      maxy = world[1]+tpc.ActiveHalfHeight();
    if (minz>world[2]-tpc.ActiveLength()/2.)
      minz = world[2]-tpc.ActiveLength()/2.;
    if (maxz<world[2]+tpc.ActiveLength()/2.)
      maxz = world[2]+tpc.ActiveLength()/2.;
//    std::cout<<t<<" "<<world[0]-tpc.ActiveHalfWidth()
//	     <<" "<<world[0]+tpc.ActiveHalfWidth()
//	     <<" "<<world[1]-tpc.ActiveHalfHeight()
//	     <<" "<<world[1]+tpc.ActiveHalfHeight()
//	     <<" "<<world[2]-tpc.ActiveLength()/2.
//	     <<" "<<world[2]+tpc.ActiveLength()/2.<<std::endl;
 
    TPCBox.push_back(new TBox(world[2]-tpc.ActiveLength()/2.,
			      world[1]-tpc.ActiveHalfHeight(),
			      world[2]+tpc.ActiveLength()/2.,
			      world[1]+tpc.ActiveHalfHeight()));
    TPCBox.back()->SetFillStyle(0);
    TPCBox.back()->SetLineStyle(2);
    TPCBox.back()->SetLineWidth(2);
    TPCBox.back()->SetLineColor(16);
    
    for (size_t p = 0; p<geo->Nplanes(t);++p){
      for (size_t w = 0; w<geo->Nwires(p,t); ++w){
	++nwires;
	++nwires_tpc[t];
	double xyz0[3];
	double xyz1[3];
	unsigned int c = 0;
//	if ((t==7&&p==0&&w==192)||
//	    (t==7&&p==1&&w==112)||
//	    (t==7&&p==2&&w==0)){
//	if (true){
        if ((t==2||t==6||t==10)&&p==0&&w%10==0){
	  geo->WireEndPoints(c,t,p,w,xyz0,xyz1);
	  Wires.push_back(new TLine(xyz0[2],xyz0[1],xyz1[2],xyz1[1]));
	}
	//std::cout<<t<<" "<<p<<" "<<w<<" "<<xyz0[0]<<" "<<xyz0[1]<<" "<<xyz0[2]<<std::endl;
      }
    }
  }

  TH2F *frame = new TH2F("frame",";z (cm);y (cm)",100,minz,maxz,100,miny,maxy);
  frame->SetStats(0);
  frame->Draw();
  for (auto box: TPCBox) box->Draw();
  for (auto wire: Wires) wire->Draw();
  can->Print("wires.pdf");
  std::cout<<"N wires = "<<nwires<<std::endl;
  for (size_t i = 0; i<geo->NTPC(); ++i){
    std::cout<<"TPC "<<i<<" has "<<nwires_tpc[i]<<" wires"<<std::endl;
  }

  //CRT
  std::vector<TBox*> CRTBox0;
  std::vector<TBox*> CRTBox1;
  std::vector<std::vector<TBox*>> CRTStrips(2);
  std::vector<int> modules0;
  std::vector<int> modules1;
  std::vector<TText *> text0;
  std::vector<TText *> text1;
  std::vector<double> module_x(32);
  std::vector<double> module_y(32);
  std::vector<double> module_z(32);
  std::vector<double> pixel_x(32);
  std::vector<double> pixel_y(32);
  std::vector<double> pixel_z(32);
  for (unsigned int i = 0; i < geo->NAuxDets(); ++i){
    auto& auxdet = geo->AuxDet(i);
    double xyz[3];
    auxdet.GetCenter(xyz);
    //std::cout<<"Aux "<<i<<" "<<xyz[0]<<" "<<xyz[1]<<" "<<xyz[2]<<" "<<auxdet.HalfWidth1()<<" "<<auxdet.HalfHeight()<<" "<<auxdet.Length()/2<<std::endl;
    double auxdet0[3] = {-auxdet.HalfWidth1(), auxdet.HalfHeight(), -auxdet.Length()/2};
    double world0[3];
    auxdet.LocalToWorld(auxdet0, world0);
    double auxdet1[3] = {auxdet.HalfWidth1(), auxdet.HalfHeight(), auxdet.Length()/2};
    double world1[3];
    auxdet.LocalToWorld(auxdet1, world1);
    module_x[i] = (world0[0]+world1[0])/2;
    module_y[i] = (world0[1]+world1[1])/2;
    module_z[i] = (world0[2]+world1[2])/2;
    std::cout<<"CRT module "<<i<<" x = "<<module_x[i]<<" y = "<<module_y[i]<<" z = "<<module_z[i]<<std::endl;
    if (xyz[2] < 0){ //front
      CRTBox0.push_back(new TBox(world0[0],world0[1],world1[0],world1[1]));
      CRTBox0.back()->SetFillStyle(0);
      //CRTBox0.back()->SetLineStyle(i/8+1);
      CRTBox0.back()->SetLineColor(i%8+1);
      if (i%8+1==5){
        CRTBox0.back()->SetLineColor(kOrange);
      }
      CRTBox0.back()->SetLineWidth(2);
      modules0.push_back(i);
      text0.push_back(new TText((world0[0]+world1[0])/2,(world0[1]+world1[1])/2,Form("%d",i)));
      text0.back()->SetTextColor(i%8+1);
      if (i%8+1==5){
        text0.back()->SetTextColor(kOrange);
      }

    }
    else{
      CRTBox1.push_back(new TBox(world0[0],world0[1],world1[0],world1[1]));
      CRTBox1.back()->SetFillStyle(0);
      //CRTBox1.back()->SetLineStyle(i/8+1);
      CRTBox1.back()->SetLineColor(i%8+1);
      if (i%8+1==5){
        CRTBox1.back()->SetLineColor(kOrange);
      }
      CRTBox1.back()->SetLineWidth(2);
      modules1.push_back(i);
      text1.push_back(new TText((world0[0]+world1[0])/2,(world0[1]+world1[1])/2,Form("%d",i)));
      text1.back()->SetTextColor(i%8+1);
      if (i%8+1==5){
        text1.back()->SetTextColor(kOrange);
      }
    }
    if (i==0){
      //std::cout<<"Aux "<<i<<" has "<<auxdet.NSensitiveVolume()<<std::endl;
    }
  }

  //Get the pixel map
  std::vector< std::pair <int,int> > vect; 
  vect.push_back( std::make_pair(1,2)); 
  vect.push_back( std::make_pair(0,2)); 
  vect.push_back( std::make_pair(15,13)); 
  vect.push_back( std::make_pair(14,13)); 
  vect.push_back( std::make_pair(14,12)); 
  vect.push_back( std::make_pair(9,11)); 
  vect.push_back( std::make_pair(9,10)); 
  vect.push_back( std::make_pair(8,10)); 
  vect.push_back( std::make_pair(7,5)); 
  vect.push_back( std::make_pair(6,5)); 
  vect.push_back( std::make_pair(6,4)); 
  vect.push_back( std::make_pair(1,3)); 
  vect.push_back( std::make_pair(0,3)); 
  vect.push_back( std::make_pair(15,12)); 
  vect.push_back( std::make_pair(8,11)); 
  vect.push_back( std::make_pair(7,4)); 
  vect.push_back( std::make_pair(17,18)); 
  vect.push_back( std::make_pair(16,18)); 
  vect.push_back( std::make_pair(31,29)); 
  vect.push_back( std::make_pair(30,29)); 
  vect.push_back( std::make_pair(30,28)); 
  vect.push_back( std::make_pair(25,27)); 
  vect.push_back( std::make_pair(25,26)); 
  vect.push_back( std::make_pair(24,26)); 
  vect.push_back( std::make_pair(23,21)); 
  vect.push_back( std::make_pair(22,21)); 
  vect.push_back( std::make_pair(22,20)); 
  vect.push_back( std::make_pair(17,19)); 
  vect.push_back( std::make_pair(16,19)); 
  vect.push_back( std::make_pair(31,28)); 
  vect.push_back( std::make_pair(24,27)); 
  vect.push_back( std::make_pair(23,20));
  //module map
  std::vector<size_t> fChannelMap;
  fChannelMap.push_back(24);
  fChannelMap.push_back(25);
  fChannelMap.push_back(30);
  fChannelMap.push_back(18);
  fChannelMap.push_back(15);
  fChannelMap.push_back(7);
  fChannelMap.push_back(13);
  fChannelMap.push_back(12);
  fChannelMap.push_back(11);
  fChannelMap.push_back(10);
  fChannelMap.push_back(6);
  fChannelMap.push_back(14);
  fChannelMap.push_back(19);
  fChannelMap.push_back(31);
  fChannelMap.push_back(26);
  fChannelMap.push_back(27);
  fChannelMap.push_back(22);
  fChannelMap.push_back(23);
  fChannelMap.push_back(29);
  fChannelMap.push_back(17);
  fChannelMap.push_back(8);
  fChannelMap.push_back(0);
  fChannelMap.push_back(3);
  fChannelMap.push_back(2);
  fChannelMap.push_back(5);
  fChannelMap.push_back(4);
  fChannelMap.push_back(1);
  fChannelMap.push_back(9);
  fChannelMap.push_back(16);
  fChannelMap.push_back(28);
  fChannelMap.push_back(20);
  fChannelMap.push_back(21);

  for (size_t i = 0; i<32; ++i){
    int module[2];
    double x[2];
    double y[2];
    double z[2];
    double x0[2];
    double x1[2];
//    double y0[2];
//    double y1[2];
    module[0] = fChannelMap[vect[i].first];
    module[1] = fChannelMap[vect[i].second];
    for (size_t j = 0; j<2; ++j){
      auto& auxdet = geo->AuxDet(module[j]);
      double xyz[3];
      auxdet.GetCenter(xyz);
      x[j] = xyz[0];
      y[j] = xyz[1];
      z[j] = xyz[2];
      double auxdet0[3] = {-auxdet.HalfWidth1(), auxdet.HalfHeight(), -auxdet.Length()/2};
      double world0[3];
      auxdet.LocalToWorld(auxdet0, world0);
      double auxdet1[3] = {auxdet.HalfWidth1(), auxdet.HalfHeight(), auxdet.Length()/2};
      double world1[3];
      auxdet.LocalToWorld(auxdet1, world1);
      x0[j] = world0[0];
      x1[j] = world1[0];
//      y0[j] = world0[1];
//      y1[j] = world1[1];
    }
    if (std::abs(x0[0]-x1[0])<std::abs(x0[1]-x1[1])){
      pixel_x[i] = x[0];
      pixel_y[i] = y[1];
    }
    else{
      pixel_x[i] = x[1];
      pixel_y[i] = y[0];
    }
    pixel_z[i] = (z[0]+z[1])/2;
  }
   
  TLatex tex;
  tex.SetTextSize(0.02);

  TH2F *frcrt0 = new TH2F("frcrt0","Upstream;x(cm);y(cm)",100,-300, 600, 100, -100, 650);
  TH2F *frcrt1 = new TH2F("frcrt1","Downstream;x(cm);y(cm)",100,-400, 450, 100, -200, 600);
  frcrt0->SetStats(0);
  frcrt1->SetStats(0);
  TCanvas *cancrt = new TCanvas("cancrt","cancrt",1600,800);
  cancrt->Divide(2,1);
  cancrt->cd(1);
  frcrt0->DrawCopy();
  TLegend *leg0 = new TLegend(0.7,0.1,0.9,0.9);
  leg0->SetFillStyle(0);
  for (size_t i = 0; i< CRTBox0.size(); ++i){
    CRTBox0[i]->Draw();
    text0[i]->Draw();
    leg0->AddEntry(CRTBox0[i], Form("Module %d",modules0[i]), "l");
  }
  //leg0->Draw();
  for (int i = 0; i<16; ++i){
    //tex.DrawLatex(pixel_x[i], pixel_y[i], Form("CH %d",i));
  }
  TLatex tt;
  tt.SetTextSize(0.02);
  for (unsigned int i = 0; i < geo->NAuxDets(); ++i){
    auto& auxdet = geo->AuxDet(i);
    double xyz[3];
    auxdet.GetCenter(xyz);
    if (xyz[2] < 0){ //front
      if (auxdet.HalfWidth1()>auxdet.HalfHeight()){//horizontal
        for (int j = 0; j<1; ++j){
          auto& auxdetsen = auxdet.SensitiveVolume(j);
          //std::cout<<i<<" "<<j<<" "<<auxdetsen.HalfWidth1()<<" "<<auxdetsen.HalfHeight()<<" "<<auxdetsen.Length()/2<<std::endl;
          double auxdetsen0[3] = {0,0,auxdetsen.Length()/2-20};
          double world0[3];
          auxdetsen.LocalToWorld(auxdetsen0,world0);
          //std::cout<<i<<" "<<j<<" "<<world0[0]<<" "<<world0[1]<<std::endl;
          tt.SetTextColor(i%8+1);
          tt.DrawLatex(world0[0], world0[1], Form("%d",j));
        }
      }
    }
  }

  cancrt->cd(2);
  frcrt1->DrawCopy();
  TLegend *leg1 = new TLegend(0.7,0.1,0.9,0.9);
  leg1->SetFillStyle(0);
  for (size_t i = 0; i< CRTBox1.size(); ++i){
    CRTBox1[i]->Draw();
    text1[i]->Draw();
    leg1->AddEntry(CRTBox1[i], Form("Module %d",modules1[i]), "l");
  }
  //leg1->Draw();
  for (int i = 16; i<32; ++i){
    //tex.DrawLatex(pixel_x[i], pixel_y[i], Form("CH %d",i));
  }

  for (unsigned int i = 0; i < geo->NAuxDets(); ++i){
    auto& auxdet = geo->AuxDet(i);
    double xyz[3];
    auxdet.GetCenter(xyz);
    if (xyz[2] > 0){ //front
      for (int j = 0; j<1; ++j){
        auto& auxdetsen = auxdet.SensitiveVolume(j);
        //std::cout<<i<<" "<<j<<" "<<auxdetsen.HalfWidth1()<<" "<<auxdetsen.HalfHeight()<<" "<<auxdetsen.Length()/2<<std::endl;
        double auxdetsen0[3] = {0,0,auxdetsen.Length()/2-20};
        double world0[3];
        auxdetsen.LocalToWorld(auxdetsen0,world0);
        //std::cout<<i<<" "<<j<<" "<<world0[0]<<" "<<world0[1]<<std::endl;
        tt.SetTextColor(i%8+1);
        tt.DrawLatex(world0[0], world0[1], Form("%d",j));
      }
    }
  }

  tt.SetTextColor(1);
  for (int i = 0; i<2; ++i){
    auto& auxdet = geo->AuxDet(i*2);
    for (size_t j = 0; j<auxdet.NSensitiveVolume(); ++j){
      auto& auxdetsen = auxdet.SensitiveVolume(j);
      double world0[3] = {0};
      double world1[3] = {0};
      double auxdet0[3] = {-auxdetsen.HalfWidth1(),
                           -auxdetsen.HalfHeight(),
                           0};
      double auxdet1[3] = {auxdetsen.HalfWidth1(),
                           auxdetsen.HalfHeight(),
                           0};
      auxdetsen.LocalToWorld(auxdet0, world0);
      auxdetsen.LocalToWorld(auxdet1, world1);
//      std::cout<<world0[0]<<" "<<world0[1]<<" "<<world0[2]<<std::endl;
//      std::cout<<world1[0]<<" "<<world1[1]<<" "<<world1[2]<<std::endl;
      if (i==0){
        CRTStrips[i].push_back(new TBox(world0[1], world0[2], world1[1], world1[2]));
      }
      else {
        CRTStrips[i].push_back(new TBox(world0[0], world0[2], world1[0], world1[2]));
      }
      CRTStrips[i].back()->SetFillStyle(0);
      CRTStrips[i].back()->SetLineWidth(2);
    }
  }
  //std::cout<<CRTStrips[0].size()<<" "<<CRTStrips[1].size()<<std::endl;
  TH2D *frstp[2];
  TCanvas *cstp[2];
  for (int i = 0; i<2; ++i){
    cstp[i] = new TCanvas(Form("cstp_%d",i), Form("cstp_%d",i), 1000,500);
    double minx = 1e10;
    double maxx = -1e10;
    double miny = 1e10;
    double maxy = -1e10;
    for (size_t j = 0; j<CRTStrips[i].size(); ++j){
      if (minx>CRTStrips[i][j]->GetX1()) minx = CRTStrips[i][j]->GetX1();
      if (minx>CRTStrips[i][j]->GetX2()) minx = CRTStrips[i][j]->GetX2();
      if (maxx<CRTStrips[i][j]->GetX1()) maxx = CRTStrips[i][j]->GetX1();
      if (maxx<CRTStrips[i][j]->GetX2()) maxx = CRTStrips[i][j]->GetX2();
      if (miny>CRTStrips[i][j]->GetY1()) miny = CRTStrips[i][j]->GetY1();
      if (miny>CRTStrips[i][j]->GetY2()) miny = CRTStrips[i][j]->GetY2();
      if (maxy<CRTStrips[i][j]->GetY1()) maxy = CRTStrips[i][j]->GetY1();
      if (maxy<CRTStrips[i][j]->GetY2()) maxy = CRTStrips[i][j]->GetY2();
    }
    frstp[i] = new TH2D(Form("frstp_%d",i),Form("frstp_%d",i),100,minx-10,maxx+10,100,miny-1,maxy+1);
    frstp[i]->SetStats(0);
    if (i==0) frstp[i]->SetTitle("Horizontal module (offline module 0);y(cm);z(cm)");
    else frstp[i]->SetTitle("Vertical module (offline module 2);x(cm);z(cm)");
    frstp[i]->Draw();
    for (size_t j = 0; j<CRTStrips[i].size(); ++j){
      CRTStrips[i][j]->Draw();
    }
    auto& auxdet = geo->AuxDet(i*2);
    for (size_t j = 0; j<auxdet.NSensitiveVolume(); ++j){
      auto& auxdetsen = auxdet.SensitiveVolume(j);
      double xyz[3];
      auxdetsen.GetCenter(xyz);
      if (i==0) tt.DrawLatex(xyz[1]-0.5, xyz[2], Form("%d",int(j)));
      else tt.DrawLatex(xyz[0]-0.5, xyz[2], Form("%d",int(j)));
    }
    cstp[i]->Print(Form("cstp_%d.png",i));
  }
  std::ofstream outfile("pixel.txt");
  for (int i = 0; i<32; ++i){
    outfile<<pixel_x[i]<<" "<<pixel_y[i]<<" "<<pixel_z[i]<<std::endl;
  }
  outfile.close();

  cancrt->Print("crt.pdf");
  cancrt->Print("crt.png");

  //convert channel number to t/p/w
  outfile.clear();
  outfile.open("channelmap.txt");
  for (size_t i = 0; i<geo->Nchannels(); ++i){
    auto wire = geo->ChannelToWire(i)[0];
    outfile<<i<<" "<<wire.TPC<<" "<<wire.Plane<<" "<<wire.Wire<<std::endl;
  }

}

void dune::CheckGeometry::reconfigure(fhicl::ParameterSet const & p)
{
  // Implementation of optional member function here.
}

DEFINE_ART_MODULE(dune::CheckGeometry)
