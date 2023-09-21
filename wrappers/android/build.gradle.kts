plugins {
    base
    alias(libs.plugins.android.application) apply false
    alias(libs.plugins.android.library) apply false
    alias(libs.plugins.kotlin.android) apply false
}

tasks.named<Delete>("clean") {
    val buildDirs = allprojects.map { it.layout.buildDirectory.asFile }
    delete(buildDirs)
}
