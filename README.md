Indoor Localization DB server
==========================

&nbsp;__Author__: Chris Card

# Topics #
 - [Environment](#environment)
 - [Compilation](#compilation)
 - [Program Parts](#program_parts)


---------------
# Environment #
&nbsp;&nbsp;This section describes what the environment setup must be inorder for this application to compile and execute.

## Environmental Variables (OS level) ##
&nbsp;&nbsp;&nbsp;This section describes what environmental variables need to be set up (__Linux not supported__).
 - `OPENCV_RELEASE_DIR`: `<Path to opencv builds dir>\bin\Release`
 - `OPENCV_DEBUG_DIR`: `<Path to opencv builds dir>\bin\Debug`
 - `OPENCV_INCLUDE_DIRS`: `<Path to opencv build dir>\include`
 - `OPENCV_LIB_DIRS`: `<Path to opencv build dir>\x86\vc10;<Path to opencv build dir>\x86\vc10\lib`

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
 - First Create a new Project in the solution named *Localization_CodeRedizine*
 - Then add all the files to the solution
 - Then open the project properties and set the properties appropriatly

### Debug Mode (only for testing should be run Release Mode) ###
 - In the Debuging propertie set the _Environment_ to `PATH=$(OPENCV_DEBUG_DIR);<Path to opencv build dir>\x86\vc10\bin;$(Path)`
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_Debug_Config.png "See ScreenShots folder")
 - In the _C/C++_ configuration page set _Additional Dependencies_ to `$(OPENCV_INCLUDE_DIRS);%(AdditionalIncludeDirectories)`
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_C_CPP_Config.png "See ScreenShots folder")
 - (__Optional__) To turn on OpenMP go to _C/C++_->_Language_ and set _Open MP Support_ to Yes
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_OpenMP_Config.png "See ScreenShots folder")
 - In the _Linker_ configuration page set _Additional Library Directories_ to `$(OPENCV_LIB_DIRS);%(AdditionalLibraryDirectories)`
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_Linker_Config.png "See ScreenShots folder")
 - In the _Linker_->_Input_ set _Addition Dependincies_ to:
