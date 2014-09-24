%module DHCalEventReader
%include "std_string.i"
%include "std_vector.i"
 
%{
#include "DHCalEventReader.h"
#include "DCHistogramHandler.h"
#include "DHCALAnalyzer.h"
#include "RawAnalyzer.h"

#include "FilterAnalyzer.h"
#include "IO/LCRunListener.h"
#include "IO/LCEventListener.h"
 %}

%include "DHCalEventReader.h"
%include "DCHistogramHandler.h"
%include "DHCALAnalyzer.h"
%include "RawAnalyzer.h"
%include "FilterAnalyzer.h"
%include "IO/LCRunListener.h"
%include "IO/LCEventListener.h"



