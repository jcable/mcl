/* $Id: wincheck.cpp,v 1.1.1.1 2003/09/03 12:45:41 chneuman Exp $ */
/*
 *  Copyright (c) 1999-2001 INRIA - Universite Paris 6 - All rights reserved
 *  (main authors: Julien Laboure - julien.laboure@inrialpes.fr
 *                 Vincent Roca - vincent.roca@inrialpes.fr)
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
 * Wincheck.cpp
 *
 *	MCL validation tool for Windows Platforms
 */


#include <iostream>
#include <process.h>
#include <direct.h>
#include <stdlib.h>

using namespace std;



#define nbTests 9
#define TestnameLen 30

char SomeTests[nbTests][TestnameLen];
char WorkDir[_MAX_PATH];

int test_abort();
int test_demux_label();
int test_fcast_mcl_modes();
int test_fcast_xfert();
int test_fcast_single_xfert();
int test_multi_sessions();
int test_no_tx();
int test_vtm();
int test_wait_rx();
int test_wait_tx();


void initTests()
{
	strncpy(SomeTests[0], "test_abort", TestnameLen);
	strncpy(SomeTests[1], "test_demux_label", TestnameLen);
	strncpy(SomeTests[2], "test_fcast_mcl_modes", TestnameLen);
	strncpy(SomeTests[3], "test_fcast_xfert", TestnameLen);
	strncpy(SomeTests[4], "test_fcast_single_xfert", TestnameLen);
	strncpy(SomeTests[5], "test_multi_sessions", TestnameLen);
	strncpy(SomeTests[6], "test_no_tx", TestnameLen);
	strncpy(SomeTests[7], "test_vtm", TestnameLen);
	strncpy(SomeTests[8], "test_wait_rx", TestnameLen);
	strncpy(SomeTests[9], "test_wait_tx", TestnameLen);

	_getcwd( WorkDir, _MAX_PATH);
}


int ExecTest(int indice)
{
	int res = -1;
	_chdir(WorkDir);

	switch(indice)
	{
		case 0:
			res = test_abort();
			break;

		case 1:
			res = test_demux_label();
			break;
		case 2:
			res = test_fcast_mcl_modes();
			break;
		case 3:
			res = test_fcast_xfert();
			break;
		case 4:
			res = test_fcast_single_xfert();
			break;
		case 5:
			res = test_multi_sessions();
			break;
		case 6:
			res = test_no_tx();
			break;
		case 7:
			res = test_vtm();
			break;
		case 8:
			res = test_wait_rx();
			break;
		case 9:
			res = test_wait_tx();
			break;
		default:
			res=1;
			break;

	}
	return res;
}



int main(int argc, char* argv[])
{
	int result = -1;
	initTests();

	for(int i=0; i<nbTests; i++)
	{
		cout << endl << "------>   Running Test " << SomeTests[i] << endl;
		result = ExecTest(i);
		if(result !=0)
		{
			cout << endl << "****** ERROR: Test " << SomeTests[i] << " failed! Aborting... ******" << endl;
			exit(-1);
		}
		else
		{
			cout << "<------   Test " << SomeTests[i] << ": Success!!!" << endl;
		}
	}

	cout << endl <<"****** All tests succeeded! Validation OK. ******" << endl;
	system("pause");
	return 0;
}



// Test the MCL with an application that opens/sends/aborts
int test_abort()
{
	return _spawnl(_P_WAIT, "test_abort", "test_abort", NULL);
}



