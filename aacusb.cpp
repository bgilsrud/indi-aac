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
#include "aacusb.h"
#include "aacdriver.h"
#include <memory>

#define POLLMS 250

// We declare an auto pointer to gpGuide.
std::auto_ptr<AACUSB> gpGuide(0);

void ISInit()
{
   static int isInit =0;

   if (isInit == 1)
       return;

    isInit = 1;
    if(gpGuide.get() == 0) gpGuide.reset(new AACUSB());
    //IEAddTimer(POLLMS, ISPoll, NULL);

}

void ISGetProperties(const char *dev)
{
        ISInit();
        gpGuide->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num)
{
        ISInit();
        gpGuide->ISNewSwitch(dev, name, states, names, num);
}

void ISNewText(	const char *dev, const char *name, char *texts[], char *names[], int num)
{
        ISInit();
        gpGuide->ISNewText(dev, name, texts, names, num);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num)
{
        ISInit();
        gpGuide->ISNewNumber(dev, name, values, names, num);
}

void ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
  INDI_UNUSED(dev);
  INDI_UNUSED(name);
  INDI_UNUSED(sizes);
  INDI_UNUSED(blobsizes);
  INDI_UNUSED(blobs);
  INDI_UNUSED(formats);
  INDI_UNUSED(names);
  INDI_UNUSED(n);
}
void ISSnoopDevice (XMLEle *root)
{
    INDI_UNUSED(root);
}

AACUSB::AACUSB()
{
    driver = new AACDriver();
    WEDir = NSDir = 0;
    //ctor
}

AACUSB::~AACUSB()
{
    //dtor

    delete (driver);

}


const char * AACUSB::getDefaultName()
{
    return (char *)"AACUSB";
}

bool AACUSB::Connect()
{
    driver->setDebug(isDebug());

    bool rc = driver->Connect();

    if (rc)
        IDMessage(getDeviceName(), "AACUSB is online.");
    else
        IDMessage(getDeviceName(), "Error: cannot find AACUSB device.");

    return rc;
    
}

bool AACUSB::Disconnect()
{
    IDMessage(getDeviceName(), "GPSUSB is offline.");

    return driver->Disconnect();
}

bool AACUSB::initProperties()
{
    initGuiderProperties(getDeviceName(), MAIN_CONTROL_TAB);

    addDebugControl();

    return INDI::DefaultDevice::initProperties();
}

bool AACUSB::updateProperties()
{

    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        defineNumber(&GuideNSNP);
        defineNumber(&GuideWENP);
    }
    else
    {
        deleteProperty(GuideNSNP.name);
        deleteProperty(GuideWENP.name);
    }

    return true;

}

void AACUSB::ISGetProperties (const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);
}

