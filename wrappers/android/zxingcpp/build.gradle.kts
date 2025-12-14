@file:Suppress("UnstableApiUsage")

plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
    id("com.vanniktech.maven.publish") version "0.35.0"
}

// Determine the Java version from the current JVM running Gradle.
// This was the only way to make it compile on axxel's Android Studio based dev-env as well as on the CI build.
val jvmVersion = JavaVersion.toVersion(System.getProperty("java.version"))

android {
    namespace = "zxingcpp.lib" // used to be just zxingcpp but needs to contain a '.' in release builds
    // ndk version 27 has sufficient c++20 support to enable all features (see #386)
    // ndkVersion = "27.0.12077973"

    defaultConfig {
        compileSdk = libs.versions.androidCompileSdk.get().toInt()
        minSdk = libs.versions.androidMinSdk.get().toInt()

        ndk {
            // speed up build: compile only arm versions
            // abiFilters += listOf("armeabi-v7a", "arm64-v8a")
        }
        externalNativeBuild {
            cmake {
                arguments(
                    "-DCMAKE_BUILD_TYPE=RelWithDebInfo",
                    "-DANDROID_ARM_NEON=ON",
                    "-DZXING_WRITERS=OFF",
                    "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON" // This flag can be removed when NDK 28 is the default version
                )
            }
        }

        consumerProguardFiles("consumer-rules.pro")
    }
    compileOptions {
        sourceCompatibility = jvmVersion
        targetCompatibility = jvmVersion
    }
//  kotlin {
//      jvmToolchain(17) // defaults to the JDK version used by Gradle
//  }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
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

val publishSnapshot: String? by project
group = "io.github.zxing-cpp"
version = "2.3.1" + if (publishSnapshot == "true") "-SNAPSHOT" else ""

val javadocJar by tasks.registering(Jar::class) {
    archiveClassifier.set("javadoc")
}

mavenPublishing {
    publishToMavenCentral()
    signAllPublications()

    coordinates(project.group.toString(), "android", project.version.toString())

    pom {
        name.set("zxing-cpp")
        description.set("Wrapper for zxing-cpp barcode image processing library")
        url.set("https://github.com/zxing-cpp/zxing-cpp")
        licenses {
            license {
                name.set("The Apache License, Version 2.0")
                url.set("http://www.apache.org/licenses/LICENSE-2.0.txt")
                distribution.set("http://www.apache.org/licenses/LICENSE-2.0.txt")
            }
        }
        developers {
            developer {
                id.set("zxing-cpp")
                name.set("zxing-cpp community")
                email.set("zxingcpp@gmail.com")
            }
        }
        scm {
            url.set("https://github.com/zxing-cpp/zxing-cpp")
            connection.set("scm:git:git://github.com/zxing-cpp/zxing-cpp.git")
            developerConnection.set("scm:git:git://github.com/zxing-cpp/zxing-cpp.git")
        }
    }
}
