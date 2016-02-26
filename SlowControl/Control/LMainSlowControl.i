%module LMainSlowControl
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "stdint.i"


 %{
using std::string;
#include "DimSlowControl.h"
 %}

%include "DimSlowControl.h"
