/*
 * Copyright 2007 ZXing authors
 * Copyright 2022 KURZ Digital Solutions GmbH & Co. KG
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
#include "Dimension.h"

using ZXing::MicroQRCode::Dimension;

 /**
	* Will try to compute dimension by choosing the bigger one if given dimension is on the border
	* between two dimensions
	* @param estimatedDimension The estimated dimension
	* @return The real dimension
	*/
int Dimension::computeRoundUp(int estimatedDimension)
{
	if (estimatedDimension < 12) 
	{
		return 11;
	}
	else if (estimatedDimension >= 12 && estimatedDimension < 14) 
	{
		return 13;
	}
	else if (estimatedDimension >= 14 && estimatedDimension < 16) 
	{
		return 15;
	}
	return 17;
}

/**
  * Will try to compute dimension by choosing the smaller one if given dimension is on the border
  * between two dimensions
  * @param estimatedDimension The estimated dimension
  * @return The real dimension
  */
int Dimension::computeRoundOff(int estimatedDimension) {
	switch (estimatedDimension & 0x03) { // mod 4
	case 0:
		estimatedDimension++;
		break;
	case 2:
		estimatedDimension--;
		break;
	}
	return estimatedDimension;
}
