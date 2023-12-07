plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
}

android {
    namespace = "zxingcpp.app"
    defaultConfig {
        applicationId = "io.github.zxingcpp.app"
        compileSdk = libs.versions.androidCompileSdk.get().toInt()
        minSdk = 26 // for the adaptive icons. TODO: remove adaptive icons and lower to API 21
        targetSdk = libs.versions.androidTargetSdk.get().toInt()
        versionCode = 1
        versionName = "1.0"
    }
    buildFeatures {
        viewBinding = true
    }
    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
    lint {
        disable.add("UnsafeExperimentalUsageError")
    }
}

dependencies {
    implementation(project(":zxingcpp"))

    implementation(libs.androidx.appcompat)
    implementation(libs.androidx.constraintlayout)
    implementation(libs.androidx.core)
    implementation(libs.androidx.camera.camera2)
    implementation(libs.androidx.camera.lifecycle)
    implementation(libs.androidx.camera.view)
    implementation(libs.android.material)

    // Java "upstream" version of zxing (to compare performance)
    implementation(libs.zxing.core)
}
