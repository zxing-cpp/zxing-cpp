/*
* Copyright 2016 Nu-book Inc.
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
package com.zxing;

import android.graphics.Bitmap;

public class BarcodeReader
{
	public static class Result
	{
		public BarcodeFormat getFormat() {
			return format;
		}

		public String getText() {
			return text;
		}

		Result(BarcodeFormat format, String text) {
			this.format = format;
			this.text = text;
		}

		private BarcodeFormat format;
		private String text;
	}

	public BarcodeReader(BarcodeFormat... formats)
	{
		int[] nativeFormats = new int[formats.length];
		for (int i = 0; i < formats.length; ++i) {
			nativeFormats[i] = formats[i].ordinal();
		}
		_nativePtr = createInstance(nativeFormats);
	}

	public Result read(Bitmap bitmap, int cropWidth, int cropHeight)
	{
		int imgWidth = bitmap.getWidth();
		int imgHeight = bitmap.getHeight();
		cropWidth = cropWidth <= 0 ? imgWidth : Math.min(imgWidth, cropWidth);
		cropHeight = cropHeight <= 0 ? imgHeight : Math.min(imgHeight, cropHeight);
		int cropLeft = (imgWidth - cropWidth) / 2;
		int cropTop = (imgHeight - cropHeight) / 2;
		Object[] result = new Object[1];
		int resultFormat = readBarcode(_nativePtr, bitmap, cropLeft, cropTop, cropWidth, cropHeight, result);
		if (resultFormat >= 0)
		{
			return new Result(BarcodeFormat.values()[resultFormat], (String)result[0]);
		}
		return null;
	}

	@Override
	protected void finalize() throws Throwable
	{
		try
		{
			if (_nativePtr != 0)
			{
				destroyInstance(_nativePtr);
				_nativePtr = 0;
			}
		}
		finally
		{
			super.finalize();
		}
	}


	private long _nativePtr = 0;

	private static native long createInstance(int[] formats);
	private static native void destroyInstance(long objPtr);
	private static native int readBarcode(long objPtr, Bitmap bitmap, int left, int top, int width, int height, Object[] result);

	static {
		System.loadLibrary("zxing-android");
	}

}