// test the MCL with an application that uses different sessions
// with different demux labels.
// An object is sent on each session, containing the demux label value.
int test_demux_label()
{
	int pid1, pid2 = -1;
	int res = -1;

//	cout<<"WARNING: test skipped !!!!!!";
//	return 0;


	pid1 = _spawnl(_P_NOWAIT, "test_demux_label1_rx", "test_demux_label1_rx", NULL);
	pid2 = _spawnl(_P_NOWAIT, "test_demux_label1_tx", "test_demux_label1_tx", NULL);

	if(pid1==-1 || pid2==-1)
	{
		cout << "_spawnl Failure in test_demux_label" << endl;
		return -1;
	}
	
	if( _cwait( &res, pid1, NULL)!=pid1 || res!=0 ) // Error RX!!
	{
		cout << "DEMUX_LABEL Recv Failed" << endl;
		return -1;
	}
	if( _cwait( &res, pid2, NULL)!=pid2 || res!=0 ) // Error TX!
	{
		cout << "DEMUX_LABEL Send Failed" << endl;
		return -1;
	}
	return res;
}




int test_fcast_mcl_modes()
{
	int pid1, pid2 = -1;
	int res = 0;

	char *file="./Other_Files/i-am-a-BIG.file";


	cout <<endl<<"** Multicast tests only..."<<endl;

	// --- Speed Optmimization Test... -------------------------------
	cout <<"--- Speed Optmimization Test..." << endl;
	_chdir("..\\..\\fcast_test\\recv");
	system("del /F /S /Q  ..\\recv");

	pid1 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-recv", "-never", "-phigh", 
					"-ospeed", "-a226.1.2.3/9991", NULL);
	
	_chdir("..\\send");

	pid2 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-v0", "-send", "-phigh", "-ospeed", 
					"-a226.1.2.3/9991", file, NULL);

	if(pid1==-1 || pid2==-1)
	{
		cout << "_spawnl Failure in test_fcast_mcl_modes" << endl;
		return -1;
	}

	if( _cwait( &res, pid1, NULL)!=pid1 || res!=0 ) // Error RX!!
	{
		cout << "FCAST Recv Failed" << endl;
		_cwait( &res, pid2, NULL);
		return -1;
	}

	if( _cwait( &res, pid2, NULL)!=pid2  || res!=0 ) // Error TX!
	{
		cout << "FCAST Send Failed" << endl;
		return -1;
	}
	else
	{
		cout << "-- Speed Optmimization Test OK!\n\n\n";
	}
	/* TODO DIFF files ???? */


	// --- Space Optmimization Test... -------------------------------
	cout<<"--- Space Optmimization Test...\n";
	system("del /F /S /Q  ..\\recv");
	_chdir("..\\recv");


	pid1 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-recv", "-never", "-phigh", 
					"-ospace", "-a226.2.3.4/9992", NULL);
	
	_chdir("..\\send");

	pid2 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-v0", "-send", "-phigh", "-ospace", 
					"-a226.2.3.4/9992", file, NULL);

	if(pid1==-1 || pid2==-1)
	{
		cout << "_spawnl Failure in test_fcast_mcl_modes" << endl;
		return -1;
	}

	if( _cwait( &res, pid1, NULL)!=pid1 || res!=0 ) // Error RX!!
	{
		cout << "FCAST Recv Failed" << endl;
		_cwait( &res, pid2, NULL);
		return -1;
	}

	if( _cwait( &res, pid2, NULL)!=pid2  || res!=0 ) // Error TX!
	{
		cout << "FCAST Send Failed" << endl;
		return -1;
	}
	else
	{
		cout << "-- Space Optmimization Test OK!\n\n\n";
	}


	// --- CPU Optmimization Test... -------------------------------
	cout<<"--- CPU Optmimization Test...\n";
	system("del /F /S /Q  ..\\recv");
	_chdir("..\\recv");


	pid1 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-recv", "-never", "-phigh", 
					"-ocpu", "-a226.3.4.5/9993", NULL);

	
	_chdir("..\\send");
	pid2 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-v0", "-send", "-phigh", "-ocpu", 
					"-a226.3.4.5/9993", file, NULL);

	if(pid1==-1 || pid2==-1)
	{
		cout << "_spawnl Failure in test_fcast_mcl_modes" << endl;
		return -1;
	}

	if( _cwait( &res, pid1, NULL)!=pid1 || res!=0 ) // Error RX!!
	{
		cout << "FCAST Recv Failed" << endl;
		_cwait( &res, pid2, NULL);
		return -1;
	}

	if( _cwait( &res, pid2, NULL)!=pid2  || res!=0 ) // Error TX!
	{
		cout << "FCAST Send Failed" << endl;
		return -1;
	}
	else
	{
		cout << "-- CPU OptmimizationTest OK!\n\n\n";
	}

	return 0; // All OK
}



