#ifndef SRC_ALGORITH_LABELING_H_
#define SRC_ALGORITH_LABELING_H_

#include <opencv2/opencv.hpp>

cv::Mat label(cv::Mat in);
void label_buffer(unsigned char *buffer_in, unsigned char *buffer_out, int width, int height);

#endif
