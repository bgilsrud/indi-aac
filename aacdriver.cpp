/*******************************************************************************
  Copyright(c) 2012 Jasem Mutlaq. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/
#include "aacdriver.h"
//#include "gpdriver.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>


AACDriver::AACDriver()
{
}

AACDriver::~AACDriver()
{
}


bool AACDriver::Connect()
{
   int rc;
   struct termios tio;
   serial_fd = open("/dev/aac_guider", O_RDWR | O_NOCTTY | O_NDELAY);
   if (serial_fd == -1) {
        IDLog("Error connecting to /dev/aac_guider: %s\n", strerror(errno));
        return -1;
   } 
   tcgetattr(serial_fd, &tio);
   cfsetospeed(&tio, B9600);
   tcsetattr(serial_fd, TCSANOW, &tio);

   rc = fcntl(serial_fd, F_SETFL, 0);
   return (rc != -1);
    
}

bool AACDriver::Disconnect()
{
    if (serial_fd != -1) {
        close(serial_fd);
        serial_fd = -1;
    }
    return true;
}

#define START_GUIDE '1'
#define STOP_GUIDE '0'
bool AACDriver::startPulse(int direction)
{
    int rc=0;

    guideCMD[1] = START_GUIDE;
    switch (direction)
    {
        case AAC_NORTH:
        guideCMD[0] = AAC_NORTH;
        break;

        case AAC_WEST:
        guideCMD[0] = AAC_WEST;
        break;

        case AAC_SOUTH:
        guideCMD[0] = AAC_SOUTH;
        break;

        case AAC_EAST:
        guideCMD[0] = AAC_EAST;
        break;
    }

    if (debug)
        IDLog("start command value is 0x%c%c\n", guideCMD[0], guideCMD[1]);

    rc = write(serial_fd, guideCMD, 2);

    if(rc==2)
        return true;

    return false;
}

bool AACDriver::stopPulse(int direction)
{
   int rc=0;

    guideCMD[1] = STOP_GUIDE;
    switch (direction)
    {
        case AAC_NORTH:
        if (debug) IDLog("Stop North\n");
        guideCMD[0] = AAC_NORTH;
        break;

        case AAC_WEST:
        if (debug) IDLog("Stop West\n");
        guideCMD[0] = AAC_WEST;
        break;

        case AAC_SOUTH:
        if (debug) IDLog("Stop South\n");
        guideCMD[0] = AAC_SOUTH;
        break;

        case AAC_EAST:
        if (debug) IDLog("Stop East\n");
        guideCMD[0] = AAC_EAST;
        break;
    }

    rc = write(serial_fd, guideCMD, 2);

    if(rc==2)
        return true;

    return false;
}

