
#include "stackArray.h"
#include "stackMap.h"
#include "simLib.h"

#include "RTDEclient.h"

#include "simExtRTDE.h"

#include <iostream>

#ifdef _WIN32
    #ifdef QT_COMPIL
        #include <direct.h>
    #else
        #include <shlwapi.h>
        #pragma comment(lib, "Shlwapi.lib")
    #endif
#endif

#if defined(__linux) || defined(__APPLE__)
    #include <unistd.h>
    #include <string.h>
    #define _stricmp(x,y) strcasecmp(x,y)
#endif

#define CONCAT(x,y,z) x y z
#define strConCat(x,y,z)    CONCAT(x,y,z)

#define PLUGIN_VERSION 5 // 2 since version 3.2.1, 3 since V3.3.1, 4 since V3.4.0, 5 since V3.4.1

static LIBRARY simLib; // the CoppelisSim library that we will dynamically load and bind

using namespace std;

static unique_ptr<RTDEclient> ptr_rtde_client;
static unique_ptr<RobotInterface> ptr_robot;

static bool plugin_loaded = false;
static bool connected = false;

static int cycle_count=0;

bool simExtGetRobotHandles(Robot &robot)
{
    //Get robot handles
    int rhandle = simGetObjectHandle(robot.name.c_str());
    if(rhandle != -1)
    {
        robot.handle = rhandle;
        //Get robot joint handles
        int bit_options, num_objects;
        bit_options = 1; //bit0 set (1): exclude the tree base from the returned array
        int *ret = simGetObjectsInTree(rhandle,sim_object_joint_type,bit_options,&num_objects);
        if(ret != nullptr)
        {
            if(num_objects == robot.dofs)
            {
                robot.jhandles.assign(ret, ret+num_objects);
            }
            else
            {
                simAddStatusbarMessage("Number of joint objects in the robot does not correspond to the model. ");
                return false;
            }
        }
        else {
            simAddStatusbarMessage("Could not retrieve the robot joint handles.");
            return false;
        }
    }
    else {
        simAddStatusbarMessage("Could not retrieve the robot handle.");
        return false;
    }
    return true;
}

bool simExtGetJointLimits(Robot &robot)
{
    simBool cyclic;
    float limits[2]; //[0] minimum, [1] range
    robot.jplim.resize(robot.dofs);
    robot.jvlim.resize(robot.dofs);
    for(int i=0; i<robot.dofs; ++i)
    {
        //Get position limits
        if(simGetJointInterval(robot.jhandles[i],&cyclic,limits) != -1)
        {
            robot.jplim[i] = std::pair<double, double>(limits[0], limits[0] + limits[1]);
        }
        else
        {
            simAddStatusbarMessage("Could not retrieve robot's joints position limits.");
            return false;
        }
        //Get velocity limits
        float parameter;
        if(simGetObjectFloatParameter(robot.jhandles[i],sim_jointfloatparam_upper_limit,&parameter) == 1)
        {
            robot.jvlim[i] = parameter;
        }
        else
        {
            simAddStatusbarMessage("Could not retrieve robot's joints velocity limit.");
            return false;
        }
    }
    return true;
}

bool simExtGetJointPositions(const std::vector<int> &jhandles, std::vector<double> &pos)
{
    pos.resize(jhandles.size());
    for(size_t i=0; i<jhandles.size(); ++i)
    {
        simFloat jpos;
        if(simGetJointPosition(jhandles[i], &jpos) != -1)
        {
            pos[i] = static_cast<double>(jpos);
        }
        else
        {
            simAddStatusbarMessage("Could not retrieve current joint positions.");
            return false;
        }
    }
    return true;
}

bool simExtGetJointVelocities(const std::vector<int> &jhandles, std::vector<double> &vel)
{
    vel.resize(jhandles.size());
    for(size_t i=0; i<jhandles.size(); ++i)
    {
        simFloat jvel;
        if(simGetObjectFloatParameter(jhandles[i],sim_jointfloatparam_velocity,&jvel) != -1)
        {
            vel[i] = static_cast<double>(jvel);
        }
        else
        {
            simAddStatusbarMessage("Could not retrieve current joint velocities.");
            return false;
        }
    }
    return true;
}

