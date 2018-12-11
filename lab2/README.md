# USB CAM MODULE
## Structure
├── application
│   ├── app
│   ├── app.c
│   ├── dht_data.h
│   ├── pics
│   │   └── test.jpg
│   └── usb_cam.h
└── module
    ├── Makefile
    ├── reload.sh
    ├── rmuvc.sh
    ├── usb_cam.c
    ├── usb_cam.h
    └── usbvideo.h

## Application
* Compilation: `cd application && gcc -o app app.c`
* Commandes:
	* Reset: `./app reset`
	* Streamon/off: `./app streamon` or `./app streamoff`
	* Photo: `./app selfie <path/to/pics>` 
		* NB: <filename>.jpg will be created automatically

## Module
* Compilation: `cd module && make`
* Load: `sh reload.sh`
* Remove UVC modules: `sh rmuvc.sh`
