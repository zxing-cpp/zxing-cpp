import io.github.isning.gradle.plugins.cmake.params.CustomCMakeParams
import io.github.isning.gradle.plugins.cmake.params.entries.CustomCMakeCacheEntries
import io.github.isning.gradle.plugins.cmake.params.entries.asCMakeParams
import io.github.isning.gradle.plugins.cmake.params.entries.platform.ModifiablePlatformEntriesImpl
import io.github.isning.gradle.plugins.cmake.params.entries.plus
import io.github.isning.gradle.plugins.cmake.params.plus
import io.github.isning.gradle.plugins.kn.krossCompile.invoke
import org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget
import org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTargetWithHostTests
import org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTargetWithSimulatorTests
import org.jetbrains.kotlin.gradle.targets.native.DefaultHostTestRun
import org.jetbrains.kotlin.gradle.targets.native.DefaultSimulatorTestRun
import java.util.*

plugins {
    alias(libs.plugins.kotlinMultiplatform)
    alias(libs.plugins.krossCompile)
    id("com.vanniktech.maven.publish") version "0.35.0"
}

group = "io.github.zxing-cpp"
version = "3.0.0"

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
    val androidTargets by lazy {
        listOf(
            androidNativeArm32(),
            androidNativeArm64(),
            androidNativeX86(),
            androidNativeX64(),
        )
    }
    val appleTargets by lazy {
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
    val linuxTargets by lazy {
        listOf(
            linuxX64(),
            linuxArm64(),
        )
    }
//    val windowsTargets by lazy {
//        listOf(
//            mingwX64(),
//        )
//    }
    val enabledTargetList = mutableListOf<KotlinNativeTarget>()
    enabledTargetList.addAll(androidTargets)
    enabledTargetList.addAll(linuxTargets)
    // TODO: Linking failed, keep up with https://youtrack.jetbrains.com/issue/KT-65671
//    enabledTargetList.addAll(windowsTargets)

    if (hostOs == "Mac OS X") enabledTargetList.addAll(appleTargets)

    // Enable testing with shared libraries for Linux targets
    linuxTargets.forEach { target ->
        val soPath = "build/cmake/libZXing/${target.name}/out/lib"
        when (target) {
            is KotlinNativeTargetWithHostTests -> {
                target.testRuns.withType<DefaultHostTestRun> {
                    executionTask.configure {
                        environment("LD_LIBRARY_PATH", soPath)
                    }
                }
            }

            is KotlinNativeTargetWithSimulatorTests -> {
                target.testRuns.withType<DefaultSimulatorTestRun> {
                    executionTask.configure {
                        environment("LD_LIBRARY_PATH", soPath)
                    }
                }
            }
        }

        // Link to the shared library built by kross-compile
        val test by target.compilations.getting
        test.compileTaskProvider.configure {
            compilerOptions.freeCompilerArgs.addAll(
                "-linker-options",
                "-L$soPath -lZXing",
            )
        }

        // Customizable linker options for test compilation
        (project.properties["${target.name}.test.compilerOptions"] as? String)?.let {
            test.compileTaskProvider.configure {
                compilerOptions.freeCompilerArgs.addAll(
                    it.split(",").map(String::trim)
                )
            }
        }
    }


    sourceSets {
        val commonMain by getting
        val nativeMain by creating
        val nativeTest by creating
    }
}

