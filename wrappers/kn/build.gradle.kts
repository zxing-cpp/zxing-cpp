import io.github.isning.gradle.plugins.cmake.params.CustomCMakeParams
import io.github.isning.gradle.plugins.cmake.params.entries.CustomCMakeCacheEntries
import io.github.isning.gradle.plugins.cmake.params.entries.asCMakeParams
import io.github.isning.gradle.plugins.cmake.params.entries.platform.ModifiablePlatformEntriesImpl
import io.github.isning.gradle.plugins.cmake.params.entries.plus
import io.github.isning.gradle.plugins.cmake.params.plus
import io.github.isning.gradle.plugins.kn.krossCompile.invoke
import org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget
import java.util.*

plugins {
    alias(libs.plugins.kotlinMultiplatform)
    alias(libs.plugins.krossCompile)
    `maven-publish`
    signing
}

group = "io.github.zxing-cpp"
version = "2.3.0-SNAPSHOT"

Properties().apply {
    rootProject.file("local.properties").takeIf { it.exists() && it.isFile }?.let { load(it.reader()) }
}.onEach { (key, value) ->
    if (key is String) ext[key] = value
}

val hostOs = System.getProperty("os.name")

repositories {
    mavenCentral()
    google()
}

kotlin {
    val androidTargets = {
        listOf(
            androidNativeArm32(),
            androidNativeArm64(),
            androidNativeX86(),
            androidNativeX64(),
        )
    }
    val appleTargets = {
        listOf(
            iosX64(),
            iosArm64(),
            iosSimulatorArm64(),
            macosX64(),
            macosArm64(),
            watchosX64(),
            watchosArm32(),
            watchosArm64(),
            watchosSimulatorArm64(),
            tvosX64(),
            tvosArm64(),
            tvosSimulatorArm64(),
        )
    }
    val linuxTargets = {
        listOf(
            linuxX64(),
            linuxArm64(),
        )
    }
    // TODO: Linking failed, keep up with https://youtrack.jetbrains.com/issue/KT-65671
//    val windowsTargets = {
//        listOf(
//            mingwX64(),
//        )
//    }
    val enabledTargetList = mutableListOf<KotlinNativeTarget>()
    enabledTargetList.addAll(androidTargets())
    enabledTargetList.addAll(linuxTargets())
    // TODO: Linking failed, keep up with https://youtrack.jetbrains.com/issue/KT-65671
//    enabledTargetList.addAll(windowsTargets())

    if (hostOs == "Mac OS X") enabledTargetList.addAll(appleTargets())
}

krossCompile {
    libraries {
        val cmakeDir = project.layout.buildDirectory.dir("cmake").get().asFile.absolutePath
        val zxingCpp by creating {
            sourceDir = file("../../core").absolutePath
            outputPath = ""
            libraryArtifactNames = listOf("libZXing.a")

            cinterop {
                val buildDir = "$cmakeDir/{libraryName}/{targetName}"
                packageName = "zxingcpp.cinterop"
                includeDirs.from(buildDir)
                headers = listOf("$sourceDir/src/ZXingC.h")
                compilerOpts += "-DZXING_EXPERIMENTAL_API=ON"
            }
            cmake.apply {
                val buildDir = "$cmakeDir/{projectName}/{targetName}"
                configParams {
                    this.buildDir = buildDir
                }
                configParams += (ModifiablePlatformEntriesImpl().apply {
                    buildType = "Release"
                    buildSharedLibs = false
                } + CustomCMakeCacheEntries(
                    mapOf(
                        "ZXING_READERS" to "ON",
                        "ZXING_WRITERS" to "NEW",
                        "ZXING_EXPERIMENTAL_API" to "ON",
                        "ZXING_USE_BUNDLED_ZINT" to "ON",
                        "ZXING_C_API" to "ON",
                    )
                )).asCMakeParams
                buildParams {
                    this.buildDir = buildDir
                    config = "Release"
                }
                buildParams += CustomCMakeParams(listOf("-j16"))
            }

            androidNativeX64.ndk()
            androidNativeX86.ndk()
            androidNativeArm32.ndk()
            androidNativeArm64.ndk()

            // TODO: Find a way to build linux targets with cxx20. Detail: https://github.com/zxing-cpp/zxing-cpp/pull/719#discussion_r1485701269
            linuxX64.konan {
                cmake {
                    configParams += CustomCMakeCacheEntries(
                        mapOf(
                            "CMAKE_CXX_STANDARD" to "17",
                        )
                    ).asCMakeParams
                }
            }
            linuxArm64.konan {
                cmake {
                    configParams += CustomCMakeCacheEntries(
                        mapOf(
                            "CMAKE_CXX_STANDARD" to "17",
                        )
                    ).asCMakeParams
                }
            }
            // TODO: Linking failed, keep up with https://youtrack.jetbrains.com/issue/KT-65671
//            mingwX64.konan()

            if (hostOs == "Mac OS X") {
                iosX64.xcode()
                iosArm64.xcode()
                iosSimulatorArm64.xcode()
                macosX64.xcode()
                macosArm64.xcode()
                watchosX64.xcode()
                watchosArm32.xcode()
                watchosArm64.xcode()
                watchosSimulatorArm64.xcode()
                tvosX64.xcode()
                tvosArm64.xcode()
                tvosSimulatorArm64.xcode()
            }
        }
    }
}

publishing {
    publications.withType<MavenPublication>().all {
        artifactId = artifactId.replace(project.name, "kotlin-native")
            groupId = project.group.toString()
            version = project.version.toString()

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
    if (signingKey != null && signingPassword != null) {
        useInMemoryPgpKeys(signingKey, signingPassword)
        sign(publishing.publications)
    }
}
