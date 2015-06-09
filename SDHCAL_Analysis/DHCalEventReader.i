%module DHCalEventReader
%include "std_string.i"
%include "std_vector.i"
%include "stdint.i"
 
%{
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include "RecoHit.h"
#include "RECOCluster.h"
#include "RecoCandTk.h"
#include "RecoPoint.h"
#include "Amas.h"
#include "Shower.h"
#include "HC.h"
#include "HTImage.h"
#include "DHCALAnalyzer.h"
#include "ChamberAnalyzer.h"
#include "ShowerAnalyzer.h"
#include "TrackAnalyzer.h"
#include "RawAnalyzer.h"

#include "FilterAnalyzer.h"
#include "IO/LCRunListener.h"
#include "IO/LCEventListener.h"
 %}

%include "DHCalEventReader.h"
%include "DCHistogramHandler.h"
%include "RecoHit.h"
%include "RECOCluster.h"
%include "RecoCandTk.h"
%include "RecoPoint.h"
%include "Amas.h"
%include "Shower.h"
%include "HC.h"
%include "HTImage.h"
%include "DHCALAnalyzer.h"
%include "ChamberAnalyzer.h"
%include "ShowerAnalyzer.h"
%include "TrackAnalyzer.h"
%include "RawAnalyzer.h"
%include "FilterAnalyzer.h"
%include "IO/LCRunListener.h"
%include "IO/LCEventListener.h"



