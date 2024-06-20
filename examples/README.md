These examples have been created to demonstrate the drivers capabilities.


## Static image

In this example a static image consisting of 4 equally spaced bars of red, blue, green and white are displayed on the screen.The display configuration settings in hdmi_data are set using the predefined array vic_table. The following hdmi_data members are set:

* **rgba_format** - This is set to RGBA, which defines the ordering of each 32bit memory region of the frame buffer to be split into 4 8bit addresses for each colour - Red, Blue, Green and Alpha.
* **alpha_enable** - This is set to ALPHA_ON which means that the alpha channel is present. In this example for each colour bar the value of the alpha channel is incremented every 3 pixels.
* **mode** - This is set to STATIC_IMAGE so that an additonal buffer is not used.
* **ms_delay** This is set to 30000 miliseconds.

## Resolution change

## Moving square

## Rotating bars


