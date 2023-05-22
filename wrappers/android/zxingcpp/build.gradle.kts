import java.io.FileInputStream
import java.util.Properties

plugins {
    id("com.android.library")
    id("kotlin-android")
    id("maven-publish")
    id("org.jetbrains.kotlin.android")
}

group = "com.zxingcpp"
version = "2.0.0"
val kotlinVersion = "1.7.10"

android {
    compileSdk = 33
    buildToolsVersion("32.0.0")
    // ndk version 25 is known to support c++20 (see #386)
    // ndkVersion "25.1.8937393"

    defaultConfig {
        minSdk = 21
        targetSdk = 33

        ndk {
            // speed up build: compile only arm versions
            abiFilters.add("armeabi-v7a")
            abiFilters.add("arm64-v8a")
        }
        externalNativeBuild {
            cmake {
                arguments("-DCMAKE_BUILD_TYPE=RelWithDebInfo")
            }
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }
    lint {
        disable.add("UnsafeExperimentalUsageError")
    }

    namespace = "com.zxingcpp"
}

val githubProperties = Properties()
githubProperties.load(FileInputStream(rootProject.file("github.properties")))

fun getVersionName() = version.toString()

fun getArtificatId() = "zxingcpp"
val repoUrl = "https://maven.pkg.github.com/SymetriaSpJ/zxing-cpp"

publishing {
    publications {
        create<MavenPublication>("maven") {
            groupId = "com.zxingcpp"
            artifactId = getArtificatId()
            version = getVersionName()
            artifact("$buildDir/outputs/aar/${getArtificatId()}-release.aar")
            pom.withXml {
                asNode().appendNode("repositories")
                    .appendNode("repository").apply {
                        appendNode("id", getArtificatId())
                        appendNode("url", repoUrl)
                    }
            }
        }

        repositories {
            maven {
                name = "GitHubPackages"
                url = uri(repoUrl)

                credentials {
                    /**Create github.properties in root project folder file with gpr.usr=GITHUB_USER_ID  & gpr.key=PERSONAL_ACCESS_TOKEN**/
                    username = githubProperties["gpr.usr"]?.toString() ?: System.getenv("GPR_USER")
                    password =
                        githubProperties["gpr.key"]?.toString() ?: System.getenv("GPR_API_KEY")
                }
            }
        }
    }

    dependencies {

        implementation("org.jetbrains.kotlin:kotlin-stdlib-jdk7:$kotlinVersion")
        implementation("androidx.core:core-ktx:1.9.0")
        implementation("androidx.core:core-ktx:+")

        val camerax_version = "1.2.1"
        implementation("androidx.camera:camera-core:${camerax_version}")

    }
}
