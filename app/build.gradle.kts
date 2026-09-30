plugins {
    alias(libs.plugins.android.application)
}

val enable_asan: Boolean by extra

android {
    namespace = "org.androidaudioplugin.ports.airwindows"
    compileSdk = libs.versions.android.compileSdk.get().toInt()
    defaultConfig {
        applicationId = "org.androidaudioplugin.ports.airwindows"
        minSdk = libs.versions.android.minSdk.get().toInt()
        targetSdk = libs.versions.android.targetSdk.get().toInt()
        versionCode = 1
        versionName = "0.1.0"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        proguardFiles ("proguard-rules.pro")

        externalNativeBuild {
            cmake {
                // https://github.com/google/prefab/blob/bccf5a6a75b67add30afbb6d4f7a7c50081d2d86/api/src/main/kotlin/com/google/prefab/api/Android.kt#L243
                arguments ("-DANDROID_STL=c++_shared", "-DAAP_ENABLE_ASAN=" + (if (enable_asan) "1" else "0"))
            }
        }
    }

    // see androidaudioplugin/build.gradle.kts for details...
    ndkVersion = libs.versions.ndk.get()

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles (getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }
    externalNativeBuild {
        cmake {
            path ("src/main/cpp/CMakeLists.txt")
            version = libs.versions.cmake.get()
        }
    }
    buildFeatures {
        prefab = true
    }
    packaging {
        // for debugging and ASan settings
        //jniLibs.keepDebugSymbols.add("*/*/*.so")
        if (enable_asan)
            jniLibs.useLegacyPackaging = true
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

dependencies {
    implementation (libs.aap.core)
    implementation (libs.aap.midi.device.service)
    implementation (libs.aap.ui.compose.app)
    implementation (libs.aap.ui.web)
    androidTestImplementation (libs.aap.testing)
    //  If you want to test aap-core locally, switch to these local references
    //  (along with settings.gradle.kts changes)
    /*
    implementation (project(":androidaudioplugin"))
    implementation (project(":androidaudioplugin-midi-device-service"))
    implementation (project(":androidaudioplugin-manager"))
    implementation (project(":androidaudioplugin-ui-compose-app"))
    implementation (project(":androidaudioplugin-ui-web"))
    androidTestImplementation (project(":androidaudioplugin-testing"))
     */
}
