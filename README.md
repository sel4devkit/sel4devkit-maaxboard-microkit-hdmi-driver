# Building firmware (uboot)

# Building picolibc

# Building the project

This API contains the following examples:

static_image - Displays 4 colour bars on the screen.
resolution_change - Displays a square of the same size in pixels one after another at three different resolutions.
rotating_bars - Displays 4 colour bars rotating across the screen.
empty_client - An empty example ready to be used to create a static or moving image

To build the project use ./build.sh with an example as the first argument.

e.g ./build.sh empty_client

It is recommended to use a prebuilt docker image to run this project. This can be found at: https://github.com/sel4-cap/microkit-maaxboard-dockerfiles

You can use your own custom configuration by providing "c" as the second argument. If this is the case, the custom configuration will need to be implemented in the build script.

e.g ./build.sh static_image c
