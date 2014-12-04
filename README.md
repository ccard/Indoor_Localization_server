Indoor Localization DB server
==========================

&nbsp;__Author__: Chris Card

# Topics #
 - [Environment](#environment)


---------------
# Environement #
&nbsp;&nbsp;This section describes what the environment setup must be inorder for this application to compile and execute.

## Programs ##
&nbsp;&nbsp;&nbsp;This section describes what programs are needed in order to run:
 - Microsoft Visual Studio&reg; 2010 or newer: version _Profesional_ or better (contains libraries that are helpful)
 - [OpenCV 2.4.8](http://sourceforge.net/projects/opencvlibrary/files/) (_perfered_) or [OpenCV 2.4.(9|10)](http://opencv.org/downloads.html)
 - Linux compilation not yet supported but can be in the future

### OpenCV Compilation ##
&nbsp;&nbsp;&nbsp;&nbsp;In order for this application to work _OpenCV_ needs to be compiled for your version of Visual Studio&reg; with certain flags in _CMake_ 
turned on:
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/CMakeFlags1.png "See ScreenShots folder")
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/CMakeFlags2.png "See ScreenShots folder")
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/CMakeFlags3.png "See ScreenShots folder")

## VS Project Setup ##
&nbsp;&nbsp;&nbsp;This section describes how to setup the project in Visual Studio&reg; so that it will compile.
 - First Create a new Project in the solution
 - Then add all the files to the solution
 - Then open the project properties and set the properties appropriatly

### Debug Mode (only for testing should be run Release Mode) ###
 - In the Debuging propertie set the _Environment_ to `PATH=<Path to opencv builds dir>\bin\Debug;<Path to opencv build dir>\x86\vc10\bin;C:\OpenCV\opencv\build\x86\vc
10\bin;$(Path)`
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_Debug_Config.png "See ScreenShots folder")
 - In the _C/C++_ configuration page set Additional Dependencies to `<Path to opencv build dir>\include;%(AdditionalIncludeDirectories)`
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_C_CPP_Config.png "See ScreenShots folder")
 - (Optional) To turn on OpenMP go to _C/C++_->_Language_ and set Open MP Support to Yes
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_OpenMP_Config.png "See ScreenShots folder")
 - In the _Linker_ configuration page set _Additional Library Directories_ to `<Path to opencv build dir>\x86\vc10;<Path to opencv build dir>\x86\vc10\lib;%(AdditionalLibraryDirectories)`
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_Linker_Config.png "See ScreenShots folder")
 - In the _Linker_->_Input_ set _Addition Dependincies_ to:
```
 	opencv_calib3d248d.lib;opencv_contrib248d.lib;opencv_core248d.lib;opencv_features2d248d.lib;opencv_flann248d.lib;opencv_gpu248d.lib;opencv_highgui248d.lib;opencv_imgproc248d.lib;opencv_legacy248d.lib;opencv_ml248d.lib;opencv_nonfree248d.lib;opencv_objdetect248d.lib;opencv_photo248d.lib;opencv_stitching248d.lib;opencv_ts248d.lib;opencv_video248d.lib;opencv_videostab248d.lib;%(AdditionalDependencies)
```
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_Linker_Input_Config.png "See ScreenShots folder")