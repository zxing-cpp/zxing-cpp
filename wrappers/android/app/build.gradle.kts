plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
}

// Determine the Java version from the current JVM running Gradle.
// This was the only way to make it compile on axxels Android Studio based dev-env as well as on the CI build.
val jvmVersion = JavaVersion.toVersion(System.getProperty("java.version"))

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
        sourceCompatibility = jvmVersion
        targetCompatibility = jvmVersion
    }
//  kotlin {
//      jvmToolchain(17) // defaults to the JDK version used by Gradle
//  }
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
