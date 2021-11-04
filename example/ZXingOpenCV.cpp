/*
 * Copyright 2021 Axel Waggershauser
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
			auto res = ReadBarcode(image);
			if (res.isValid())
				DrawResult(image, res);
			imshow("Display window", image);
		}

	return 0;
}
