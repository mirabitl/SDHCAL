%module LSDHCALClient
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "stdint.i"

 %{
#include "DIFDBManager.h"
#include "CCCClient.h"
#include "DIFClient.h"
#include "DIFMultiClient.h"
 %}

%include "CCCClient.h"
%include "DIFClient.h"
%include "DIFMultiClient.h"


