/* $Id: flute_includes.h,v 1.2 2005/05/12 16:03:43 moi Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 */
 

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/MemBufFormatTarget.hpp>
#if 0
#include <xercesc/parsers/SAXParser.hpp>
#endif

#include "FluteAPI.h"
#include "macros.h"
#include "FluteTools.h"
#include "flute_lock.h"
#include "FluteFDT.h"
#include "FluteSDP.h"
#include "flute_md5.h"
#include "flute_cb.h"
#include "FluteFec.h"
#include "FluteFile.h"
#include "FluteFileList.h"

/* other constants, do not edit */
#define NOT_INITIALIZED		0
#define SEND			1
#define RECV			2

#define READY 0
#define IN_TX	1
