package com.fitatu.barcodescanner.fitatu_barcode_scanner

import android.app.Activity
import android.content.Intent
import android.provider.MediaStore.MediaColumns.ORIENTATION
import android.util.Log
import com.google.zxing.client.android.CaptureActivity
import com.google.zxing.client.android.Intents
import com.google.zxing.client.android.Intents.Scan.FORMATS
import com.google.zxing.client.android.Intents.Scan.SHOW_FLIP_CAMERA_BUTTON
import com.google.zxing.client.android.Intents.Scan.SHOW_TORCH_BUTTON
import com.google.zxing.client.android.Intents.Scan.TORCH_ON
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding
import io.flutter.plugin.common.PluginRegistry
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject

class LegacyFitatuBarcodeScanner(
    private val flutterApi: FitatuBarcodeScannerFlutterApi,
    activityPluginBinding: ActivityPluginBinding
) : PluginRegistry.ActivityResultListener, LegacyFitatuBarcodeScannerHostApi {

    private val activity = activityPluginBinding.activity

    init {
        activityPluginBinding.addActivityResultListener(this)
    }

    companion object {
        private const val PROMPT = "prompt"
        private const val RESULTDISPLAY_DURATION = "resultDisplayDuration"
        private const val PREFER_FRONTCAMERA = "preferFrontCamera"
    }

    private fun scanIntent(args: JSONArray = JSONArray()) {
        val intentScan = Intent(
            activity,
            CaptureActivity::class.java
        )
        intentScan.action = Intents.Scan.ACTION
        intentScan.addCategory(Intent.CATEGORY_DEFAULT)

        // add config as intent extras

        // add config as intent extras
        if (args.length() > 0) {
            var obj: JSONObject
            var names: JSONArray
            var key: String?
            var value: Any?
            for (i in 0 until args.length()) {
                obj = try {
                    args.getJSONObject(i)
                } catch (e: JSONException) {
                    Log.i("FlutterLog", e.localizedMessage)
                    continue
                }
                names = obj.names()
                for (j in 0 until names.length()) {
                    try {
                        key = names.getString(j)
                        value = obj[key]
                        if (value is Int) {
                            intentScan.putExtra(key, value as Int?)
                        } else if (value is String) {
                            intentScan.putExtra(key, value as String?)
                        }
                    } catch (e: JSONException) {
                        Log.i("CordovaLog", e.localizedMessage)
                    }
                }
                intentScan.putExtra(
                    Intents.Scan.CAMERA_ID,
                    if (obj.optBoolean(PREFER_FRONTCAMERA, false)) 1 else 0
                )
                intentScan.putExtra(
                    SHOW_FLIP_CAMERA_BUTTON,
                    obj.optBoolean(SHOW_FLIP_CAMERA_BUTTON, false)
                )
                intentScan.putExtra(SHOW_TORCH_BUTTON, obj.optBoolean(SHOW_TORCH_BUTTON, false))
                intentScan.putExtra(TORCH_ON, obj.optBoolean(TORCH_ON, false))
                if (obj.has(RESULTDISPLAY_DURATION)) {
                    intentScan.putExtra(
                        Intents.Scan.RESULT_DISPLAY_DURATION_MS,
                        "" + obj.optLong(RESULTDISPLAY_DURATION)
                    )
                }
                if (obj.has(FORMATS)) {
                    intentScan.putExtra(FORMATS, obj.optString(FORMATS))
                }
                if (obj.has(PROMPT)) {
                    intentScan.putExtra(Intents.Scan.PROMPT_MESSAGE, obj.optString(PROMPT))
                }
                if (obj.has(ORIENTATION)) {
                    intentScan.putExtra(Intents.Scan.ORIENTATION_LOCK, obj.optString(ORIENTATION))
                }
            }
        }

        // avoid calling other phonegap apps
        intentScan.setPackage(activity.packageName)
        activity.startActivityForResult(intentScan, 1)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?): Boolean {
        if (requestCode == 1 && resultCode == Activity.RESULT_OK) {
            val result = data?.getStringExtra("SCAN_RESULT")
            val format = data?.getStringExtra("SCAN_RESULT_FORMAT")
            flutterApi.result(result) {}
            return true
        }
        return false
    }

    override fun scan() {
        scanIntent()
    }

    override fun setTorchEnabled(isEnabled: Boolean) {
    }
}
