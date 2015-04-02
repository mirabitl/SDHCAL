%module LYDaq
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "stdint.i"


 %{
using std::string;
#include "YDaqManager.h"
#include "onedifhandler.h"

 %}

%include "YDaqManager.h"
%include "onedifhandler.h"