krossCompile {
    libraries {
        val cmakeDir = project.layout.buildDirectory.dir("cmake").get().asFile.absolutePath

        /**
         * At the moment, we build all targets in a uniform way:
         * most of them are compiled as static libraries and linked accordingly.
         *
         * Linux is the only exception. For Linux targets, we override this behavior
         * and build shared libraries instead, because the Kotlin toolchain currently
         * prevents us from enabling C++20 support:
         * https://github.com/zxing-cpp/zxing-cpp/issues/939
         *
         * We do NOT ship prebuilt Linux binaries as part of the Kotlin/Native
         * distribution. Linux consumers are expected to provide and distribute the
         * required shared library themselves.
         *
         * The Linux shared-library build is integrated into Gradle mainly to support
         * development workflows: it produces the header files required for cinterop
         * and allows us to run tests directly against the resulting dynamic library,
         * without having to build zxing-cpp manually.
         */
        val libZXing by creating {
            sourceDir = file("../../core").absolutePath
            outputPath = "out"
            libraryArtifactNames = listOf("lib/libZXing.a")

            val buildDir = "$cmakeDir/{libraryName}/{targetName}"
            val installDir = listOf("$cmakeDir/{libraryName}/{targetName}", outputPath).filter { isNotEmpty() }.joinToString("/")

            /**
             * For reference, see: https://kotlinlang.org/docs/native-definition-file.html#properties
             */
            cinterop {
                packageName = "zxingcpp.cinterop"
                headers = listOf("ZXingC.h")
                includeDirs.from("$installDir/include/ZXing")
                strictEnums = listOf(
                    "ZXing_ContentType",
                    "ZXing_Binarizer",
                    "ZXing_EanAddOnSymbol",
                    "ZXing_TextMode",
                    "ZXing_ImageFormat",
                    "ZXing_BarcodeFormat",
                )
                userSetupHint = "Due to Kotlin toolchain limitations (C++20 cannot be enabled), the Kotlin/Native " +
                        "wrapper of zxing-cpp does not ship prebuilt binaries for Linux targets in its klibs. " +
                        "Linux users must provide and distribute the required shared library themselves. " +
                        "For details, see: https://github.com/zxing-cpp/zxing-cpp/issues/939."
            }
            cmake.apply {
                val buildDirCmake = buildDir.replace("{libraryName}","{projectName}")
                val installDirCmake = installDir.replace("{libraryName}","{projectName}")
                configParams {
                    this.buildDir = buildDirCmake
                }
                configParams += (ModifiablePlatformEntriesImpl().apply {
                    buildType = "Release"
                    buildSharedLibs = false
                } + CustomCMakeCacheEntries(
                    mapOf(
                        "CMAKE_INSTALL_PREFIX" to installDirCmake,
                        "ZXING_READERS" to "ON",
                        "ZXING_WRITERS" to "ON",
                        "ZXING_EXPERIMENTAL_API" to "ON",
                        "ZXING_USE_BUNDLED_ZINT" to "ON",
                        "ZXING_C_API" to "ON",
                    )
                )).asCMakeParams
                buildParams {
                    this.buildDir = buildDirCmake
                    config = "Release"
                    target = "install"
                }
                buildParams += CustomCMakeParams(listOf("-j16"))
            }

            androidNativeX64.ndk()
            androidNativeX86.ndk()
            androidNativeArm32.ndk()
            androidNativeArm64.ndk()

            /**
             * We use zig provided sysroot for cross compilation convenience for Linux targets
             *
             * Although Zig is often described as “another language compatible with C,”
             * in this context we only use it for the cross-compilation sysroot it provides,
             * in order to build Linux targets.
             * In practice, it is essentially a wrapper around Clang, combined with a set of tools
             * that handle tasks such as downloading and managing sysroots.
             * It also makes it easy to specify details like the glibc version used by the sysroot.
             */
            linuxX64.zig {
                libraryArtifactNames = listOf()
                cmake {
                    configParams += (ModifiablePlatformEntriesImpl().apply {
                        buildSharedLibs = true
                    }).asCMakeParams
                }
            }
            linuxArm64.zig {
                libraryArtifactNames = listOf()
                cmake {
                    configParams += (ModifiablePlatformEntriesImpl().apply {
                        buildSharedLibs = true
                    }).asCMakeParams
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

mavenPublishing {
    publishToMavenCentral()
    signAllPublications()

    coordinates(project.group.toString(), "kotlin-native", project.version.toString())
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
