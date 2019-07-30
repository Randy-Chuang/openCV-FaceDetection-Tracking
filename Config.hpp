/*
* author: Randy Chuang
* created time: July 5, 2019
* E-mail: sharmane578@gmail.com
* illustration: This file is used for configuration of webcam properties.
*				We can just simply change the setting in this configuration sheet and call the function to do the setting (openCV).
*				
* note1:	The pixel data format we use is YUY2 which is one downsampling type of YUV. There is another data format widely used in webcam 
*			called mjpeg (compressed data format).
			More information: https://www.fourcc.org/yuv.php
*/

//File: Config.hpp

#ifndef CONFIG_HPP
#define CONFIG_HPP

/*-------------webcam properties----------------*/
#define webcam_Selection 0		// same as the openCV. If we are using notebook as platform.  0 ==> on board webcam of notebook, 1 ==> the second webcam connected with USB
#define webcam_Capture_API 0	// 0 ==> CAP_DSHOW (windows), 1 ==> CAP_V4L2 (Linux)
#define webcam_FOURCC 0			// 0 ==> YUY2 (hex code: 0x32595559) , more information: https://www.fourcc.org/yuv.php
#define webcam_Width 640
#define webcam_Height 480
#define webcam_FPS 30

#endif // !CONFIG_HPP