int test_fcast_xfert()
{

	int pid1, pid2 = -1;
	int res = -1;

	cout<<endl<<"** Multicast tests first..."<<endl;

	_chdir("..\\..\\fcast_test\\recv");
	system("del /F /S /Q  ..\\recv");

	pid1 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-recv", "-never", "-phigh", 
					"-ospeed", "-a226.1.2.3/9991", NULL);
	
	_chdir("..\\send");

	pid2 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-send", "-phigh", "-ospeed", 
					"-a226.1.2.3/9991", "-R", "./", NULL);

	if(pid1==-1 || pid2==-1)
	{
		cout << "_spawnl Failure in test_fcast_xfert" << endl;
		return -1;
	}

	if( _cwait( &res, pid1, NULL)!=pid1 || res!=0 ) // Error RX!!
	{
		cout << "FCAST Recv Failed" << endl;
		_cwait( &res, pid2, NULL);
		return -1;
	}

	if( _cwait( &res, pid2, NULL)!=pid2  || res!=0 ) // Error TX!
	{
		cout << "FCAST Send Failed" << endl;
		return -1;
	}
	else
	{
		cout << "-- Multicast Test OK!\n\n\n";
	}
	/* TODO DIFF files ???? */



	cout<<endl<<"And unicast tests..."<<endl;

	system("del /F /S /Q  ..\\recv");
	_chdir("..\\recv");


	pid1 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-recv", "-never",
					"-a127.0.0.1/9998", NULL);
	
	_chdir("..\\send");

	pid2 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-send", 
					"-a127.0.0.1/9998", "-R", "./", NULL);

	if(pid1==-1 || pid2==-1)
	{
		cout << "_spawnl Failure in test_fcast_xfert" << endl;
		return -1;
	}

	if( _cwait( &res, pid1, NULL)!=pid1 || res!=0 ) // Error RX!!
	{
		cout << "FCAST Recv Failed" << endl;
		_cwait( &res, pid2, NULL);
		return -1;
	}


	// do not test send_val return value here as in unicast,
	// the sender is always stopped as soon as the receiver leaves
	// with a sendmsg error. This is normal so ignore it and only
	// rely on diff tests
	if( _cwait( &res, pid2, NULL)!=pid2  /* || res!=0 */ ) // Error TX!
	{
		cout << "FCAST Send Failed" << endl;
		return -1;
	}
	else
	{
		cout << "-- Unicast Test OK!\n\n\n";
	}

	return 0;
}



int test_fcast_single_xfert()
{

	int pid1, pid2 = -1;
	int res = -1;

	cout<<endl<<"** Fcast test in progress, sending a single file..."<<endl;

	_chdir("..\\..\\fcast_test\\recv");
	system("del /F /S /Q  ..\\recv");

	pid1 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-recv", "-never", "-phigh", 
					"-ospeed", "-a226.1.2.3/9991", NULL);
	
	_chdir("..\\send\\Other_Files");

	pid2 = _spawnl(	_P_NOWAIT, "..\\..\\..\\..\\..\\bin\\win32\\fcast.exe", 
					"fcast", "-send", "-phigh", "-ospeed", 
					"-a226.1.2.3/9991", "i-am-a-BIG.file", NULL);

	if(pid1==-1 || pid2==-1)
	{
		cout << "_spawnl Failure in test_fcast_xfert" << endl;
		return -1;
	}

	if( _cwait( &res, pid1, NULL)!=pid1 || res!=0 ) // Error RX!!
	{
		cout << "FCAST Recv Failed" << endl;
		_cwait( &res, pid2, NULL);
		return -1;
	}

	if( _cwait( &res, pid2, NULL)!=pid2  || res!=0 ) // Error TX!
	{
		cout << "FCAST Send Failed" << endl;
		return -1;
	}
	else
	{
		cout << "-- Fcast single file xfert Test OK!\n\n\n";
	}
	/* TODO DIFF files ???? */

	return 0;
}




