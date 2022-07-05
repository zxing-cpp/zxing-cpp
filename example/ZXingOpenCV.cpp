/*
 * Copyright 2021 Axel Waggershauser
*/
// SPDX-License-Identifier: Apache-2.0

#include "ZXingOpenCV.h"

#include <iostream>

using namespace cv;

int main()
{
	namedWindow("Display window");

	Mat image;
	VideoCapture cap(0);

	if (!cap.isOpened())
		std::cout << "cannot open camera";
	else
		while (waitKey(25) != 27) {
			cap >> image;
			auto results = ReadBarcodes(image);
			for (auto& r : results)
				DrawResult(image, r);
			imshow("Display window", image);
		}

	return 0;
}
