%module LSDHCALDB
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "stdint.i"

 %{
#include "DIFDBManager.h"
#include "TextDIFDBManager.h"
#include "OracleDIFDBManager.h"
#include "WebDIFDBServer.h"
 %}

%include "DIFDBManager.h"
%include "TextDIFDBManager.h"
%include "OracleDIFDBManager.h"
%include "WebDIFDBServer.h"
