/*******************************************
 *
 * Name.......:  cameraIrControl Library
 * Description:  A powerful Library to control easy various cameras via IR. Please check the project page and leave a comment.
 * Author.....:  Sebastian Setz
 * Modified...:  Sergey Firsov - 2016-08-05 Adding a factory to create camera objects
 * Version....:  1.9
 * Date.......:  2013-02-12
 * Project....:  http://sebastian.setz.name/arduino/my-libraries/multiCameraIrControl
 * Contact....:  http://Sebastian.Setz.name
 * License....:  This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
 *               To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or send a letter to
 *               Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
 * Keywords...:  arduino, library, camera, ir, control, canon, nikon, olympus, minolta, sony, pentax, interval, timelapse
 * History....:  2010-12-08 V1.0 - release
 *               2010-12-09 V1.1 
 *               2010-12-16 V1.2
 *               2011-01-01 V1.3
 *               2011-01-04 V1.4 - making Sony function work, thank you Walter.
 *               2011-01-25 V1.5 - making Olympus work, thank you Steve Stav.
 *               2011-12-05 V1.6 - adding Olympus zoom, thank you again Steve! Refresh keywords.txt; Arduino 1.0 compatible
 *               2011-12-07 V1.7 - repairing Canon function - thanks to Viktor
 *               2013-02-11 V1.8 - adding toggleFocus for Pentax - thanks to Marcel Normann
 * 								   adding toggleVideo for Sony - thanks to InfinitR
 *               	    V1.9 - adding CanonWLDC100 support - thanks to ImaRH
 *
 ********************************************/

#ifndef multiCameraIrControl_h
#define multiCameraIrControl_h

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// Helper to use different cameras in one application

enum CameraType {
    SONY    = 0,
    NIKON   = 1,
    CANON   = 2,
    PENTAX  = 3,
    OLYMPUS = 4,
    MINOLTA = 5
};

class Camera {
public:
    virtual void shutterNow() = 0;
};

Camera* createCamera(CameraType cameraType, int pin);

//----------------------------------------------------

class Nikon : public Camera{
public:
  Nikon(int pin);
  void shutterNow();
private:
  int _pin;
  int _freq;
};

class Canon : public Camera{
public:
  Canon(int pin);
  void shutterNow();
  void shutterDelayed();
private:
  int _pin;
  int _freq;
};

class CanonWLDC100 : public Camera{
public:
  CanonWLDC100(int pin);
  void shutterNow();
private:
  int _pin;
  int _freq;
};
 
class Pentax : public Camera{
public:
  Pentax(int pin);
  void shutterNow();
  void toggleFocus();
private:
  int _pin;
  int _freq;
};

class Olympus : public Camera{
public:
  Olympus(int pin);
  void shutterNow();
  void zoomin(unsigned int pct);
  void zoomout(unsigned int pct);
private:
  int _pin;
  int _freq;
};

class Minolta : public Camera{
public:
  Minolta(int pin);
  void shutterNow();
  void shutterDelayed();
private:
  int _pin;
  int _freq;
};

class Sony : public Camera{
public:
  Sony(int pin);
  void shutterNow();
  void shutterDelayed();
  void toggleVideo();
private:
  int _pin;
  int _freq;
};

#endif
