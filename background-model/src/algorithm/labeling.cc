#include "labeling.h"

#include "../debug.h"

#include <vector>
#include <algorithm>

cv::Mat label(cv::Mat const foregroundMask) {

	cv::Mat labels, stats, centroids, result;

	int nccomps = cv::connectedComponentsWithStats(foregroundMask, labels, stats, centroids);

	debug("Computed components with stats...");
	std::vector<cv::Vec3b> colors(nccomps + 1);

	colors[0] = cv::Vec3b(0, 0, 0);

	for (int i = 0; i < nccomps; i++) {
		colors[i] = cv::Vec3b(rand() % 256, rand() % 256, rand() % 256);

		if (stats.at<int>(i - 1, cv::CC_STAT_AREA) < 100)
			colors[i] = cv::Vec3b(0, 0, 0);
	}

	result = cv::Mat::zeros(foregroundMask.size(), CV_8UC3);

	for (int y = 0; y < foregroundMask.rows; y++) {
		for (int x = 0; x < foregroundMask.cols; x++) {
			int label = labels.at<int>(y, x);
			result.at<cv::Vec3b>(y, x) = colors[label];
		}
	}

	return result;
}

void label_buffer(unsigned char *buffer_in, unsigned char *buffer_out, int width, int height) {
	fdebug("Performing algorithm computations on buffers: in=%p, out=%p, %dx%d.", buffer_in, buffer_out, width, height);
	cv::Mat input = cv::Mat(height, width, CV_8UC3, buffer_in);
	input = input.clone();
	cv::Mat result = label(input);

	std::memcpy(buffer_out, result.data, result.total() * result.elemSize());
	debug("Algorithm finished successfully.");
}