bool AACUSB::ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        if (!strcmp(name,GuideNSNP.name) || !strcmp(name,GuideWENP.name))
        {
            processGuiderProperties(name, values, names, n);
            return true;
        }
    }

    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool AACUSB::ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n)
{
    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool AACUSB::ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n)
{
    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool AACUSB::ISSnoopDevice (XMLEle *root)
{
    return INDI::DefaultDevice::ISSnoopDevice(root);
}

void AACUSB::debugTriggered(bool enable)
{
    driver->setDebug(enable);
}

float AACUSB::CalcWEPulseTimeLeft()
{
    double timesince;
    double timeleft;
    struct timeval now;
    gettimeofday(&now,NULL);

    timesince=(double)(now.tv_sec * 1000.0 + now.tv_usec/1000) - (double)(WEPulseStart.tv_sec * 1000.0 + WEPulseStart.tv_usec/1000);
    timesince=timesince/1000;


    timeleft=WEPulseRequest-timesince;
    return timeleft;
}

float AACUSB::CalcNSPulseTimeLeft()
{
    double timesince;
    double timeleft;
    struct timeval now;
    gettimeofday(&now,NULL);

    timesince=(double)(now.tv_sec * 1000.0 + now.tv_usec/1000) - (double)(NSPulseStart.tv_sec * 1000.0 + NSPulseStart.tv_usec/1000);
    timesince=timesince/1000;


    timeleft=NSPulseRequest-timesince;
    return timeleft;
}


void AACUSB::TimerHit()
{
    float timeleft;
    int rc;

    if(InWEPulse)
    {
        timeleft=CalcWEPulseTimeLeft();

        if(timeleft < 1.0)
        {
            if(timeleft > 0.25)
            {
                //  a quarter of a second or more
                //  just set a tighter timer
                WEtimerID = SetTimer(250);
            } else
            {
                if(timeleft >0.07)
                {
                    //  use an even tighter timer
                    WEtimerID = SetTimer(50);
                } else
                {
                    //  it's real close now, so spin on it
                    while(timeleft > 0)
                    {
                        int slv;
                        slv=100000*timeleft;
                        //IDLog("usleep %d\n",slv);
                        usleep(slv);
                        timeleft=CalcWEPulseTimeLeft();
                    }


                    driver->stopPulse(WEDir);
                    InWEPulse = false;

                    // If we have another pulse, keep going
                    if (!InNSPulse)
                        SetTimer(250);

                }
            }
        } else if (!InNSPulse)
        {
            WEtimerID = SetTimer(250);
        }
    }

    if(InNSPulse)
    {
        timeleft=CalcNSPulseTimeLeft();

        if(timeleft < 1.0)
        {
            if(timeleft > 0.25)
            {
                //  a quarter of a second or more
                //  just set a tighter timer
                NStimerID =  SetTimer(250);
            } else
            {
                if(timeleft >0.07)
                {
                    //  use an even tighter timer
                    NStimerID = SetTimer(50);
                } else
                {
                    //  it's real close now, so spin on it
                    while(timeleft > 0)
                    {
                        int slv;
                        slv=100000*timeleft;
                        //IDLog("usleep %d\n",slv);
                        usleep(slv);
                        timeleft=CalcNSPulseTimeLeft();
                    }

                    driver->stopPulse(NSDir);
                    InNSPulse = false;
                }
            }
        } else
        {
            NStimerID = SetTimer(250);
        }
    }

}

bool AACUSB::GuideNorth(float ms)
{

    RemoveTimer(NStimerID);


    driver->startPulse(AAC_NORTH);

    NSDir = AAC_NORTH;

    DEBUG(INDI::Logger::DBG_DEBUG, "Starting NORTH guide");

    if (ms <= POLLMS)
    {

        usleep(ms*1000);

        driver->stopPulse(AAC_NORTH);
        return true;
    }

    NSPulseRequest=ms/1000.0;
    gettimeofday(&NSPulseStart,NULL);
    InNSPulse=true;


    NStimerID = SetTimer(ms-50);

    return true;
}

bool AACUSB::GuideSouth(float ms)
{


    RemoveTimer(NStimerID);

    driver->startPulse(AAC_SOUTH);

    DEBUG(INDI::Logger::DBG_DEBUG, "Starting SOUTH guide");

    NSDir = AAC_SOUTH;

    if (ms <= POLLMS)
    {

        usleep(ms*1000);

        driver->stopPulse(AAC_SOUTH);
        return true;
    }

    NSPulseRequest=ms/1000.0;
    gettimeofday(&NSPulseStart,NULL);
    InNSPulse=true;


    NStimerID = SetTimer(ms-50);

    return true;

}

bool AACUSB::GuideEast(float ms)
{


    RemoveTimer(WEtimerID);

    driver->startPulse(AAC_EAST);

    DEBUG(INDI::Logger::DBG_DEBUG, "Starting EAST guide");

    WEDir = AAC_EAST;

    if (ms <= POLLMS)
    {

        usleep(ms*1000);

        driver->stopPulse(AAC_EAST);
        return true;
    }

    WEPulseRequest=ms/1000.0;
    gettimeofday(&WEPulseStart,NULL);
    InWEPulse=true;

    WEtimerID = SetTimer(ms-50);

    return true;
}

bool AACUSB::GuideWest(float ms)
{

    RemoveTimer(WEtimerID);

    driver->startPulse(AAC_WEST);

    DEBUG(INDI::Logger::DBG_DEBUG, "Starting WEST guide");

    WEDir = AAC_WEST;

    if (ms <= POLLMS)
    {

        usleep(ms*1000);

        driver->stopPulse(AAC_WEST);
        return true;
    }

    WEPulseRequest=ms/1000.0;
    gettimeofday(&WEPulseStart,NULL);
    InWEPulse=true;


    WEtimerID = SetTimer(ms-50);

    return true;

}
