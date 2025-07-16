import org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget
import java.util.*

plugins {
    alias(libs.plugins.kotlinMultiplatform)
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
    val windowsTargets = {
        listOf(
            mingwX64(),
        )
    }
    val enabledTargetList = mutableListOf<KotlinNativeTarget>()
    enabledTargetList.addAll(androidTargets())
    enabledTargetList.addAll(linuxTargets())
    enabledTargetList.addAll(windowsTargets())

    if (hostOs == "Mac OS X") enabledTargetList.addAll(appleTargets())

    enabledTargetList.forEach { target ->
        val main by target.compilations.getting
        val test by target.compilations.getting
        val libZXing by main.cinterops.creating {
            packageName = "zxingcpp.cinterop"
        }

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
