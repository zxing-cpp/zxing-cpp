/*
* Copyright 2022 Axel Waggershauser
* Copyright 2022 Markus Fisch
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

package com.example.zxingcppdemo

import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import android.view.View
import androidx.camera.core.ImageProxy
import kotlin.math.max
import kotlin.math.min

class PreviewOverlay constructor(context: Context, attributeSet: AttributeSet?) :
	View(context, attributeSet) {

	private val paintPath = Paint(Paint.ANTI_ALIAS_FLAG).apply {
		style = Paint.Style.STROKE
		color = 0xff00ff00.toInt()
		strokeWidth = 2 * context.resources.displayMetrics.density
	}
	private val paintRect = Paint().apply {
		style = Paint.Style.STROKE
		color = 0x80ffffff.toInt()
		strokeWidth = 3 * context.resources.displayMetrics.density
	}
	private val path = Path()
	private var cropRect = Rect()
	private var rotation = 0
	private var s: Float = 0f
	private var o: Float = 0f

	fun update(viewFinder: View, image: ImageProxy, points: List<PointF>?) {
		cropRect = image.cropRect
		rotation = image.imageInfo.rotationDegrees
		s = min(viewFinder.width, viewFinder.height).toFloat() / image.height
		o = (max(viewFinder.width, viewFinder.height) - (image.width * s).toInt()).toFloat() / 2

		path.apply {
			rewind()
			if (!points.isNullOrEmpty()) {
				moveTo(points.last().x, points.last().y)
				for (p in points)
					lineTo(p.x, p.y)
			}
		}
		invalidate()
	}

	override fun onDraw(canvas: Canvas) {
		canvas.apply {
			// draw the cropRect, which is relative to the original image orientation
			save()
			if (rotation == 90) {
				translate(width.toFloat(), 0f)
				rotate(rotation.toFloat())
			}
			translate(o, 0f)
			scale(s, s)
			drawRect(cropRect, paintRect)
			restore()

			// draw the path, which is relative to the (centered) rotated cropRect
			when (rotation) {
				0, 180 -> translate(o + cropRect.left * s, cropRect.top * s)
				90 -> translate(cropRect.top * s, o + cropRect.left * s)
			}
			scale(s, s)
			drawPath(path, paintPath)
		}
	}
}