// Test the MCL with a multi-session application
int test_multi_sessions()
{
	int res=-1;

	res=_spawnl(_P_WAIT, "test_multi_sessions1.exe", "test_multi_sessions1", NULL);
	if(res!=0)
	{
		cout<<"Test test_multi_sessions1 FAILED! ("<<res<<")\n\n";
		return -1;
	}

// TODO : fix & restore
cout<<"Test test_multi_sessions2 SKIPPED...\n\n";
return 0;

	res=_spawnl(_P_WAIT, "test_multi_sessions2", "test_multi_sessions2", NULL);
	if(res!=0)
	{
		cout<<"Test test_multi_sessions2 FAILED! ("<<res<<")\n\n";
		return -1;
	}

	return 0;
}



// test the MCL with an application that only performs 
// mcl_open and immediately mcl_close
int test_no_tx()
{
	return _spawnl(_P_WAIT, "test_no_tx.exe", "test_no_tx", NULL);
}



// test the MCL with an application that uses two sessions
// each of them using the Virtual Tx Memory service
// An object is sent on each session, containing the session value.
int test_vtm()
{
	int pid1, pid2 = -1;
	int res = -1;

	pid1 = _spawnl(_P_NOWAIT, "test_vtm_rx", "test_vtm_rx", NULL);
	pid2 = _spawnl(_P_NOWAIT, "test_vtm_tx", "test_vtm_tx", NULL);

	if(pid1==-1 || pid2==-1)
	{
		cout << "_spawnl Failure in test_vtm" << endl;
		return -1;
	}
	
	if( _cwait( &res, pid1, NULL)!=pid1 || res!=0 ) // Error RX!!
	{
		cout << "VTM (Virtual Tx Memory service) recv failed" << endl;
		return -1;
	}
	if( _cwait( &res, pid2, NULL)!=pid2 || res!=0 ) // Error TX!
	{
		cout << "VTM (Virtual Tx Memory service) send failed" << endl;
		return -1;
	}
	return res;
}



// test the wait_event at the receiver
//
// scenario 1:
// The source tx a single ADU, waits the end (wait event),
// then issues a close.
// The receiver receives this ADU, waits the end (wait event),
// then issues a close.
int test_wait_rx()
{
	int pid1, pid2 = -1;
	int res = -1;

	pid1 = _spawnl(_P_NOWAIT, "test_wait_rx_sender", "test_wait_rx_sender", NULL);
	pid2 = _spawnl(_P_NOWAIT, "test_wait_rx_receiver", "test_wait_rx_receiver", NULL);

	if(pid1==-1 || pid2==-1)
	{
		cout << "_spawnl Failure in test_wait_rx" << endl;
		return -1;
	}
	
	if( _cwait( &res, pid1, NULL)!=pid1 || res!=0 ) // Error RX!!
	{
		cout << "wait_rx Recv Failed" << endl;
		return -1;
	}
	if( _cwait( &res, pid2, NULL)!=pid2 || res!=0 ) // Error TX!
	{
		cout << "wait_rx Send Failed" << endl;
		return -1;
	}
	return res;
}



// test the MCL with an application that waits till all
// DUs are sent before closing.
int test_wait_tx()
{
	return _spawnl(_P_WAIT, "test_wait_tx", "test_no_tx", NULL);
}
