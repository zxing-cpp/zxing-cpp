plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
    id("maven-publish")
    id("signing")
}

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

val publishSnapshot: String? by project
group = "io.github.zxing-cpp"
version = "2.4.0" + if (publishSnapshot == "true") "-SNAPSHOT" else ""

val javadocJar by tasks.registering(Jar::class) {
    archiveClassifier.set("javadoc")
}

publishing {
    publications {
        register<MavenPublication>("release") {
            artifactId = "android"
            groupId = project.group.toString()
            version = project.version.toString()

            afterEvaluate {
                from(components["release"])
            }

            artifact(javadocJar.get())

            pom {
                name = "zxing-cpp"
                description = "Wrapper for zxing-cpp barcode image processing library"
                url = "https://github.com/zxing-cpp/zxing-cpp"
                licenses {
                    license {
                        name = "The Apache License, Version 2.0"
                        url = "http://www.apache.org/licenses/LICENSE-2.0.txt"
                    }
                }
                developers {
                    developer {
                        id = "zxing-cpp"
                        name = "zxing-cpp community"
                        email = "zxingcpp@gmail.com"
                    }
                }
                scm {
                    connection = "scm:git:git://github.com/zxing-cpp/zxing-cpp.git"
                    developerConnection = "scm:git:git://github.com/zxing-cpp/zxing-cpp.git"
                    url = "https://github.com/zxing-cpp/zxing-cpp"
                }
            }
        }
    }
    repositories {
        maven {
            name = "sonatype"

            val releasesRepoUrl = "https://s01.oss.sonatype.org/service/local/staging/deploy/maven2/"
            val snapshotsRepoUrl = "https://s01.oss.sonatype.org/content/repositories/snapshots/"
            setUrl(if (version.toString().endsWith("-SNAPSHOT")) snapshotsRepoUrl else releasesRepoUrl)

            credentials {
                val ossrhUsername: String? by project
                val ossrhPassword: String? by project
                username = ossrhUsername
                password = ossrhPassword
            }
        }
    }
}

signing {
    setRequired {
        // signing is required if the artifacts are to be published
        gradle.taskGraph.allTasks.any { it is PublishToMavenRepository }
    }
    val signingKey: String? by project
    val signingPassword: String? by project
    useInMemoryPgpKeys(signingKey, signingPassword)
    sign(publishing.publications)
}
