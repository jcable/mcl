/* $Id: FluteSender.cpp,v 1.2 2005/05/12 16:03:40 moi Exp $ */
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

#include "flute_includes.h"

FluteSender::FluteSender () {

	if ((flutecb->id = mcl_open("w")) < 0)
		EXIT(("FluteSender: ERROR, mcl_open(w) failed\n"))

	flutecb->mode = SEND;
	flutecb->fec = new FluteFec(flutecb);
	carousel = new FluteFileDeliveryCarousel(this);

}


FluteSender::~FluteSender () {

	/* cleanup everything */
	
	/* free FEC class if needed */
	if (flutecb->fec != NULL) {
		delete flutecb->fec;
		flutecb->fec = NULL;
	}

	/* free carousel class if needed */
	if (this->carousel != NULL) {
		delete this->carousel;
	}

}



bool FluteSender::setFECRatio (float fec_ratio) {


	flutecb->lock();
	
	flutecb->fec_ratio = fec_ratio;
	if (mcl_ctl(flutecb->id, MCL_OPT_FEC_RATIO, (void*)&flutecb->fec_ratio, sizeof(flutecb->fec_ratio))) {
		flutecb->unlock();
		return false;
	}
	else {
		flutecb->unlock();
		return true;
	}
}


float FluteSender::getFECRatio () {

	float result;
	flutecb->lock();
	result = flutecb->fec_ratio;
	flutecb->unlock();
	return result;

}

void FluteSender::setTTL (INT32 ttl) {

	flutecb->lock();
	flutecb->ttl = ttl;
	flutecb->unlock();

}

