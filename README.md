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

### Initialising the client PD

Each PD in microkit must implement ```init()``` and ```notified()``` functions. In this project, the example implements the ```init()``` function, which means that only one example can be built at a time.

The init function is responsible for making the call to initialise the api and to select if the current image will display a static or moving image. See ```static_image()``` and ```moving_image()``` defined in src/api/api.c.

These two functions take in a function pointer as an argument. This function pointer points to a function with no arguments and returns a ```display_config``` struct. In the examples this function is implemented as ```init_example()```. 

```
struct display_config init_example() {
}
```
The ```display_config``` struct is needs to be initialised so that the configuration settings can be sent to the dcss PD and so that the function to write to the frame buffer is defined. 

```
struct display_config {
	struct hdmi_data hd;
	void (*write_fb)();
};
```
### Configuration 

The ```hdmi_data``` struct is used to store the configuration settings. The first members hold Video Information Code (VIC) values. These values can be manually typed in (see rotating_bars example) or the ```vic_table``` defined in src/hdmi/vic_table.c can be used (see examples static_image, resolution change and moving_square).

The following settings also need to be set in ```hdmi_data```

* **rgb_format** - The ordering of the Red, Blue, Green and Alpha channels. See ```RGB_FORMAT``` in include/hdmi/hdmi_data.h.
* **alpha_enable** - Whether or not the alpha channel is present. For an example use see the example static_image.
* **mode** - Whether or not the image is static or moving.
* **ms_delay** - How long the each frame lasts for. For moving images, this is the time between the frame. For static images this is how long the image is displayed.

The length of time that moving images are displayed by is set by the ```MAX_FRAME_COUNT``` in src/api/api.c.

### Writing to the buffer 

The function to write the frame buffer is set to the member ```write_fb``` in the ```display_config``` struct. In the examples this is implemented as ```write_frame_buffer()```.

A pointer to the active or cached frame buffer can be retrieved using the following two functions where N is the size of the pointer in bytes ```get_active_frame_buffer_uintN()``` ```get_cache_frame_buffer_uintN()```. 8, 32 and 64 bit pointers are currently implemented. See src/api/framebuffer.c.

The width and the height are defined by ```hdmi_data.H_ACTIVE``` and ```hdmi_data.V_ACTIVE```. A loop can be set up to read left to right and top to bottom to access and modify the data inside the frame buffer. 

For moving images, global variables are used to keep track of frame data (see example moving_square and rotating_bars). It is set up this way so that the dcss PD can notify the client PD when the frame buffer is ready to be modified and so that the client PD can notify the dcss PD when the frame buffer has finished being written to.

### Empty client

An empty example has been provided in empty_client/empty_client.c. To use the driver uncomment ```static_image()``` or ```moving_image()``` to choose the type of image you wish to see. Then implement init_static_example by initialising all fields of the ```hdmi_data``` struct. Then modify ```write_static_frame_buffer()``` with your desired image.

Modify empty_client/Makefile to add extra files or build configurations specific to the example.
