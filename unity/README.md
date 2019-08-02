# cb-unity
Unity SDK, compiles into CambrianUnity.framework

This is a branch for the purpose of building the CambrianUnity.framework, unity running the CambrianAR unity plugin 
within a widget and building the static CambrianUnity.framework. Since Unity does not normally build anything
but entire apps, interfacing itself at the application delegate, and forcing you to use Unity UI, this modification
of the Unity project structure via scripts and modified unity code was created. The build process is therefore a little bit
unusual. Essentially the generated C++ files and the Data must be moved over. It will not run as a standalone application,
so **you must ignore the Build and Run option**

## Open Unity
Open Unity and make sure both the CambrianARPlugin/Scenes/SceneLoader and CambrianAR/Scenes/CambrianScene 
are showing fully loaded in the application.

## Building and running
First build the Unity application.

Go to File->Build Settings
Verify the Scenes to build being checked and if building a VR scene such as Scenes/ShawVR, make sure that is also checked.

Click Build
Select the location cb-base/cb-unity/src/cb-unity/Build to build into, name it "ios", and next choose either option.

Now build the ios framework from the cb-ios repo by running apps such as ShawARVR or HarmonyApp which use this framework 
or directly build the CambrianUnity framework by opening and running cb-base/cb-unity/src/cb-unity/CambrianUnity/Unity/CambrianUnity.xcodeproj
