# SeL4 HDMI

This repo contains the code for an SeL4 HDMI driver that supports the creation and the display of static and moving images. Double buffering has been implemented for moving images which means that whilst the current frame is being displayed, the next frame is being written to. In it's current state, there is a visual glitch that is caused by the screen being redrawn each time the new buffer is displayed. This is more noticeable when the entire screen has changed (see example rotating_bars). 

# Building firmware (uboot)

TODO: potentially make another fork of uboot for firmware, if not explain what needs to be removed.

# Building picolibc

This project requires pibolibc. To build picolibc from this repo:

```
cd picolibc
./build
```

# Building the project

Talk about the build script variables

This API contains the following examples:

* static_image - Displays 4 colour bars on the screen.
* resolution_change - Displays a square of the same size in pixels one after another at three different resolutions.
* rotating_bars - Displays 4 colour bars rotating across the screen.
* empty_client - An empty example ready to be used to create a static or moving image

To build the project use ./build.sh with an example as the first argument.

__e.g ./build.sh empty_client__

It is recommended to use a prebuilt docker image to run this project. This can be found at: https://github.com/sel4-cap/microkit-maaxboard-dockerfiles

You can use your own custom configuration by providing "c" as the second argument. If this is the case, the custom configuration will need to be implemented in the build script.

__e.g ./build.sh static_image c__

# Running an example


# Using the API

empty client and how it is set up
hdmi_data struct and what that holds (RGBA ordering, alpha)
how static images are made with a timer
how moving images are made
how to end a moving image example
