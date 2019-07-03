/***************************************************************************************************

  File:     sd_common_defs.h
  Author:   Laird
  Version:  0.1

  Description:  Project wide definitions 

  Copyright 2016, Laird Technologies (Laird)
  All Rights Reserved.

 This is UNPUBLISHED PROPRIETARY SOURCE CODE of Laird;
 the contents of this file may not be disclosed to third parties, copied
 or duplicated in any form, in whole or in part, without the prior
 written permission of Laird.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

***************************************************************************************************/
#pragma once

#include <stdint.h>     // use the standard integer definitions for fixed point numbers
#include <stdbool.h>    // use standard boolean types also
#include "sd_mem_diag.h"

/******************************************************************************
* Constants
******************************************************************************/

// MAC and BD ADDR
#define MAC_ADDR_BYTES ( 6 )
#define MAC_ADDR_AS_STR_BUF_SIZE (16)  // we only need 12 plus a null but make it N * 4
#define BD_ADDR_AS_STR_BUF_SIZE  (16)  // we only need 12 plus a null but make it N * 4

// Error handling
#define RETURN_ON_ERROR(err){ if(err != 0) { return( err ); } }

// Time related defines
#define SEC_PER_MIN   ( 60 )
#define MIN_PER_HOUR  ( 60 )
#define MS_PER_SECOND ( 1000 )
#define MS_PER_MIN ( SEC_PER_MIN * MS_PER_SECOND )
#define MS_PER_HOUR ( MS_PER_MIN * MIN_PER_HOUR)
