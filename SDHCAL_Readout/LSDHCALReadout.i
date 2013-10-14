%module LSDHCALReadout
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "stdint.i"

 %{
#include "CCCManager.h"
#include "CCCServer.h"
#include "DIFServer.h"
#include "FtdiUsbDriver.h"
 %}

%include "CCCManager.h"
%include "CCCServer.h"
%include "DIFServer.h"
%include "FtdiUsbDriver.h"