bool simExtSetJointPositions(const std::vector<int> &jhandles, const std::vector<double> &dpos)
{
    if(jhandles.size() != dpos.size())
    {
        simAddStatusbarMessage("Joint handles specified do not match target joint positions.");
        return false;
    }

    for(size_t i=0; i<jhandles.size(); ++i)
    {
        if(simSetJointPosition(jhandles[i], dpos[i]) == -1)
        {
            simAddStatusbarMessage("Could not set desired joint position for a passive joint.");
            return false;
        }
    }
    return true;
}

// --------------------------------------------------------------------------------------
// simRTDE.init: Launch this plugin
// --------------------------------------------------------------------------------------
#define LUA_INIT_COMMAND "simRTDE.init" // the name of the new Lua command

void LUA_INIT_CALLBACK(SScriptCallBack* p)
{
    // the callback function of the new Lua command ("simRTDE.init")
    int stack=p->stackID;

    CStackArray inArguments;
    inArguments.buildFromStack(stack);

    if ( inArguments.getSize() == 0 )
    {
        if (plugin_loaded) {
            ptr_rtde_client.reset();

            plugin_loaded = false;
        }
        plugin_loaded = true;

        // Create RobotInterface
        ptr_robot = make_unique<RobotInterface>();
        ptr_robot->InitRobot();

        // Create RTDEclient
        ptr_rtde_client = make_unique<RTDEclient>();
        simAddStatusbarMessage("RTDE Loaded");

        // Get Robot and Joint Handles
        simExtGetRobotHandles(ptr_robot->ur10);
        simExtGetJointLimits(ptr_robot->ur10);

        // TODO: Guarantee joints are passive
    }
}

// --------------------------------------------------------------------------------------
// simRTDE.connect: Launch this plugin
// --------------------------------------------------------------------------------------
#define LUA_CONNECT_COMMAND "simRTDE.connect" // the name of the new Lua command

void LUA_CONNECT_CALLBACK(SScriptCallBack* p)
{
    // the callback function of the new Lua command ("simRTDE.connect")
    int stack=p->stackID;

    CStackArray inArguments;
    inArguments.buildFromStack(stack);

    if ( inArguments.getSize() == 0 )
    {
        if (plugin_loaded) {
            connected = true;

            simAddStatusbarMessage("RTDE Connected");
        }
        else {
            simAddStatusbarMessage("RTDE plugin not loaded!");
        }
    }
}

// --------------------------------------------------------------------------------------
// simRTDE.disconnect: Launch this plugin
// --------------------------------------------------------------------------------------
#define LUA_DISCONNECT_COMMAND "simRTDE.disconnect" // the name of the new Lua command

void LUA_DISCONNECT_CALLBACK(SScriptCallBack* p)
{
    // the callback function of the new Lua command ("simRTDE.disconnect")
    int stack=p->stackID;

    CStackArray inArguments;
    inArguments.buildFromStack(stack);

    if ( inArguments.getSize() == 0 )
    {
        if (plugin_loaded) {
            connected = false;

            simAddStatusbarMessage("RTDE Disconnected");
        }
        else {
            simAddStatusbarMessage("RTDE plugin not loaded!");
        }
    }
}

