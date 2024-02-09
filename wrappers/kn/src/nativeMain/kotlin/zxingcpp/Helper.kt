package zxingcpp

data class LumImageView(
    override val data: UByteArray,
	override val rowStride: Int,
	override val left: Int,
	override val top: Int,
	override val width: Int,
	override val height: Int,
	override val rotation: Int,
) : ImageView() {
	override val format: ImageFormat = ImageFormat.Lum
	override val dataOffset: Int
		get() = top * rowStride + left

	override fun equals(other: Any?): Boolean {
		if (this === other) return true
        if ((other == null) || (other::class != LumImageView::class)) return false

        other as LumImageView

        if (!data.contentEquals(other.data)) return false
		if (rowStride != other.rowStride) return false
		if (left != other.left) return false
		if (top != other.top) return false
		if (width != other.width) return false
		if (height != other.height) return false
		if (rotation != other.rotation) return false

		return true
	}

	override fun hashCode(): Int {
        var barcode = data.contentHashCode()
		barcode = 31 * barcode + rowStride.hashCode()
		barcode = 31 * barcode + left.hashCode()
		barcode = 31 * barcode + top.hashCode()
		barcode = 31 * barcode + width.hashCode()
		barcode = 31 * barcode + height.hashCode()
		barcode = 31 * barcode + rotation.hashCode()
		return barcode
	}
}
