# SeL4 HDMI

This repo contains the code for an SeL4 HDMI driver that supports the creation and the display of static and moving images. Double buffering has been implemented for moving images which means that whilst the current frame is being displayed, the next frame is being written to. In it's current state there is a visual glitch that is caused by the screen being redrawn each time the new buffer is displayed. This is more noticeable when the entire screen has changed (see example rotating_bars). 

# Building firmware (uboot)

TODO: potentially make another fork of uboot for firmware, if not explain what needs to be removed.

# Building picolibc

This project requires pibolibc which can be built from this repo

```
cd picolibc
./build-picolibc.sh
```
Alternatively picolibc can be built from https://github.com/sel4-cap/picolibc.git

Then from within the repo copy the required files into the picolibc directory.

```
sudo cp picolib-microkit/newlib/libc.a path-to-sel4-hdmi/picolibc
sudo cp picolib-microkit/picolibc.specs path-to-sel4-hdmi/picolibc
```

# Building the project

This API contains the following examples

* static_image - Displays 4 colour bars on the screen.
* resolution_change - Displays a square of the same size in pixels one after another at three different resolutions.
* rotating_bars - Displays 4 colour bars rotating across the screen.
* moving_square - A small sqaure that moves around the screen, changing direction each time it hits the side of the screen.

To build the project use ./build.sh with an example as the first argument.

```./build.sh static_image ```

It is recommended to use a prebuilt docker image to build this project. This can be found at: https://github.com/sel4-cap/microkit-maaxboard-dockerfiles

You can use your own custom configuration by providing "c" as the second argument. For example, the custom configuration in the build script currently is set up to be run in an environment set up by microkit manifest: https://github.com/sel4-cap/microkit-manifest.

```./build.sh static_image c```

This will create the loader.img file that will need to be loaded into the maaxboard. The name of this can be changed in the top level Makefile.

For more information setting up an environment for creating Sel4 applications see https://github.com/sel4devkit.

# Using the API

Microkit is used to create the SeL4 image for this project. For more information on SeL4 and microkit see https://github.com/seL4/microkit/blob/main/docs/manual.md

This API makes use of two Protection Domains (PD's). 

* **dcss** - This PD is responsible for the set up of the driver and the configuration of the Display Controller Subsystem (DCSS) and the HDMI TX controller.
* **client** - This PD is responsible for the api and writing to the frame buffer. 

Each PD in microkit must implement ```init()``` and ```notified()``` functions. In this project, the example implements the ```init()``` function, which means that only one example can be built at a time.

The init function is responsible for making the call to initialise the api and selecting if the current image will display a static or moving image. See ```static_image()``` and ```moving_image()``` defined in src/api/api.c.

Each of these functions take in a function pointer as an argument. This function pointer points to a function with no arguments and returns a display config struct. In the examples this function is implemented as init_example

```
struct display_config init_example() {

}

```









In the client init() function, the api must be initialised:

``` init_apit() ```

This is followed by a call to static_image() or moving_image(), which takes in a function pointer to a function with no arguments and a returned display_config struct.   





hdmi_data struct and what that holds (RGBA ordering, alpha, delay for static and for moving)
how static images are made
how moving images are made
how to end a moving image example
to add extra files specific to the example, include them nin the makefile 


In the empty_client directory an empty example has been set up, ready to be implemented. (explain how to use the empty client, or maybe self explanatory at this stage)
