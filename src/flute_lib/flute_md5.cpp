/* $Id: flute_md5.cpp,v 1.2 2005/05/12 16:03:44 moi Exp $ */
/*
 *  Copyright (c) 2003-2004 INRIA - All rights reserved
 *  main authors: Christoph Neumann - christoph.neumann@inrialpes.fr
 *		  Vincent Roca - vincent.roca@inrialpes.fr
 *		  Julien Laboure - julien.laboure@inrialpes.fr
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


/*
 * flute_md5.c
 *
 *	Tools for md5 manipulations...
 */

#include "flute_includes.h"

#ifdef OPENSSL /* { */
/* MD5sum */

#include <openssl/md5.h>
#include <openssl/bio.h>
#include <openssl/evp.h>


void base64enc(char *inbuffer, char ** outbuffer)
{

	BIO *mbio,*b64bio,*bio;
	char *p;

	mbio=BIO_new(BIO_s_mem());
	b64bio=BIO_new(BIO_f_base64());
	bio=BIO_push(b64bio,mbio);
	
	BIO_write(bio,inbuffer,MD5_DIGEST_LENGTH);

	BIO_flush(bio);

	BIO_ctrl(mbio,BIO_CTRL_INFO,0,(char *)&p);

	memcpy(*outbuffer,p,MD5BASE64_LENGTH);
	
	BIO_free_all(bio);

}


void md5sum_calculate(int fd, unsigned char ** digest)
{
	unsigned char buf[1024];
	MD5_CTX ctx;
	int n;
	unsigned char	*md5_digest		= (unsigned char*)calloc(1, MD5_DIGEST_LENGTH);
	
	lseek(fd, 0, SEEK_SET); /*rewind*/

	MD5_Init(&ctx); 
	
	while ((n = read(fd, buf,  sizeof(buf))) > 0) 
		MD5_Update(&ctx, buf, n);
	MD5_Final(md5_digest, &ctx);

	base64enc((char *) md5_digest,(char **) digest);
	
	free(md5_digest);
}

#endif /* } */
