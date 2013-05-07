
/******************************************************************************

  Filename:      socketapp.c
  Description:   Simple socket programming example for the WiShield 1.0

******************************************************************************

  TCP/IP stack and driver for the WiShield 1.0 wireless devices

  Copyright(c) 2009 Async Labs Inc. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  Contact Information:
  <asynclabs@asynclabs.com>

   Author               Date        Comment
  ---------------------------------------------------------------
   AsyncLabs         06/06/2009   Initial version

*****************************************************************************/

/*
* This is a short example of how to write uIP applications using
* protosockets.
*/

/*
* We define the application state (struct socket_app_state) in the
* socketapp.h file, so we need to include it here. We also include
* uip.h (since this cannot be included in socketapp.h) and
* <string.h>, since we use the memcpy() function in the code.
*/
#include "socketapp.h"
#include "uip.h"
#include <string.h>

extern char recvChar;
extern char sms[8];
extern char outBuffer[40];



/*
* Declaration of the protosocket function that handles the connection
* (defined at the end of the code).
*/
static int handle_connection(struct socket_app_state *s);
/*---------------------------------------------------------------------------*/
/*
* The initialization function. We must explicitly call this function
* from the system initialization code, some time after uip_init() is
* called.
*/
void socket_app_init(void)
{
   /* We start to listen for connections on TCP port 1000. */
   uip_listen(HTONS(1000));

}
/*---------------------------------------------------------------------------*/
/*
* In socketapp.h we have defined the UIP_APPCALL macro to
* socket_app_appcall so that this function is uIP's application
* function. This function is called whenever an uIP event occurs
* (e.g. when a new connection is established, new data arrives, sent
* data is acknowledged, data needs to be retransmitted, etc.).
*/
void socket_app_appcall(void)
{
   /*
    * The uip_conn structure has a field called "appstate" that holds
    * the application state of the connection. We make a pointer to
    * this to access it easier.
    */
   struct socket_app_state *s = &(uip_conn->appstate);

   /*
    * If a new connection was just established, we should initialize
    * the protosocket in our applications' state structure.
    */
   if(uip_connected()) {
      PSOCK_INIT(&s->p, s->inputbuffer, sizeof(s->inputbuffer));
   }

   /*
    * Finally, we run the protosocket function that actually handles
   * the communication. We pass it a pointer to the application state
   * of the current connection.
   */
   handle_connection(s);
}
/*---------------------------------------------------------------------------*/
/*
* This is the protosocket function that handles the communication. A
* protosocket function must always return an int, but must never
* explicitly return - all return statements are hidden in the PSOCK
* macros.
*/
static int handle_connection(struct socket_app_state *s)
{
	
	/*
		
	}*/
	
   if(uip_newdata()) {
      PSOCK_BEGIN(&s->p);
      while(!uip_closed()){
      PSOCK_READTO(&s->p, '\n');	
      recvChar = s->inputbuffer[0];
      sms[0] = s->inputbuffer[0];
      sms[1] = s->inputbuffer[1];
      sms[2] = s->inputbuffer[2];
      sms[3] = s->inputbuffer[3];
      if(sms[2]=='\n')
        sms[2] = '\0';
      else if(sms[3]=='\n')
        sms[3] = '\0';
        else
          sms[4]='\0';
      memset(s->inputbuffer, 0x00, sizeof(s->inputbuffer));
      }
      PSOCK_CLOSE(&s->p);
      PSOCK_END(&s->p);
   }
   else {
      //if no new data is received set our global into to 0 to indicate no new data
      recvChar = NULL;
	 
   }
   if(outBuffer[0] != NULL) {
		PSOCK_BEGIN(&s->p);
		PSOCK_SEND_STR(&s->p, outBuffer);
		PSOCK_END(&s->p);
		outBuffer[0] = NULL;
		memset(outBuffer, 0x00, sizeof(outBuffer));
		}
   
 
}
/*---------------------------------------------------------------------------*/
