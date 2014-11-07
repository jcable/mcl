/* $Id: getopt.h,v 1.3 2005/05/12 16:03:31 moi Exp $ */
///////////////////////////////////////////////////////////////////////////////
//
//  FILE: getopt.h
//              
//      Header for the GetOption function
//
//  COMMENTS:
//
///////////////////////////////////////////////////////////////////////////////

// function prototypes
int GetOption (int argc, char** argv, char* pszValidOpts, char** ppszParam, int * optind);
