package zxingcpp

import kotlinx.cinterop.ByteVar
import kotlinx.cinterop.CPointer
import kotlinx.cinterop.ExperimentalForeignApi
import kotlinx.cinterop.toKString
import zxingcpp.cinterop.ZXing_free

@OptIn(ExperimentalForeignApi::class)
internal fun CPointer<ByteVar>.toKStringAndFree(): String = run {
	toKString().also { ZXing_free(this) }
}