// This is the plugin start routine (called just once, just after the plugin was loaded):
SIM_DLLEXPORT unsigned char simStart(void* reservedPointer,int reservedInt)
{
    // Dynamically load and bind CoppelisSim functions:
    // 1. Figure out this plugin's directory:
    char curDirAndFile[1024];
#ifdef _WIN32
    #ifdef QT_COMPIL
        _getcwd(curDirAndFile, sizeof(curDirAndFile));
    #else
        GetModuleFileName(NULL,curDirAndFile,1023);
        PathRemoveFileSpec(curDirAndFile);
    #endif
#else
    getcwd(curDirAndFile, sizeof(curDirAndFile));
#endif

    std::string currentDirAndPath(curDirAndFile);
    // 2. Append the CoppelisSim library's name:
    std::string temp(currentDirAndPath);
#ifdef _WIN32
    temp+="\\coppeliaSim.dll";
#elif defined (__linux)
    temp+="/libcoppeliaSim.so";
#elif defined (__APPLE__)
    temp+="/libcoppeliaSim.dylib";
#endif /* __linux || __APPLE__ */
    // 3. Load the CoppelisSim library:
    simLib=loadSimLibrary(temp.c_str());
    if (simLib==NULL)
    {
        printf("simExtRTDE: error: could not find or correctly load the CoppeliaSim library. Cannot start the plugin.\n"); // cannot use simAddLog here.
        return(0); // Means error, CoppelisSim will unload this plugin
    }
    if (getSimProcAddresses(simLib)==0)
    {
        printf("simExtRTDE: error: could not find all required functions in the CoppeliaSim library. Cannot start the plugin.\n"); // cannot use simAddLog here.
        unloadSimLibrary(simLib);
        return(0); // Means error, CoppelisSim will unload this plugin
    }

    // Check the version of CoppelisSim:
    int simVer,simRev;
    simGetIntegerParameter(sim_intparam_program_version,&simVer);
    simGetIntegerParameter(sim_intparam_program_revision,&simRev);
    if( (simVer<40000) || ((simVer==40000)&&(simRev<1)) )
    {
        simAddStatusbarMessage("sorry, your CoppelisSim copy is somewhat old, CoppelisSim 4.0.0 rev1 or higher is required. Cannot start the plugin.");
        //simAddLog("RTDE",sim_verbosity_errors,"sorry, your CoppelisSim copy is somewhat old, CoppelisSim 4.0.0 rev1 or higher is required. Cannot start the plugin.");
        unloadSimLibrary(simLib);
        return(0); // Means error, CoppelisSim will unload this plugin
    }

    // Implicitely include the script lua/simExtRTDE.lua:
    simRegisterScriptVariable("simRTDE","require('simExtRTDE')",0);

    // Register the new function:
    simRegisterScriptCallbackFunction(strConCat(LUA_INIT_COMMAND,"@","RTDE"),strConCat("...=",LUA_INIT_COMMAND,"()"),LUA_INIT_CALLBACK);
    simRegisterScriptCallbackFunction(strConCat(LUA_CONNECT_COMMAND,"@","RTDE"),strConCat("...=",LUA_CONNECT_COMMAND,"()"),LUA_CONNECT_CALLBACK);
    simRegisterScriptCallbackFunction(strConCat(LUA_DISCONNECT_COMMAND,"@","RTDE"),strConCat("...=",LUA_DISCONNECT_COMMAND,"()"),LUA_DISCONNECT_CALLBACK);


    return(PLUGIN_VERSION); // initialization went fine, we return the version number of this plugin (can be queried with simGetModuleName)
}

// This is the plugin end routine (called just once, when CoppelisSim is ending, i.e. releasing this plugin):
SIM_DLLEXPORT void simEnd()
{
    // Here you could handle various clean-up tasks

    unloadSimLibrary(simLib); // release the library
}

