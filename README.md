# SeL4 HDMI

This repo contains the code for an SeL4 HDMI driver that supports the creation and the display of static and moving images. Double buffering has been implemented for moving images which means that whilst the current frame is being displayed, the next frame is being written to. In it's current state, there is a visual glitch that is caused by the screen being redrawn each time the new buffer is displayed. This is more noticeable when the entire screen has changed (see example rotating_bars). 

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

It is recommended to use a prebuilt docker image to run this project. This can be found at: https://github.com/sel4-cap/microkit-maaxboard-dockerfiles

You can use your own custom configuration by providing "c" as the second argument. For example, the custom configuration in the build script currently is set up to be run in an environment set up by microkit manifest: https://github.com/sel4-cap/microkit-manifest.

```./build.sh static_image c```

This will create the loader.img file that will need to be loaded into the maaxboard. The name of this can be changed in the top level Makefile.

For more information setting up an environment for creating Sel4 applications, see https://github.com/sel4devkit.

# Running an example


# Using the API

empty client and how it is set up
hdmi_data struct and what that holds (RGBA ordering, alpha)
how static images are made with a timer
how moving images are made
how to end a moving image example
