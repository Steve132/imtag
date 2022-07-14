#include "cvConnectedComponentsWithStats.h"
#include <opencv2/opencv.hpp>

std::vector<int> cvConnectedComponentsWithStats(const uint8_t* image, const size_t width, const size_t height, const int connectivity, const bool onlyBenchmark)
{
	cv::Mat centroids, stats;
	cv::Mat bw(height, width, CV_8UC1, const_cast<unsigned char*>(image));

	cv::Mat labelImage(height, width, CV_32S);
	int nLabels = cv::connectedComponentsWithStats(bw, labelImage, stats, centroids, connectivity);

	if(!onlyBenchmark)
	{
		std::vector<int> labelImg;
		labelImg.reserve(height*width);
		if(labelImage.isContinuous())
			labelImg.assign((int*)labelImage.data, (int*)labelImage.data + labelImage.total());
		else
		{
			for (int i = 0; i < labelImage.rows; ++i)
				labelImg.insert(labelImg.end(), labelImage.ptr<int>(i), labelImage.ptr<int>(i) + labelImage.cols);
		}
		return labelImg;
	}
	return {};
}