// This is the plugin messaging routine (i.e. CoppelisSim calls this function very often, with various messages):
SIM_DLLEXPORT void* simMessage(int message,int* auxiliaryData,void* customData,int* replyData)
{ // This is called quite often. Just watch out for messages/events you want to handle
    // Keep following 5 lines at the beginning and unchanged:
    static bool refreshDlgFlag=true;
    int errorModeSaved;
    simGetIntegerParameter(sim_intparam_error_report_mode,&errorModeSaved);
    simSetIntegerParameter(sim_intparam_error_report_mode,sim_api_errormessage_ignore);
    void* retVal=NULL;

    // Here we can intercept many messages from CoppelisSim (actually callbacks). Only the most important messages are listed here.
    // For a complete list of messages that you can intercept/react with, search for "sim_message_eventcallback"-type constants
    // in the CoppelisSim user manual.

    if (message==sim_message_eventcallback_refreshdialogs)
        refreshDlgFlag=true; // CoppelisSim dialogs were refreshed. Maybe a good idea to refresh this plugin's dialog too

    if (message==sim_message_eventcallback_menuitemselected)
    { // A custom menu bar entry was selected..
        // here you could make a plugin's main dialog visible/invisible
    }

    if (message==sim_message_eventcallback_instancepass)
    {   // This message is sent each time the scene was rendered (well, shortly after) (very often)
        // It is important to always correctly react to events in CoppelisSim. This message is the most convenient way to do so:

        int flags=auxiliaryData[0];
        bool sceneContentChanged=((flags&(1+2+4+8+16+32+64+256))!=0); // object erased, created, model or scene loaded, und/redo called, instance switched, or object scaled since last sim_message_eventcallback_instancepass message 
        bool instanceSwitched=((flags&64)!=0);

        if (instanceSwitched)
        {
            // React to an instance switch here!!
        }

        if (sceneContentChanged)
        { // we actualize plugin objects for changes in the scene

            //...

            refreshDlgFlag=true; // always a good idea to trigger a refresh of this plugin's dialog here
        }
    }

    if (message==sim_message_eventcallback_mainscriptabouttobecalled)
    { // The main script is about to be run (only called while a simulation is running (and not paused!))
        
    }

    if (message==sim_message_eventcallback_simulationabouttostart)
    { // Simulation is about to start

    }

    if (message==sim_message_eventcallback_simulationended)
    { // Simulation just ended

    }

    if (message==sim_message_eventcallback_moduleopen)
    { // A script called simOpenModule (by default the main script). Is only called during simulation.
        if ( (customData==NULL)||(_stricmp("RTDE",(char*)customData)==0) ) // is the command also meant for this plugin?
        {
            // we arrive here only at the beginning of a simulation
        }
    }

    if (message==sim_message_eventcallback_modulehandle)
    { // A script called simHandleModule (by default the main script). Is only called during simulation.
        if ( (customData==NULL)||(_stricmp("RTDE",(char*)customData)==0) ) // is the command also meant for this plugin?
        {
            // HERE
            if(plugin_loaded) {
                if(connected) {
                    std::vector<double> cjpos;
                    if(ptr_rtde_client->getJoints(cjpos))
                    {
                        for(int j=0; j<ptr_robot->ur10.dofs; ++j) {
                            simSetJointPosition(ptr_robot->ur10.jhandles[j], cjpos[j]);
                        }
                    }
                }
            }
        }
    }

    if (message==sim_message_eventcallback_moduleclose)
    { // A script called simCloseModule (by default the main script). Is only called during simulation.
        if ( (customData==NULL)||(_stricmp("RTDE",(char*)customData)==0) ) // is the command also meant for this plugin?
        {
            // we arrive here only at the end of a simulation
        }
    }

    if (message==sim_message_eventcallback_instanceswitch)
    { // We switched to a different scene. Such a switch can only happen while simulation is not running

    }

    if (message==sim_message_eventcallback_broadcast)
    { // Here we have a plugin that is broadcasting data (the broadcaster will also receive this data!)

    }

    if (message==sim_message_eventcallback_scenesave)
    { // The scene is about to be saved. If required do some processing here (e.g. add custom scene data to be serialized with the scene)

    }

    // You can add many more messages to handle here

    if ((message==sim_message_eventcallback_guipass)&&refreshDlgFlag)
    { // handle refresh of the plugin's dialogs
        // ...
        refreshDlgFlag=false;
    }

    // Keep following unchanged:
    simSetIntegerParameter(sim_intparam_error_report_mode,errorModeSaved); // restore previous settings
    return(retVal);
}
