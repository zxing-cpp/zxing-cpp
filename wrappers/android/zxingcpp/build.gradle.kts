plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
}

android {
    namespace = "com.zxingcpp"
    // ndk version 25 is known to support c++20 (see #386)
    // ndkVersion = "25.1.8937393"

    defaultConfig {
        compileSdk = libs.versions.androidCompileSdk.get().toInt()
        minSdk = libs.versions.androidMinSdk.get().toInt()

        ndk {
            // speed up build: compile only arm versions

            //noinspection ChromeOsAbiSupport
            abiFilters += "armeabi-v7a"

            //noinspection ChromeOsAbiSupport
            abiFilters += "arm64-v8a"
        }
        externalNativeBuild {
            cmake {
                arguments("-DCMAKE_BUILD_TYPE=RelWithDebInfo")
            }
        }
    }
    compileOptions {
        sourceCompatibility(JavaVersion.VERSION_1_8)
        targetCompatibility(JavaVersion.VERSION_1_8)
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
    externalNativeBuild {
        cmake {
            path(file("src/main/cpp/CMakeLists.txt"))
        }
    }
    lint {
        disable.add("UnsafeExperimentalUsageError")
    }
}

kotlin {
    explicitApi()
}

dependencies {
    implementation(libs.androidx.camera.core)
}
