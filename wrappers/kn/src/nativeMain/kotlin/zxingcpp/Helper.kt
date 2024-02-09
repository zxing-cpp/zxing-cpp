package zxingcpp

data class YuvImageView(
	val yBuffer: ByteArray,
	override val rowStride: Int,
	override val left: Int,
	override val top: Int,
	override val width: Int,
	override val height: Int,
	override val rotation: Int,
) : ImageView() {
	override val format: ImageFormat = ImageFormat.Lum
	override val data: UByteArray
		get() = yBuffer.sliceArray(IntRange(top * rowStride + left, yBuffer.size - 1)).asUByteArray()

	override fun equals(other: Any?): Boolean {
		if (this === other) return true
		if ((other == null) || (other::class != YuvImageView::class)) return false

		other as YuvImageView

		if (!yBuffer.contentEquals(other.yBuffer)) return false
		if (rowStride != other.rowStride) return false
		if (left != other.left) return false
		if (top != other.top) return false
		if (width != other.width) return false
		if (height != other.height) return false
		if (rotation != other.rotation) return false

		return true
	}

	override fun hashCode(): Int {
		var result = yBuffer.contentHashCode()
		result = 31 * result + rowStride.hashCode()
		result = 31 * result + left.hashCode()
		result = 31 * result + top.hashCode()
		result = 31 * result + width.hashCode()
		result = 31 * result + height.hashCode()
		result = 31 * result + rotation.hashCode()
		return result
	}
}
