/*
* author: Randy Chuang
* created time: May 31, 2019
* E-mail: sharmane578@gmail.com
* illustration: This is the header file for webcam setting.	It include the "Config.hpp" which is the top-level configuration sheet of setting.
*/

//File: webcam_setting.hpp

#ifndef WEBCAM_SETTING_HPP
#define WEBCAM_SETTING_HPP

#include<opencv2/opencv.hpp>

#include"Config.hpp"
int webcam_set(cv::VideoCapture &cap);

#endif // !WEBCAM_SETTING_HPP