```
 	opencv_calib3d248d.lib;opencv_contrib248d.lib;opencv_core248d.lib;opencv_features2d248d.lib;opencv_flann248d.lib;opencv_gpu248d.lib;opencv_highgui248d.lib;
 	opencv_imgproc248d.lib;opencv_legacy248d.lib;opencv_ml248d.lib;opencv_nonfree248d.lib;opencv_objdetect248d.lib;opencv_photo248d.lib;opencv_stitching248d.lib;
 	opencv_ts248d.lib;opencv_video248d.lib;opencv_videostab248d.lib;%(AdditionalDependencies)
```
![alt text](https://github.com/ccard/Indoor_Localization_server/blob/debug_mode/ScreenShots/Debug_Linker_Input_Config.png "See ScreenShots folder")

### Release Mode (Perfered execution mode) ###
&nbsp;&nbsp;&nbsp;&nbsp;This will be identical to the debug mode except for some config changes:
 - In the Debuging propertie set the _Environment_ to `PATH=$(OPENCV_RELEASE_DIR);$(Path)`
 - In the _C/C++_ configuration page set _Additional Dependencies_ to `$(OPENCV_INCLUDE_DIRS);%(AdditionalIncludeDirectories)`
 - (__Optional__) To turn on OpenMP go to _C/C++_->_Language_ and set _Open MP Support_ to Yes
 - In the _Linker_ configuration page set _Additional Library Directories_ to `$(OPENCV_LIB_DIRS);%(AdditionalLibraryDirectories)`
 - In the _Linker_->_Input_ set _Addition Dependincies_ to:
```
 	opencv_calib3d248.lib;opencv_contrib248.lib;opencv_core248.lib;opencv_features2d248.lib;opencv_flann248.lib;opencv_gpu248.lib;
 	opencv_highgui248.lib;opencv_imgproc248.lib;opencv_legacy248.lib;opencv_ml248.lib;opencv_nonfree248.lib;opencv_objdetect248.lib;
 	opencv_photo248.lib;opencv_stitching248.lib;opencv_ts248.lib;opencv_video248.lib;opencv_videostab248.lib;%(AdditionalDependencies)
```

--------
# Compilation #
&nbsp;This section describes how to compile the program.

## Windows ##
&nbsp;&nbsp;To compile in Visual Studio&reg; simply build in the desired mode, _Release_ is the perfered mode.

## Linux ##
&nbsp;&nbsp;__Not Supported__ at this time.

## VS Compilation ##
&nbsp;&nbsp;You should only have to do a build/rebuild on the project and it should all work if you set up the [environment](#environment) correctly.

## Linux ##
&nbsp;&nbsp;__Not Supported__ at this time!

---------
# Program Parts #
&nbsp;This section describes the different classes, interfaces, and precompiler flags.

## Interfaces ##
&nbsp;&nbsp;These interfaces represent the core of the code as most if not all of the class extend one of them.  If you wish to add classes to this code they should be template classes 
and should use the interfaces so that any class that extends the interfaces can be used.  The interfaces also define key parameters and precompiler flags that are used through out the provided
code. These interfaces are also template classes that perform compile time type checking on the defined type (e.x. `static_assert(is_base_of<ImageContainer,ImType>::value, "Must be derived class of ImageContainer");`).
 - `ImageContainer`: This interface defines an image object that containes at the very least a descriptor mat and a list of key points.  All other template interfaces defined type must be a derivate of ImageContainer, compile time errors will be thrown if the type is not an ImageContainer.  For example, `Matcher<ImageContainer> c` is valid where `Matcher<int> c` is not.
 - `ImageProvider<T>`: This interface encapuslates a list of images and access to those images.  It typically loads them in from a database/file but does support adding images after reading from a db/file opening a file or without opening a file.  `T` must be a derived class of `ImageContainer`.
 - `Matcher<ImType>`: This interface abstracts the matching process and provides access to the matching capabilities. `ImType` must be a derived class of `ImageContainer`.

## Classes ##
&nbsp;&nbsp; These classes are what implements the program and test the algorithm.
 - Classes derived from `ImageProvider<T>`:
  - `DBProvider<ImType>`: This class loads in xml files that contain the precomputed descriptors and key points of the images along with the original images location.  The text file that it opens is expected to be in the following format `<xml file location> <original image file location>`.  It how ever does not load in the original image file just the location.
  - `MatProvider<TmgType>`: This class reads in the original images and computes their descriptors.  This class should be used only if the descriptors where not precomputed as it takes significantly longer to run than `DBProvider<ImType>`. The input file must be in the format `<Image directory location>` and the directory must have a `images.txt` file in the format `<Image file name>`.__Note__: it is recomend that the descriptors and key points be precomputed as this reduces the db loading time by at least an order of magnitude (look at [FileStorage](http://docs.opencv.org/modules/core/doc/xml_yaml_persistence.html) for how to save the descriptors and key points to xml files).
 - Classes derived from `Matcher<ImType>`:
  - `LSHMatching<ImType>`: This class trains a LSH matcher and then uses it to localize a query image.  This class implements the actual indoor localization algorithm and is the core of all the provided code.
  - `MapMatching<ImType>`: This class is the same as `LSHMatching<ImType>` except that it provides an additional find method that returns the top 3 matches with > t inliers to the fundamental matrix model along with there rotation and translation matricies.
 - Classes derived from `ImageContainer`:
  - `DBImage`: This class represents a Image that has only a descriptor and list of key points and no information as to where the original image is so it cannot load the image.
  - `MyMat`: This class also extends OpenCV's `Mat` object so that a image can be loaded into the object along with the descriptor and key points.
 - Object classes
  - `MyDMatch`: This class extends OpenCV's `DMatch` object but stores more information than the `DMatch` object does so the `ImageProvider<T>` does not have to be accessed as often to get the same information.
 - Controller classes. These class interact with the `ImageProvider<T>`,`Matcher<ImType>`, and `ImageContainer` classes to execute the code:
  - `LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>`: ImgType must be a drived class of `ImageContainer`, ImgProviderType must be derived class of `ImageProvider<T>`, and ImgMatcherType must be a derived class of `Matcher<ImgType>`.  This class is used to load the database of images and then match query images to the database so as to abstract the management of the differnt components and only one class needs to be used to run the provided code.  This is where the interfaces are most useful because any class that derives the interfaces can be used in conjunction with this class.
  - `MapBuilder< ImgType, ImgProviderType, ImgMatcherType>`: ImgType must be a drived class of `ImageContainer`, ImgProviderType must be derived class of `ImageProvider<T>`, and ImgMatcherType must be a derived class of `Matcher<ImgType>`. This class is simalar to `LocalizationManager<ImgType, ImgProviderType, ImgMatcherType>` except that it attempts to find the relation ships between all the images in the database so a topological map can be built.  It also uses `MapMatching<ImType>` instead of `LSHMatching<ImType>`.
 - `Localization_CodeRedizine`: contains the main method and is used for testing.

## Precompiler Flages ##
&nbsp;&nbsp;The preocompiler flags turn on and off different sections of code so that various levels of information about execution can be obtained.  The flags `DEBUG` and `INSPECT` are defined in `ImageContainer` for standard exectution both should be set to 0.  If you want to run tests that are already written set `DEBUG` to 1.  If you want to inspect the images as the program executes set `DEBUG` and `INSPECT` to 1.  To turn on the `_OPENMP` flag simply enamble the openmp setting in the properties as described in the [environment](#environment) section.

--------------------------------------------------------------------
# How to Run #
&nbsp;This section describes how to run the provided code and the file formats that the provided code expects. If you wish to use your own classes that extend the interfaces the comments and sections below can be ignored but the physical lines of code in `Localization_CodeRedezine.cpp` shouldn't need to be derastically changed to accept your new class (so long as they extend the interfaces).

## DB setup ##
&nbsp;&nbsp;This section describes how to set up the image database and the corresponding precomputed descriptor database.

### Image DB ###
&nbsp;&nbsp;&nbsp;The first thing that needs to be done is to set up the text files and the directories that will be used to access the orignal images (_i.e._ the "*.jpg" files). First create a `Images\` directory in the provided `Localization_CodeRedizine\` directory. Then create `<Image dir 1>`,....,`<Image dir n>` in the newly created `Images\` directory, these image directories will contain your original images. Each `<Image dir i>` will have an `images.txt` file describing the image files in the directory and is expected to be in the following format:
```
<Image file 1>
<Image file 2>
      .
      .
      .
<Image file n>
```
Then in the `Localization_CodeRedizine\` directory there will be a `<file name>.txt` file that gives list all of the image directories that you created and will have the following format:
```
Images\\<Image dir 1>\\
        .
        .
        .
Images\\<Image dir n>\\
```
This is what is expected of the original image db (__Note:__ all '*.txt' files just described need to be ASCII encoded).

### Precomputed descriptors DB (Recomended) ###
&nbsp;&nbsp;&nbsp;This creates the database of precomputed descriptors for the image db described in the previos section. It is ___recomended___ that this be done because it reduces the db loading time by at least an order of magnitude maybe more. To precompute the descriptors set `OPTION = CREATEDB` in `Localization_CodeRedezine.cpp` and follow the comments in the appropriate if statement to load the image database.  __Note:__ after the new files are created and saved, open `<list output file>` and remove the blank line at the end of the file if this is not done the next steps will result in errors.

## Build the Basic Map ##
&nbsp;&nbsp;&nbsp;This creates the basic map between images and their R and T matricies. Using the precomputed descriptor DB (___recomended___) set `OPTION = CREATEMAP` in `Localization_CodeRedezine.cpp` and follow the comments in the appropriate if statement to create the map. Run `connected_graph.rb` to find the largest connected component in the graph.

## Match Against the database ##
&nbsp;&nbsp;&nbsp;This creates the database of precomputed descriptors for the image db described in the previos section. Using the precomputed descriptor DB (___recomended___) set `OPTION = MATCH` in `Localization_CodeRedezine.cpp` and follow the comments in the appropriate if statement to match to the database. If it finds a match it will display it.