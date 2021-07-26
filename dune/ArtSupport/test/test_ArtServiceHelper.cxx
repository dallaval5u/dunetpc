// test_ArtServiceHelper.cxx

//#include "AXService/ArtServiceHelper.h"
#include "dune/ArtSupport/ArtServiceHelper.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art_root_io/TFileService.h"
#include "art/Framework/Services/System/TriggerNamesService.h"

#include "TFile.h"
#include "TROOT.h"

using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::ofstream;
using art::ServiceHandle;

#undef NDEBUG
#include <cassert>

int test_ArtServiceHelper(int opt) {
  const string myname = "test_ArtServiceHelper: ";
  cout << myname << "Starting test" << endl;
#ifdef NDEBUG
  cout << myname << "NDEBUG must be off." << endl;
  abort();
#endif
  string line = "-----------------------------";
  string scfg;
  string fclfile = "test_ArtServiceHelper.fcl";
  string datafile = "mytest.root";

  if ( opt == 1 ) {
    ofstream fout(fclfile);
    fout << "services: {" << endl;
    fout << "  RandomNumberGenerator: {}" << endl;
    fout << "  TFileService: {fileName: \"" << datafile << "\"}" << endl;
    fout << "}" << endl;
  }
  ArtServiceHelper::load(fclfile);

  cout << myname << line << endl;
  cout << myname << "Check RandomNumberGenerator is available." << endl;
  ServiceHandle<art::RandomNumberGenerator> hrng;
  cout << myname << "  " << &*hrng << endl;

  cout << myname << line << endl;
  cout << myname << "Check TFileService is available." << endl;
  ServiceHandle<art::TFileService> htf;
  cout << myname << "  " << &*hrng << endl;
  cout << myname << "  TFile name: " << htf->file().GetName() << endl;
  //assert( htf->file().GetName() == datafile );

  cout << myname << line << endl;
  cout << myname << "Done." << endl;
  return 0;
}

int main(int argc, char** argv) {
  const string myname = "main: ";
  int opt = 1;
  if ( argc > 1 ) {
    opt = -1;
    istringstream ssopt(argv[1]);
    ssopt >> opt;
    if ( opt < 0 ) {
      cout << "Usage: test_ArtServiceHelper [OPT]" << endl;
      cout << "  OPT = 0 to skip test." << endl;
      cout << "        1 to load from string." << endl;
      cout << "        2 to load all services from a file." << endl;
    }
  }

  if ( opt == 0 ) return 0;
  int rstat = test_ArtServiceHelper(opt);
  return rstat;
}
