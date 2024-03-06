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
version = "2.2.1"

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
        val zxingCpp by creating {
            sourceDir = file("../../core").absolutePath
            outputPath = ""
            libraryArtifactNames = listOf("libZXing.a")

            cinterop {
                packageName = "zxingcpp.cinterop"
                headers = listOf("$sourceDir/src/ZXingC.h")
            }
            cmake.apply {
                val buildPath = project.layout.buildDirectory.dir("cmake").get().asFile.absolutePath +
                        "/{projectName}/{targetName}"
                configParams {
                    buildDir = buildPath
                }
                configParams += (ModifiablePlatformEntriesImpl().apply {
                    buildType = "Release"
                    buildSharedLibs = false
                } + CustomCMakeCacheEntries(
                    mapOf(
                        "BUILD_READERS" to "ON",
                        "BUILD_WRITERS" to "OFF",
                        "BUILD_C_API" to "ON",
                    )
                )).asCMakeParams
                buildParams {
                    buildDir = buildPath
                    config = "Release"
                }
                buildParams += CustomCMakeParams(listOf("-j16"))
            }

            androidNativeX64.konan()
            androidNativeX86.konan()
            androidNativeArm32.konan()
            androidNativeArm64.konan()

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
                iosX64.konan()
                iosArm64.konan()
                iosSimulatorArm64.konan()
                macosX64.konan()
                macosArm64.konan()
                watchosX64.konan()
                watchosArm32.konan()
                watchosArm64.konan()
                watchosSimulatorArm64.konan()
                tvosX64.konan()
                tvosArm64.konan()
                tvosSimulatorArm64.konan()
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
