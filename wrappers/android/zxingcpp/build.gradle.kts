plugins {
    alias(libs.plugins.android.library)
    alias(libs.plugins.kotlin.android)
    id("maven-publish")
    id("signing")
}

android {
    namespace = "zxingcpp"
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

group = "io.github.zxing-cpp"
version = "2.2.0"

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
            setUrl(if (version.toString().endsWith("SNAPSHOT")) snapshotsRepoUrl else releasesRepoUrl)

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
    val signingKey: String? by project
    val signingPassword: String? by project
    useInMemoryPgpKeys(signingKey, signingPassword)
    sign(publishing.publications)
}
