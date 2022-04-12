package com.example.zxingcppdemo

import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import android.view.View

class DetectorView : View {
	private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
	private val path = Path()

	init {
		paint.style = Paint.Style.STROKE
		paint.color = 0xffffffff.toInt()
		paint.strokeWidth = context.resources.displayMetrics.density
	}

	constructor(context: Context, attrs: AttributeSet) :
			super(context, attrs)

	constructor(context: Context, attrs: AttributeSet, defStyleAttr: Int) :
			super(context, attrs, defStyleAttr)

	fun update(points: List<PointF>? = null) {
		path.apply {
			rewind()
			if (!points.isNullOrEmpty()) {
				moveTo(points[0])
				val size = points.size
				for (i in 1 until size) {
					lineTo(points[i])
				}
				close()
			}
		}
		invalidate()
	}

	override fun onDraw(canvas: Canvas) {
		canvas.drawColor(0, PorterDuff.Mode.CLEAR)
		if (!path.isEmpty) {
			canvas.drawPath(path, paint)
		}
	}
}

private fun Path.lineTo(point: PointF) = lineTo(point.x, point.y)
private fun Path.moveTo(point: PointF) = moveTo(point.x, point.y)
